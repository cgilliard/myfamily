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
#include <base/macros.h>
#include <base/osdef.h>
#include <base/print_util.h>
#include <base/slabs.h>

// Alloc/release

#define SLAB_SIZES 496
#define SLABS_PER_RESIZE 128
#define INITIAL_CHUNKS 0

int slabs_page_size = 0;

void __attribute__((constructor)) __get_page_size__() {
	slabs_page_size = getpagesize();
}

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};

unsigned int slabs_aligned_size(unsigned int size) {
	size_t aligned_size =
		((size_t)size + slabs_page_size - 1) & ~(slabs_page_size - 1);
	if (aligned_size >= UINT32_MAX) {
		SetErr(Overflow);
		return UINT32_MAX;
	}
	return aligned_size;
}

Alloc alloc(unsigned int size) {
	unsigned int aligned_size = slabs_aligned_size(size);
	if (aligned_size >= UINT32_MAX) {
		SetErr(Overflow);
		Alloc ret = {};
		return ret;
	}
	void *ret = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.alloc_sum += 1;
	} else {
		SetErr(AllocErr);
	}

	Alloc aret = {.ptr = ret, .size = aligned_size};
	return aret;
}

void release(Alloc ptr) {
	int code;
	if ((code = munmap(ptr.ptr, ptr.size))) {
		panic("munmap error = %i\n", code);
	}
	THREAD_LOCAL_RESOURCE_STATS.release_sum += 1;
}

int64 alloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.alloc_sum;
}
int64 resize_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.resize_sum;
}
int64 release_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.release_sum;
}

typedef struct Type {
	// slab id used internally by sa
	unsigned int id;
	// len of slab
	unsigned int len;
	// aux data can be used by caller as desired
	int64 aux;
	// user data
	byte data[];
} Type;

unsigned int slab_overhead() {
	return sizeof(Type);
}

unsigned int ptr_len(const Ptr ptr) {
	return ptr->len - sizeof(Type);
}
unsigned int ptr_id(const Ptr ptr) {
	return ptr->id;
}
void *ptr_data(const Ptr ptr) {
	return ptr->data;
}
void *ptr_aux(const Ptr ptr) {
	return &ptr->aux;
}

#include <stdio.h>

Ptr ptr_direct_alloc(unsigned int size) {
	Alloc a = alloc(size + sizeof(Type));
	Ptr ret = a.ptr;
	if (a.ptr == NULL) return NULL;
	ret->len = a.size;
	ret->id = 0;
	ret->aux = 0;
	return ret;
}

Ptr ptr_direct_resize(Ptr ptr, unsigned int size) {
	Alloc a = alloc(size + sizeof(Type));
	if (a.ptr == NULL) return NULL;
	Ptr nptr = a.ptr;
	nptr->len = a.size;
	nptr->id = ptr->id;
	nptr->aux = ptr->aux;
	unsigned int copy_sz;
	if (a.size < ptr->len)
		copy_sz = a.size;
	else
		copy_sz = ptr->len;
	memcpy($(nptr), $(ptr), copy_sz);
	ptr_direct_release(ptr);
	return nptr;
}

void ptr_direct_release(Ptr ptr) {
	Alloc a = {.ptr = ptr, .size = ptr_len(ptr)};
	release(a);
}

typedef struct SlabType {
	unsigned int slab_size;
	unsigned int slabs_per_resize;
	unsigned int initial_chunks;
	unsigned int max_slabs;
} SlabType;

typedef struct SlabData {
	SlabType type;
	Ptr data;
	Ptr free_list;
	unsigned int cur_chunks;
	unsigned int cur_slabs;
	unsigned int free_list_head;
	int spin_lock;
} SlabData;

typedef struct SlabAllocatorImpl {
	int64 sd_count;
	SlabData sd_arr[];
} SlabAllocatorImpl;

void slab_allocator_cleanup(SlabAllocator *ptr) {
	if (ptr == NULL) return;
	SlabAllocatorNc sa = *ptr;
	if (sa) {
		if (sa->sd_count) {
			for (int i = 0; i < sa->sd_count; i++) {
				SlabData *sd = &sa->sd_arr[i];
				if (sd->cur_chunks) {
					ptr_direct_release(sd->free_list);
					Ptr *datalist = ptr_data(sd->data);
					for (int64 j = 0; j < sd->cur_chunks; j++) {
						ptr_direct_release(datalist[j]);
					}
					ptr_direct_release(sd->data);
					sd->cur_chunks = 0;
				}
			}
		}

		int size = sizeof(SlabAllocatorImpl) + SLAB_SIZES * sizeof(SlabData);
		unsigned int aligned = slabs_aligned_size(size);
		Alloc saa = {.ptr = sa, .size = aligned};
		release(saa);
		*ptr = NULL;
	}
}

void slab_allocator_init_free_list(SlabData *sd, int64 chunks) {
	sd->free_list_head = sd->cur_chunks * sd->type.slabs_per_resize;
	int64 count = chunks * (int64)sd->type.slabs_per_resize;
	for (int64 i = 0; i < count; i++) {
		if (i == count - 1) {
			*(unsigned int *)$(&sd->free_list[i + sd->free_list_head]) =
				UINT32_MAX;
		} else {
			*(unsigned int *)$(&sd->free_list[i + sd->free_list_head]) =
				1 + i + sd->free_list_head;
		}
	}
}

int slab_allocator_increase_chunks(SlabData *sd, int64 chunks) {
	if (sd->cur_chunks == 0) {
		sd->free_list = ptr_direct_alloc(chunks * sd->type.slabs_per_resize *
										 sizeof(unsigned int));
		if (sd->free_list == NULL) return -1;
		sd->data = ptr_direct_alloc(chunks * sizeof(Ptr));
		if (sd->data == NULL) {
			ptr_direct_release(sd->free_list);
			return -1;
		}
		Ptr *datalist = ptr_data(sd->data);
		for (int i = 0; i < chunks; i++) {
			datalist[i] =
				ptr_direct_alloc((chunks * sd->type.slabs_per_resize) *
								 (slab_overhead() + sd->type.slab_size));
			if (datalist[i] == NULL) {
				ptr_direct_release(sd->free_list);
				for (int64 j = i - 1; j >= 0; j--) {
					ptr_direct_release(&sd->data[j]);
				}
				ptr_direct_release(sd->data);
				return -1;
			}
		}
	} else {
		if (((int64)chunks + (int64)sd->cur_chunks) *
				(int64)sd->type.slabs_per_resize >
			sd->type.max_slabs) {
			SetErr(Overflow);
			return -1;
		}

		int nsize = (chunks + sd->cur_chunks) * sd->type.slabs_per_resize *
					sizeof(unsigned int);
		Ptr tmp = ptr_direct_resize(sd->free_list, nsize);
		if (nil(tmp)) return -1;
		sd->free_list = tmp;

		Ptr tmp2 = ptr_direct_resize(
			sd->data, (chunks + sd->cur_chunks) * sizeof(byte *));
		if (nil(tmp2)) return -1;
		sd->data = tmp2;
		Ptr *data = $(sd->data);
		for (int64 i = 0; i < chunks; i++) {
			data[i + sd->cur_chunks] =
				ptr_direct_alloc(sd->type.slabs_per_resize *
								 (slab_overhead() + sd->type.slab_size));
			if (data[i + sd->cur_chunks] == NULL) {
				for (int64 j = i - 1; j >= 0; j--) {
					ptr_direct_release(data[j + sd->cur_chunks]);
				}
			}
		}
	}

	slab_allocator_init_free_list(sd, chunks);
	sd->cur_chunks += chunks;
	return 0;
}

int slab_allocator_init_data(SlabData *sd) {
	sd->cur_slabs = 0;
	sd->cur_chunks = 0;
	sd->free_list_head = UINT32_MAX;
	sd->free_list = NULL;
	sd->data = NULL;
	sd->spin_lock = 0;

	if (sd->type.initial_chunks) {
		if (slab_allocator_increase_chunks(sd, sd->type.initial_chunks))
			return -1;
	}
	return 0;
}

int slab_allocator_init_state(SlabAllocator sa) {
	for (int i = 0; i < sa->sd_count; i++) {
		if (slab_allocator_init_data(&sa->sd_arr[i])) return -1;
	}

	return 0;
}

int slab_allocator_get_size(unsigned int index) {
	if (index <= 256) return index * 16;
	if (index <= 496) return (index - 240) * 256;
	return -1;
}

int slab_allocator_get_index(unsigned int size) {
	if (size <= 4096) return (15 + size) / 16;
	if (size <= 65536) return 240 + (255 + size) / 256;
	return -1;
}

SlabAllocator slab_allocator_create() {
	if (slabs_page_size == 0) {
		panic("slabs_page_size == 0");
	}
	SlabAllocatorNc ret;
	Alloc a = alloc(sizeof(SlabAllocatorImpl) + SLAB_SIZES * sizeof(SlabData));
	if (a.ptr == NULL) return NULL;
	ret = a.ptr;
	ret->sd_count = 0;

	int size;
	while ((size = slab_allocator_get_size(ret->sd_count)) >= 0) {
		SlabData *sd = &ret->sd_arr[ret->sd_count];
		int slabs_per_resize = slabs_page_size / (slab_overhead() + size);
		if (slabs_per_resize == 0) slabs_per_resize = 1;
		sd->type = (const SlabType){.slab_size = size,
									.slabs_per_resize = slabs_per_resize,
									.initial_chunks = INITIAL_CHUNKS,
									.max_slabs = UINT32_MAX};
		ret->sd_count++;
	}
	ret->sd_count--;

	if (ret->sd_count != SLAB_SIZES)
		panic("sd_count mismatch. Expected %i, Found %i!", SLAB_SIZES,
			  ret->sd_count);

	if (slab_allocator_init_state(ret)) {
		slab_allocator_cleanup(&ret);
		return NULL;
	}

	return ret;
}

int64 slab_allocator_slab_data_index(SlabData *sd, int64 id) {
	return id / sd->type.slabs_per_resize;
}

int64 slab_allocator_slab_data_offset(SlabData *sd, int64 id) {
	return (id % sd->type.slabs_per_resize) *
		   (slab_overhead() + sd->type.slab_size);
}

Ptr slab_allocator_allocate_sd(SlabData *sd, SlabAllocator sa) {
	bool err_cond = false;
	// spinlock only allowing one thread to enter critical section
	int expected_spin_lock = 0;
	int desired_spin_lock = 1;
	// spin while the spin_lock value is not equal to 0 then set it to 1
	do {
	} while (!__atomic_compare_exchange_n(&sd->spin_lock, &expected_spin_lock,
										  desired_spin_lock, false,
										  __ATOMIC_ACQUIRE, __ATOMIC_RELAXED) &&
			 __atomic_load_n(&sd->spin_lock, __ATOMIC_RELAXED) != 0);

	if (sd->free_list_head == UINT32_MAX) {
		if (slab_allocator_increase_chunks(sd, 1)) {
			err_cond = true;
		}
		if (!err_cond && sd->free_list_head == UINT32_MAX) {
			SetErr(CapacityExceeded);
		}
	}

	// set the spin_lock back to 0
	__atomic_store(&sd->spin_lock, &expected_spin_lock, __ATOMIC_RELEASE);

	if (err_cond) return NULL;

	unsigned int old_free_list_head;
	unsigned int new_free_list_head;
	Ptr ptr;

	do {
		old_free_list_head = sd->free_list_head;
		int64 index = slab_allocator_slab_data_index(sd, old_free_list_head);
		int64 offset = slab_allocator_slab_data_offset(sd, old_free_list_head);

		Ptr *ptrs = $(sd->data);
		ptr = (Type *)($(ptrs[index]) + offset);
		ptr->id = old_free_list_head;
		ptr->len = sd->type.slab_size;
		new_free_list_head = ((unsigned int *)$(sd->free_list))[ptr->id];

	} while (!__atomic_compare_exchange_n(
		&sd->free_list_head, &old_free_list_head, new_free_list_head, false,
		__ATOMIC_RELAXED, __ATOMIC_RELAXED));

	__atomic_fetch_add(&sd->cur_slabs, 1, __ATOMIC_RELAXED);

	return ptr;
}

Ptr slab_allocator_allocate(SlabAllocator sa, unsigned int size) {
	int index = slab_allocator_get_index(size + slab_overhead());
	if (index < 0) return NULL;

	Ptr ret = slab_allocator_allocate_sd(&sa->sd_arr[index], sa);

	return ret;
}

void slab_allocator_data_free(SlabData *sd, unsigned int id) {
	unsigned int old_free_list_head;
	unsigned int new_free_list_head;
	do {
		old_free_list_head = sd->free_list_head;
		new_free_list_head = id;  // Calculate new_free_list_head here
		((unsigned int *)$(sd->free_list))[id] =
			old_free_list_head;	 // Update sd->free_list[id] here
	} while (!__atomic_compare_exchange_n(
		&sd->free_list_head, &old_free_list_head, new_free_list_head, false,
		__ATOMIC_RELAXED, __ATOMIC_RELAXED));
	__atomic_fetch_sub(&sd->cur_slabs, 1, __ATOMIC_RELAXED);
}

void slab_allocator_free(SlabAllocator sa, Ptr ptr) {
	if (ptr == NULL || sa == NULL) {
		panic("Invalid ptr sent to slab_allocator free!");
	}
	unsigned int len = ptr_len(ptr) + slab_overhead();
	int index = slab_allocator_get_index(len);
	if (index < 0) {
		panic("Invalid ptr sent to slab_allocator free! Unknown size %lli.",
			  len);
	}
	if (sa->sd_arr[index].type.slab_size != len) {
		panic(
			"Invalid ptr sent to slab_allocator free! Size mismatch %lli vs. "
			"%lli.",
			sa->sd_arr[index].type.slab_size, len);
	}

	slab_allocator_data_free(&sa->sd_arr[index], ptr->id);
}

Ptr ptr_for(SlabAllocator sa, unsigned int id, unsigned int len) {
	int index = slab_allocator_get_index(len);
	if (index < 0) {
		panic(
			"Invalid ptr sent to slab_allocator ptr_for! Unknown id=%u,len=%u.",
			id, len);
	}
	SlabData sd = sa->sd_arr[index];
	int64 offset = slab_allocator_slab_data_offset(&sd, id);
	Ptr *ptrs = $(sd.data);
	Ptr ptr = ptrs[index];
	Ptr ret = (Type *)($(ptr) + offset);
	return ret;
}

int64 slab_allocator_cur_slabs_allocated(const SlabAllocator sa) {
	int64 slabs = 0;
	for (int i = 0; i < sa->sd_count; i++) {
		SlabData *sd = &sa->sd_arr[i];
		slabs += __atomic_fetch_add(&sd->cur_slabs, 0, __ATOMIC_RELAXED);
	}
	return slabs;
}
