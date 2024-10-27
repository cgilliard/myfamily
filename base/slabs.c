// Copyright (c) 2024, The MyFamily Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <base/fam_err.h>
#include <base/os.h>
#include <base/print_util.h>
#include <base/slabs.h>
#include <stdio.h>

#define MAX_SLAB_TYPES 256

typedef struct PtrImpl {
	int id;		  // slab id
	int len;	  // length of data in this slab (up to 3 bytes, 1 byte reserved for user flags)
	int counter1; // user counter1 (potentially for rc strong count)
	int counter2; // user counter2 (potentially for rc weak count)
	byte data[];  // user data
} PtrImpl;

const PtrImpl null = {.id = 0, .len = 0};

//  returns the slab's length
int ptr_len(const Ptr ptr) {
	return ptr->len & 0x00FFFFFF;
}

// returns the slabs id
int ptr_id(const Ptr ptr) {
	// mask off flag byte (top byte)
	return ptr->id;
}

// reutrn pointer to slab data
void *ptr_data(const Ptr ptr) {
	return ptr->data;
}

// check whether a flag is set (0-7 are possible values)
bool ptr_flag_check(const Ptr ptr, byte flag) {
	return flag < 8 && ((int)(ptr->len >> 24)) & (1 << flag);
}

// set/unset a flag
void ptr_flag_set(Ptr ptr, byte flag, bool value) {
	if (flag >= 8)
		return;
	if (value)
		ptr->len |= (int)(1 << (int)flag) << 24;
	else
		ptr->len &= ~((int)(1 << (int)flag) << 24);
}

void ptr_count1_set(Ptr ptr, int value) {
	ptr->counter1 = value;
}

void ptr_count2_set(Ptr ptr, int value) {
	ptr->counter2 = value;
}

void ptr_count1_incr(Ptr ptr, int value) {
	ptr->counter1++;
}

void ptr_count2_incr(Ptr ptr, int value) {
	ptr->counter2++;
}

void ptr_count1_decr(Ptr ptr, int value) {
	ptr->counter1--;
}

void ptr_count2_decr(Ptr ptr, int value) {
	ptr->counter2--;
}

int ptr_count1_get(Ptr ptr) {
	return ptr->counter1;
}
int ptr_count2_get(Ptr ptr) {
	return ptr->counter2;
}

#include <stdio.h>

// return true if this Ptr is nil (unallocated)
bool ptr_is_nil(const Ptr ptr) {
	return ptr->len == 0; // a nil pointer has 0 len
}

typedef struct SlabAllocatorConfigImpl {
	int slab_types_count;
	int slab_types_capacity;
	SlabType *slab_types;
} SlabAllocatorConfigImpl;

void slab_allocator_config_cleanup(SlabAllocatorConfig *ptr) {
	if (ptr == NULL)
		return;
	SlabAllocatorConfigNc sac = *ptr;
	if (sac == NULL)
		return;
	if (sac->slab_types_capacity) {
		release(sac->slab_types);
		sac->slab_types_capacity = 0;
	}
}

SlabAllocatorConfig slab_allocator_config_create() {
	SlabAllocatorConfigNc ret =
		(SlabAllocatorConfigImpl *)alloc(sizeof(SlabAllocatorConfigImpl), false);
	ret->slab_types_count = 0;
	ret->slab_types_capacity = MAX_SLAB_TYPES;
	ret->slab_types = alloc(MAX_SLAB_TYPES * sizeof(SlabType), false);
	if (ret->slab_types == NULL) {
		return NULL;
	}
	return ret;
}

int64 slab_allocator_config_add_type(SlabAllocatorConfig sc, const SlabType *st) {
	if (sc->slab_types_count >= MAX_SLAB_TYPES) {
		SetErr(Overflow);
		return -1;
	}
	sc->slab_types[sc->slab_types_count] = *st;
	sc->slab_types_count++;
	return 0;
}

typedef struct SlabData {
	SlabType type;
	byte **data;
	unsigned int *free_list;
	unsigned int cur_chunks;
	unsigned int cur_slabs;
	unsigned int free_list_head;
} SlabData;

typedef struct SlabAllocatorImpl {
	int64 sd_count;
	SlabData sd_arr[];
} SlabAllocatorImpl;

void slab_allocator_cleanup(SlabAllocator *ptr) {
	SlabAllocatorNc sa = *ptr;
	if (sa) {
		if (sa->sd_count) {
			for (int64 i = 0; i < sa->sd_count; i++) {
			}
		}
	}
}

int compare_sd(const void *a, const void *b) {
	const SlabData *sda = a;
	const SlabData *sdb = b;
	if (sda->type.slab_size < sdb->type.slab_size)
		return -1;
	else if (sda->type.slab_size > sdb->type.slab_size)
		return 1;
	return 0;
}

void slab_allocator_sort_sd_arr(SlabAllocator sa) {
	qsort(sa->sd_arr, sa->sd_count, sizeof(SlabData), compare_sd);
}

int slab_allocator_check_config(SlabAllocator sa) {
	int64 last_size = 0;
	for (int i = 0; i < sa->sd_count; i++) {
		if (sa->sd_arr[i].type.slab_size <= last_size) {
			SetErr(DuplicateSizes);
			return -1;
		}
		if (sa->sd_arr[i].type.slabs_per_resize < 1) {
			SetErr(IllegalArgument);
			return -1;
		}
		if (sa->sd_arr[i].type.initial_chunks < 0) {
			SetErr(IllegalArgument);
			return -1;
		}
		if (sa->sd_arr[i].type.max_slabs <
			sa->sd_arr[i].type.initial_chunks * sa->sd_arr[i].type.slabs_per_resize) {
			SetErr(IllegalArgument);
			return -1;
		}
		if (sa->sd_arr[i].type.slab_size < 16 || sa->sd_arr[i].type.slab_size % 16) {
			SetErr(IllegalArgument);
			return -1;
		}
		last_size = sa->sd_arr[i].type.slab_size;
	}
	return 0;
}

int64 slab_allocator_slab_data_index(SlabData *sd, int64 id) {
	return id / sd->type.slabs_per_resize;
}

int64 slab_allocator_slab_data_offset(SlabData *sd, int64 id) {
	return (id % sd->type.slabs_per_resize) * sd->type.slab_size;
}

void slab_allocator_init_free_list(SlabData *sd, int64 chunks) {
	sd->free_list_head = sd->cur_chunks * sd->type.slabs_per_resize;
	int64 count = chunks * sd->type.slabs_per_resize;
	for (int64 i = 0; i < count; i++) {
		if (i == count - 1) {
			sd->free_list[i + sd->free_list_head] = UINT32_MAX;
		} else {
			sd->free_list[i + sd->free_list_head] = 1 + i + sd->free_list_head;
		}
	}
}

int slab_allocator_increase_chunks(SlabData *sd, int64 chunks) {
	if (sd->cur_chunks == 0) {
		sd->free_list = alloc(chunks * sd->type.slabs_per_resize * sizeof(unsigned int), false);
		sd->data = alloc(chunks * sizeof(byte *), false);
		for (int64 i = 0; i < chunks; i++) {
			sd->data[i] = alloc(sd->type.slabs_per_resize * sd->type.slab_size, false);
		}
	} else {
	}

	slab_allocator_init_free_list(sd, chunks);
	return 0;
}

int slab_allocator_init_data(SlabData *sd) {
	sd->cur_slabs = 0;
	sd->cur_chunks = 0;
	sd->free_list_head = UINT32_MAX;
	sd->free_list = NULL;
	sd->data = NULL;

	if (sd->type.initial_chunks) {
		if (slab_allocator_increase_chunks(sd, sd->type.initial_chunks))
			return -1;
	}
	return 0;
}

int slab_allocator_init_state(SlabAllocator sa) {
	for (int i = 0; i < sa->sd_count; i++) {
		if (slab_allocator_init_data(&sa->sd_arr[i]))
			return -1;
	}

	return 0;
}

#define SLAB_SIZES 256

SlabAllocator slab_allocator_create() {
	SlabAllocatorNc ret = (SlabAllocatorImpl *)alloc(
		sizeof(SlabAllocatorImpl) + SLAB_SIZES * sizeof(SlabData), false);
	if (ret == NULL)
		return NULL;
	ret->sd_count = SLAB_SIZES;
	for (int i = 0; i < ret->sd_count; i++) {
		ret->sd_arr[i].type = (const SlabType) {.slab_size = (1 + i) * 16,
												.slabs_per_resize = 128,
												.initial_chunks = 1,
												.max_slabs = UINT32_MAX};
	}

	if (slab_allocator_init_state(ret))
		return NULL;
	return ret;
}

int slab_allocator_index(SlabAllocator sa, int64 size) {
	int ret = -1;

	if (sa->sd_count) {

		int left = 0;
		int right = sa->sd_count - 1;

		while (left <= right) {
			int mid = left + (right - left) / 2;
			int64 slab_size = sa->sd_arr[mid].type.slab_size;
			if (slab_size == size) {
				ret = mid;
				break;
			} else if (slab_size > size)
				right = mid - 1;
			else
				left = mid + 1;
		}

		if (ret == -1 && right + 1 < sa->sd_count) {
			ret = right + 1;
		}
	}

	return ret;
}

Ptr slab_allocator_allocate_sd(SlabData *sd) {
	if (sd->free_list_head == UINT32_MAX) {
		// TODO: resize
		return NULL;
	}
	int64 index = slab_allocator_slab_data_index(sd, sd->free_list_head);
	int64 offset = slab_allocator_slab_data_offset(sd, sd->free_list_head);
	printf("index=%lli,offset=%lli\n", index, offset);
	Ptr ptr = (PtrImpl *)(sd->data[index] + offset);
	ptr->id = sd->free_list_head;
	ptr->len = sd->type.slab_size;
	sd->free_list_head = sd->free_list[ptr->id];

	return ptr;
}

void slab_allocator_data_free(SlabData *sd, int64 id) {
	sd->free_list[id] = sd->free_list_head;
	sd->free_list_head = id;
}

Ptr slab_allocator_allocate(SlabAllocator sa, int64 size) {
	int index = slab_allocator_index(sa, size);
	if (index < 0)
		return NULL;

	return slab_allocator_allocate_sd(&sa->sd_arr[index]);
}

void slab_allocator_free(SlabAllocator sa, Ptr ptr) {
	if (ptr == NULL || sa == NULL) {
		panic("Invalid ptr sent to slab_allocator free!");
	}
	int64 len = ptr_len(ptr);
	int index = slab_allocator_index(sa, len);
	if (index < 0) {
		panic("Invalid ptr sent to slab_allocator free! Unknown size %lli.", len);
	}
	if (sa->sd_arr[index].type.slab_size != len) {
		panic("Invalid ptr sent to slab_allocator free! Size mismatch %lli vs. %lli.",
			  sa->sd_arr[index].type.slab_size, len);
	}

	slab_allocator_data_free(&sa->sd_arr[index], ptr->id);

	*ptr = null;
}
int64 slab_allocator_cur_slabs_allocated(const SlabAllocator sa) {
	return 0;
}

#ifdef TEST
Ptr ptr_test_obj(int64 id, int64 len, byte flags) {
	Ptr ptr = (PtrImpl *)alloc(sizeof(PtrImpl) + len, false);
	ptr->id = id;
	ptr->len = len;
	ptr->len |= ((int)flags) << 24;
	return ptr;
}

void ptr_free_test_obj(Ptr ptr) {
	release(ptr);
}
#endif // TEST
