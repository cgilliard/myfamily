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

#include <base/bitflags.h>
#include <base/fam_err.h>
#include <base/macro_utils.h>
#include <base/panic.h>
#include <base/resources.h>
#include <base/slabs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 2^24 -1 (3 bytes)
#define MAX_SLAB_SIZE (16777216 - 1)

#define SIZE_OVERHEAD (2 * sizeof(u32) + sizeof(u64))
#define SIZE_WITH_OVERHEAD(size) (size + SIZE_OVERHEAD)

#define BIT_FLAG_GLOBAL 0
#define BIT_FLAG_PIN 1
#define BIT_FLAG_COPY 2
#define BIT_FLAG_MALLOC 3
#define BIT_FLAG_NIL 6

typedef struct FatPtr32Impl {
	u32 id;
	u8 size_flags[4];
	void *data;
} FatPtr32Impl;

int fat_ptr_mallocate(FatPtr *ptr, u64 size) {
	void *tmp = mymalloc(SIZE_WITH_OVERHEAD(size));
	if (tmp == NULL)
		return -1;

	FatPtr32Impl *fptr32 = tmp;
	ptr->data = tmp;
	fptr32->id = 0; // id doesn't matter with mallocate
	memcpy(&fptr32->size_flags, &size, 3);
	fptr32->size_flags[3] = 0;
	BitFlags bf = {.flags = (fptr32->size_flags + 3), .capacity = 1};
	bitflags_set(&bf, BIT_FLAG_MALLOC, true);
	fptr32->data = tmp + SIZE_OVERHEAD;

	return 0;
}

void fat_ptr_malloc_free(FatPtr *ptr) {
	if (ptr->data) {
		myfree(ptr->data);
		ptr->data = NULL;
	}
}

int fat_ptr_pin(FatPtr *ptr) {
	int ret = -1;
	if (ptr && ptr->data) {
		FatPtr32Impl *fptr = ptr->data;
		BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
		ret = bitflags_set(&bf, BIT_FLAG_PIN, true);
	} else
		fam_err = IllegalArgument;
	return ret;
}

int fat_ptr_set_copy(FatPtr *ptr) {
	int ret = -1;
	if (ptr && ptr->data) {
		FatPtr32Impl *fptr = ptr->data;
		BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
		ret = bitflags_set(&bf, BIT_FLAG_COPY, true);
	} else
		fam_err = IllegalArgument;
	return ret;
}

u32 fat_ptr_size(const FatPtr *ptr) {
	u32 ret = 0;
	if (ptr != NULL) {
		FatPtr32Impl *tmp = ptr->data;
		if (!nil(*ptr))
			memcpy(&ret, tmp->size_flags, 3);
	} else
		fam_err = IllegalArgument;
	return ret;
}

void *fat_ptr_data(const FatPtr *ptr) {
	void *ret = NULL;
	if (ptr != NULL) {
		FatPtr32Impl *tmp = ptr->data;
		if (!nil(*ptr) && ptr->data != NULL)
			ret = ((FatPtr32Impl *)ptr->data)->data;
	} else
		fam_err = IllegalArgument;
	return ret;
}

bool fat_ptr_is_global(const FatPtr *ptr) {
	bool ret = false;
	if (ptr && ptr->data) {
		FatPtr32Impl *fptr = ptr->data;
		BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
		ret = bitflags_check(&bf, BIT_FLAG_GLOBAL);
	}
	return ret;
}

bool fat_ptr_is_pin(const FatPtr *ptr) {
	bool ret = false;
	if (ptr && ptr->data) {
		FatPtr32Impl *fptr = ptr->data;
		BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
		ret = bitflags_check(&bf, BIT_FLAG_PIN);
	}
	return ret;
}
bool fat_ptr_is_copy(const FatPtr *ptr) {
	bool ret = false;
	if (ptr && ptr->data) {
		FatPtr32Impl *fptr = ptr->data;
		BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
		ret = bitflags_check(&bf, BIT_FLAG_COPY);
	}
	return ret;
}

bool fat_ptr_is_malloc(const FatPtr *ptr) {
	bool ret = false;
	if (ptr && ptr->data) {
		FatPtr32Impl *fptr = ptr->data;
		BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
		ret = bitflags_check(&bf, BIT_FLAG_MALLOC);
	}
	return ret;
}

bool fat_ptr_is_nil(const FatPtr *ptr) {
	bool ret = true;
	if (ptr && ptr->data) {
		FatPtr32Impl *fptr = ptr->data;
		BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
		ret = bitflags_check(&bf, BIT_FLAG_NIL);
	}
	return ret;
}

void slab_allocator_config_cleanup(SlabAllocatorConfigNc *sc) {
	if (sc->slab_types) {
		myfree(sc->slab_types);
		sc->slab_types = NULL;
	}
}

int slab_allocator_config_build(SlabAllocatorConfig *sc, bool zeroed, bool global) {
	sc->zeroed = zeroed;
	sc->global = global;
	sc->slab_types_count = 0;
	sc->slab_types = NULL;
	return 0;
}

int slab_allocator_config_add_type(SlabAllocatorConfig *sc, const SlabType *st) {
	if (sc->slab_types) {
		void *tmp = myrealloc(sc->slab_types, sizeof(SlabType) * (1 + sc->slab_types_count));
		if (tmp == NULL)
			return -1;
		sc->slab_types = tmp;
	} else {
		sc->slab_types = mymalloc(sizeof(SlabType));
		if (sc->slab_types == NULL)
			return -1;
	}

	sc->slab_types[sc->slab_types_count] = *st;
	sc->slab_types_count++;

	return 0;
}

// Slab Allocator

typedef struct SlabData {
	SlabType type; // This slab data's type information (slab_size, slabs_per_resize, initial, max)
	void **data;   // Pointers to each chunk
	void *free_list;	// The free list pointers
	u32 cur_chunks;		// number of chunks currently allocated
	u32 cur_slabs;		// number of slabs currently allocated
	u32 free_list_head; // the free list head
} SlabData;

typedef struct SlabAllocatorImpl {
	u64 sd_size;	  // size of the SlabData array
	SlabData *sd_arr; // slab data array one for each SlabType
	bool zeroed;	  // config zeroed
	bool global;	  // whether this slab allocator is considered 'global'
} SlabAllocatorImpl;

void slab_allocator_cleanup(SlabAllocatorNc *ptr) {
	if (ptr->impl) {
		SlabAllocatorImpl *impl = ptr->impl;
		if (impl->sd_arr) {
			for (u64 i = 0; i < impl->sd_size; i++) {
				for (int j = 0; j < impl->sd_arr[i].cur_chunks; j++) {
					if (impl->sd_arr[i].data[j]) {
						myfree(impl->sd_arr[i].data[j]);
						impl->sd_arr[i].data[j] = NULL;
					}
				}
				if (impl->sd_arr[i].data) {
					myfree(impl->sd_arr[i].data);
					impl->sd_arr[i].data = NULL;
				}
				if (impl->sd_arr[i].free_list) {
					myfree(impl->sd_arr[i].free_list);
					impl->sd_arr[i].free_list = NULL;
				}
			}

			myfree(impl->sd_arr);
			impl->sd_arr = NULL;
		}
		myfree(ptr->impl);
		ptr->impl = NULL;
	}
}

int slab_allocator_init_free_list(SlabData *sd, u32 slabs, u32 offset) {
	//  initialize the values of the free list.
	for (u32 i = 0; i < slabs; i++) {
		if (i == (slabs - 1)) {
			((u32 *)(sd->free_list))[i + offset] = UINT32_MAX;
		} else {
			((u32 *)(sd->free_list))[i + offset] = offset + i + 1;
		}
	}

	return 0;
}

int slab_allocator_init_slab_data(SlabAllocatorImpl *impl, SlabType *st, u64 index) {
	impl->sd_arr[index].type = *st;
	impl->sd_arr[index].type.slab_size += SIZE_OVERHEAD;
	impl->sd_arr[index].cur_slabs = 0;
	impl->sd_arr[index].cur_chunks = st->initial_chunks;
	if (st->initial_chunks > 0) {
		impl->sd_arr[index].free_list_head = 0;
		impl->sd_arr[index].data = mymalloc(sizeof(void *) * st->initial_chunks);
		if (impl->sd_arr[index].data == NULL)
			return -1;
	} else {
		impl->sd_arr[index].data = NULL;
		impl->sd_arr[index].free_list_head = UINT32_MAX;
	}

	if (st->initial_chunks > 0) {
		impl->sd_arr[index].free_list =
			mymalloc(sizeof(u32) * (u64)st->slabs_per_resize * (u64)st->initial_chunks);
		if (impl->sd_arr[index].free_list == NULL) {
			myfree(impl->sd_arr[index].data);
			return -1;
		}
	} else {
		impl->sd_arr[index].free_list = NULL;
	}

	for (u64 i = 0; i < st->initial_chunks; i++) {
		impl->sd_arr[index].data[i] =
			mymalloc((u64)impl->sd_arr[index].type.slab_size * (u64)st->slabs_per_resize);

		if (impl->sd_arr[index].data[i] == NULL) {
			for (u64 j = 0; j < i; j++) {
				myfree(impl->sd_arr[index].data[j]);
			}
			myfree(impl->sd_arr[index].data);
			return -1;
		}
	}

	slab_allocator_init_free_list(&impl->sd_arr[index], st->initial_chunks * st->slabs_per_resize,
								  0);
	return 0;
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

int slab_allocator_sort_slab_data(SlabAllocator *ptr) {
	SlabAllocatorImpl *impl = ptr->impl;
	qsort(impl->sd_arr, impl->sd_size, sizeof(SlabData), compare_sd);
	u32 last_slab_size = 0;
	for (u64 i = 0; i < impl->sd_size; i++) {
		if (impl->sd_arr[i].type.slab_size >= MAX_SLAB_SIZE) { // 2^24 - 1
			fam_err = TooBig;
			return -1;
		}

		if (last_slab_size >= impl->sd_arr[i].type.slab_size) {
			fam_err = IllegalArgument;
			return -1;
		}
		// for alignment, must be divisible by 8
		if (impl->sd_arr[i].type.slab_size % 8 != 0) {
			fam_err = IllegalArgument;
			return -1;
		}
		// can't have higher max slabs than initial
		if (impl->sd_arr[i].type.slabs_per_resize * impl->sd_arr[i].type.initial_chunks >
			impl->sd_arr[i].type.max_slabs) {
			fam_err = IllegalArgument;
			return -1;
		}
		// max slabs
		if (impl->sd_arr[i].type.max_slabs > MAX_SLABS) {
			fam_err = IllegalArgument;
			return -1;
		}

		// max slabs must be dividisble by slabs_per_resize
		if (impl->sd_arr[i].type.max_slabs % impl->sd_arr[i].type.slabs_per_resize) {
			fam_err = IllegalArgument;
			return -1;
		}
		last_slab_size = impl->sd_arr[i].type.slab_size;
	}
	return 0;
}

int slab_allocator_build(SlabAllocator *ptr, const SlabAllocatorConfig *config) {
	ptr->impl = mymalloc(sizeof(SlabAllocatorImpl));
	if (ptr->impl == NULL)
		return -1;
	SlabAllocatorImpl *impl = ptr->impl;

	impl->zeroed = config->zeroed;
	impl->global = config->global;
	impl->sd_size = config->slab_types_count;
	impl->sd_arr = mymalloc(sizeof(SlabData) * impl->sd_size);
	if (impl->sd_arr == NULL) {
		myfree(ptr->impl);
		return -1;
	}

	for (u64 i = 0; i < config->slab_types_count; i++) {
		if (slab_allocator_init_slab_data(impl, &config->slab_types[i], i)) {
			// cleanup previously allocated slabs
			impl->sd_size = i;
			slab_allocator_cleanup(ptr);
			return -1;
		}
	}
	int ret = slab_allocator_sort_slab_data(ptr);
	if (ret)
		slab_allocator_cleanup(ptr);
	return ret;
}

int slab_allocator_index(const SlabAllocator *ptr, u32 size) {
	SlabAllocatorImpl *impl = ptr->impl;
	int ret = -1;
	if (impl->sd_size > 0) {

		int left = 0;
		int right = impl->sd_size - 1;

		while (left <= right) {
			int mid = left + (right - left) / 2;
			u32 slab_size = impl->sd_arr[mid].type.slab_size;
			if (slab_size == size) {
				ret = mid;
				break;
			} else if (slab_size > size)
				right = mid - 1;
			else
				left = mid + 1;
		}

		if (ret == -1 && right + 1 <= impl->sd_size - 1) {
			ret = right + 1;
		}
	}

	return ret;
}

int slab_data_try_resize(SlabData *sd, FatPtr *fptr, bool zeroed, bool global);

int slab_data_allocate(SlabData *sd, FatPtr *fptr, bool zeroed, bool global) {
	u32 id = sd->free_list_head;
	if (id == UINT32_MAX) {
		return slab_data_try_resize(sd, fptr, zeroed, global);
	}

	sd->free_list_head = ((u32 *)sd->free_list)[id];

	// set to MAX-1 as a marker that this is allocated
	// if any freed without this value, it's an error
	((u32 *)sd->free_list)[id] = (UINT32_MAX - 1);

	u32 len = sd->type.slab_size;
	u64 slab_data_index = id / sd->type.slabs_per_resize;
	u64 offset_mod = id % sd->type.slabs_per_resize;

	fptr->data = sd->data[slab_data_index] + offset_mod * sd->type.slab_size;

	FatPtr32Impl *fptr32 = fptr->data;
	fptr32->id = id;
	u64 data_len = len - SIZE_OVERHEAD;
	memcpy(&fptr32->size_flags, &data_len, 3);
	fptr32->size_flags[3] = 0;
	fptr32->data = fptr->data + SIZE_OVERHEAD;
	u8 *data_ptr = fptr32->data;
	if (global) {
		BitFlags bf = {.flags = (fptr32->size_flags + 3), .capacity = 1};
		bitflags_set(&bf, BIT_FLAG_GLOBAL, true);
	}

	sd->cur_slabs += 1;
	if (zeroed) {
		for (u64 i = 0; i < data_len; i++) {
			data_ptr[i] = 0;
		}
	}

	return 0;
}

int slab_data_try_resize(SlabData *sd, FatPtr *fptr, bool zeroed, bool global) {
	int ret = -1;
	if (sd->cur_slabs < sd->type.max_slabs) {
		// we can try to resize
		// first try to reallocate the free list
		void *nfree_list = NULL;
		u64 factor = sizeof(u32);
		if (sd->free_list) {
			nfree_list =
				myrealloc(sd->free_list, (1 + sd->cur_chunks) * sd->type.slabs_per_resize * factor);
		} else {

			nfree_list = mymalloc((u64)sd->type.slabs_per_resize * (u64)factor);
		}
		if (nfree_list != NULL) {

			sd->free_list = nfree_list;

			// now try to allocate more slab chunks
			void *ndata;
			if (sd->data == NULL) {
				ndata = mymalloc(sizeof(void *));
			} else {
				ndata = myrealloc(sd->data, sizeof(void *) * (1 + sd->cur_chunks));
			}
			if (ndata != NULL) {

				sd->data = ndata;
				sd->data[sd->cur_chunks] = mymalloc(sd->type.slab_size * sd->type.slabs_per_resize);
				if (sd->data[sd->cur_chunks] != NULL) {

					// data was successfully allocated, make updates.
					slab_allocator_init_free_list(sd, sd->type.slabs_per_resize, sd->cur_slabs);
					sd->free_list_head = sd->cur_slabs;
					sd->cur_chunks += 1;

					return slab_data_allocate(sd, fptr, zeroed, global);
				}
			}
		}
	}
	return ret;
}

void slab_data_free(SlabData *sd, const FatPtr *fptr, bool zeroed) {
	u64 data_len = 0;
	u8 *data_ptr;
	if (sd->free_list == NULL) {
		panic("free list not initialized (wrong slab allocator?)");
	}

	FatPtr32Impl *fptr32 = fptr->data;
	memcpy(&data_len, &fptr32->size_flags, 3);
	data_ptr = fptr32->data;
	u32 id = fptr32->id;
	if (((u32 *)sd->free_list)[id] != (UINT32_MAX - 1)) {
		panic("Potential double free. Id = %llu.\n", id);
	}
	((u32 *)sd->free_list)[id] = sd->free_list_head;
	sd->free_list_head = id;
	if (sd->cur_slabs == 0)
		panic("Potential double free. Id = %llu. Freeing slabs when none are allocated.\n", id);
	sd->cur_slabs -= 1;

	if (zeroed) {
		for (u64 i = 0; i < data_len; i++) {
			data_ptr[i] = 0;
		}
	}
}

int slab_allocator_allocate(SlabAllocator *ptr, u32 size, FatPtr *fptr) {
	if (ptr == NULL || size == 0) {
		fam_err = IllegalArgument;
		return -1;
	}

	SlabAllocatorImpl *impl = ((SlabAllocatorImpl *)ptr->impl);
	bool zeroed = impl->zeroed;
	bool global = impl->global;
	u32 needed = size + SIZE_OVERHEAD;
	int index = slab_allocator_index(ptr, needed);
	int ret = -1;
	if (index >= 0) {
		SlabData *sd = &impl->sd_arr[index];
		ret = slab_data_allocate(sd, fptr, zeroed, global);
	} else
		fam_err = ResourceNotAvailable;
	return ret;
}
void slab_allocator_free(SlabAllocator *ptr, FatPtr *fptr) {
	if (fptr == NULL || nil(*fptr)) {
		panic("Freeing a slab that's already nil. Double free?");
	}
	SlabAllocatorImpl *impl = ptr->impl;

	FatPtr32Impl *fptr32 = fptr->data;

	u32 needed = fat_ptr_size(fptr) + SIZE_OVERHEAD;
	int index = slab_allocator_index(ptr, needed);
	if (index < 0)
		panic("Freeing a slab with an unknown slab size %llu.\n", needed);
	SlabData *sd = &((SlabAllocatorImpl *)ptr->impl)->sd_arr[index];
	if (needed != sd->type.slab_size)
		panic("Freeing a slab with an unknown slab size %llu.\n", needed);
	slab_data_free(sd, fptr, ((SlabAllocatorImpl *)ptr->impl)->zeroed);
	*fptr = null;
}
u64 slab_allocator_cur_slabs_allocated(const SlabAllocator *ptr) {
	SlabAllocatorImpl *impl = ptr->impl;
	u64 ret = 0;

	for (u64 i = 0; i < impl->sd_size; i++) {
		ret += impl->sd_arr[i].cur_slabs;
	}
	return ret;
}

// These are test helper functions
#ifdef TEST
u32 fat_ptr_id(const FatPtr *ptr) {
	return ((FatPtr32Impl *)ptr->data)->id;
}
void fat_ptr_test_obj32(FatPtr *ptr, u32 id, u32 len, bool global, bool pin, bool copy) {
	ptr->data = mymalloc(SIZE_OVERHEAD + len);
	FatPtr32Impl *fptr = ptr->data;
	fptr->id = id;
	fptr->size_flags[3] = 0;
	BitFlags bf = {.flags = (fptr->size_flags + 3), .capacity = 1};
	bitflags_set(&bf, BIT_FLAG_GLOBAL, global);
	bitflags_set(&bf, BIT_FLAG_PIN, pin);
	bitflags_set(&bf, BIT_FLAG_COPY, copy);

	memcpy(&fptr->size_flags, &len, 3);
	fptr->data = ptr->data + SIZE_OVERHEAD;
}
void fat_ptr_free_test_obj32(FatPtr *ptr) {
	if (ptr && ptr->data) {
		myfree(ptr->data);
		ptr->data = NULL;
	}
}
#endif // TEST
