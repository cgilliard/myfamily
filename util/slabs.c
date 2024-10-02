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

#include <base/macro_utils.h>
#include <base/resources.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/panic.h>
#include <util/slabs.h>

#define MALLOC_64_ID (INT64_MAX - 8)
#define MALLOC_32_ID (INT32_MAX - 8)

// Define both 64/32 bit impls the slab allocator will build the approproate types
// based on configuration.
typedef struct FatPtr64Impl {
	u64 id;
	u64 len;
	void *data;
} FatPtr64Impl;

typedef struct FatPtr32Impl {
	u32 id;
	u32 len;
	void *data;
} FatPtr32Impl;

// deteremine which kind of fat pointer this is. 32 bit fat pointers are restricted to having an ID
// that does not set the most significant bit. Therefore they must be less than 2^31 or the
// maximum signed int value (INT32_MAX). Conversely, the 64 bit ids must have this bit set.
// This is the slab allocator's responsibility to ensure the bit is set correctly.
// Users need not worry about these details as they are handled internally.
bool fat_ptr_is64(const FatPtr *ptr) {
	return ((FatPtr32Impl *)ptr->data)->id & 0x1;
}

// Return the id of the FatPtr. First check bit for u64/u32 status, then return value.
u64 fat_ptr_id(const FatPtr *ptr) {
	if (fat_ptr_is64(ptr))
		return ((FatPtr64Impl *)ptr->data)->id;
	else
		return ((FatPtr32Impl *)ptr->data)->id;
}

// Get the length of this FatPtr. Once again use the status bit in the id then return value.
u64 fat_ptr_len(const FatPtr *ptr) {
	if (fat_ptr_is64(ptr))
		return ((FatPtr64Impl *)ptr->data)->len;
	else
		return ((FatPtr32Impl *)ptr->data)->len;
}

// Get the data pointer of this FatPtr. Once again use the status bit in the id then return value.
void *fat_ptr_data(const FatPtr *ptr) {
	if (fat_ptr_is64(ptr))
		return ((FatPtr64Impl *)ptr->data)->data;
	else
		return ((FatPtr32Impl *)ptr->data)->data;
}

// Test function to free the 64 bit objects.
void fat_ptr_free_test_obj64(FatPtr *ptr) {
	if (ptr && ptr->data) {
		myfree(ptr->data);
		ptr->data = NULL;
	}
}

// Test function to free the 32 bit objects.
void fat_ptr_free_test_obj32(FatPtr *ptr) {
	if (ptr && ptr->data) {
		myfree(ptr->data);
		ptr->data = NULL;
	}
}

// Test function to allocate the 64 bit objects.
void fat_ptr_test_obj64(FatPtr *ptr, u64 id, u64 len) {
	ptr->data = mymalloc(3 * sizeof(u64) + len);
	FatPtr64Impl *fptr = ptr->data;
	fptr->id = id;
	fptr->len = len;
	fptr->data = ptr->data + 3 * sizeof(u64);
}

// Test function to allocate the 32 bit objects.
void fat_ptr_test_obj32(FatPtr *ptr, u32 id, u32 len) {
	ptr->data = mymalloc(2 * sizeof(u32) + sizeof(u64) + len);
	FatPtr32Impl *fptr = ptr->data;
	fptr->id = id;
	fptr->len = len;
	fptr->data = ptr->data + 2 * sizeof(u32) + sizeof(u64);
}

// SlabAllocator Config
void slab_allocator_config_cleanup(SlabAllocatorConfigNc *sc) {
	if (sc->slab_types) {
		myfree(sc->slab_types);
		sc->slab_types = NULL;
	}
}

int slab_allocator_config_build(SlabAllocatorConfig *sc, bool zeroed, bool no_malloc,
								bool is_64_bit) {
	sc->zeroed = zeroed;
	sc->no_malloc = no_malloc;
	sc->is_64_bit = is_64_bit;
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
	u64 cur_chunks;		// number of chunks currently allocated
	u64 cur_slabs;		// number of slabs currently allocated
	u64 free_list_head; // the free list head
} SlabData;

typedef struct SlabAllocatorImpl {
	u64 sd_size;	  // size of the SlabData array
	SlabData *sd_arr; // slab data array one for each SlabType
	bool no_malloc;	  // config no_malloc
	bool zeroed;	  // config zeroed
	bool is_64_bit;	  // config 64 bit id/len
	u64 cur_mallocs;  // number of slabs allocated via malloc
} SlabAllocatorImpl;

int slab_data_allocate(SlabData *sd, FatPtr *fptr, bool is_64_bit, bool zeroed);

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

int slab_allocator_init_free_list(SlabData *sd, u64 offset, bool is_64_bit) {
	u64 slabs = sd->type.initial_chunks * sd->type.slabs_per_resize;
	// initialize the values of the free list.
	for (u64 i = 0; i < slabs; i++) {
		if (i == (slabs - 1)) {
			if (is_64_bit)
				((u64 *)(sd->free_list))[i + offset] = UINT64_MAX;
			else
				((u32 *)(sd->free_list))[i + offset] = UINT32_MAX;
		} else {
			if (is_64_bit)
				((u64 *)(sd->free_list))[i + offset] = offset + i + 1;
			else
				((u32 *)(sd->free_list))[i + offset] = offset + i + 1;
		}
	}

	return 0;
}

int slab_allocator_init_slab_data(SlabAllocatorImpl *impl, SlabType *st, u64 index) {
	impl->sd_arr[index].type = *st;
	impl->sd_arr[index].cur_slabs = 0;
	impl->sd_arr[index].cur_chunks = st->initial_chunks;
	impl->sd_arr[index].free_list_head = 0;
	if (st->initial_chunks > 0)
		impl->sd_arr[index].data = mymalloc(sizeof(void *) * st->initial_chunks);
	else
		impl->sd_arr[index].data = NULL;
	if (impl->sd_arr[index].data == NULL)
		return -1;

	if (st->initial_chunks > 0) {
		if (impl->is_64_bit)
			impl->sd_arr[index].free_list =
				mymalloc(sizeof(u64) * st->slabs_per_resize * st->initial_chunks);
		else
			impl->sd_arr[index].free_list =
				mymalloc(sizeof(u32) * st->slabs_per_resize * st->initial_chunks);
	} else {
		impl->sd_arr[index].free_list = NULL;
	}

	if (impl->sd_arr[index].free_list == NULL) {
		myfree(impl->sd_arr[index].data);
		return -1;
	}

	for (u64 i = 0; i < st->initial_chunks; i++) {
		impl->sd_arr[index].data[i] = mymalloc(st->slab_size * st->slabs_per_resize);

		if (impl->sd_arr[index].data[i] == NULL) {
			for (u64 j = 0; j < i; j++) {
				myfree(impl->sd_arr[index].data[j]);
			}
			myfree(impl->sd_arr[index].data);
			return -1;
		}
	}

	bool is_64_bit = impl->is_64_bit;
	slab_allocator_init_free_list(&impl->sd_arr[index], 0, is_64_bit);

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
		if (last_slab_size >= impl->sd_arr[i].type.slab_size) {
			errno = EINVAL;
			return -1;
		}
		// for alignment, must be divisible by 8
		if (impl->sd_arr[i].type.slab_size % 8 != 0) {
			errno = EINVAL;
			return -1;
		}
		// can't have higher max slabs than initial
		if (impl->sd_arr[i].type.slabs_per_resize * impl->sd_arr[i].type.initial_chunks >
			impl->sd_arr[i].type.max_slabs) {
			errno = EINVAL;
			return -1;
		}
		// max slabs for 32 bit and 64 bit
		if (impl->sd_arr[i].type.max_slabs > (INT32_MAX - 10) && !impl->is_64_bit) {
			errno = EINVAL;
			return -1;
		} else if (impl->sd_arr[i].type.max_slabs > (INT64_MAX - 10) && impl->is_64_bit) {
			errno = EINVAL;
			return -1;
		}

		// max slabs must be dividisble by slabs_per_resize
		if (impl->sd_arr[i].type.max_slabs % impl->sd_arr[i].type.slabs_per_resize) {
			errno = EINVAL;
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

	impl->no_malloc = config->no_malloc;
	impl->zeroed = config->zeroed;
	impl->is_64_bit = config->is_64_bit;
	impl->sd_size = config->slab_types_count;
	impl->sd_arr = mymalloc(sizeof(SlabData) * impl->sd_size);
	impl->cur_mallocs = 0;
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
	if (impl->sd_size == 0)
		return ret;

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
		return right + 1;
	}

	return ret;
}

int slab_data_try_resize(SlabData *sd, FatPtr *fptr, bool is_64_bit, bool zeroed) {
	if (sd->cur_slabs < sd->type.max_slabs) {
		// we can try to resize

		// first try to reallocate the free list
		void *nfree_list = NULL;
		u64 factor;
		if (is_64_bit)
			factor = sizeof(u64);
		else
			factor = sizeof(u32);
		if (sd->free_list) {
			nfree_list =
				myrealloc(sd->free_list, (1 + sd->cur_chunks) * sd->type.slabs_per_resize * factor);
		} else {

			nfree_list = mymalloc(sd->type.slabs_per_resize * factor);
		}
		if (nfree_list == NULL)
			return -1;
		sd->free_list = nfree_list;

		// now try to allocate more slab chunks
		void *ndata;
		if (sd->data == NULL) {
			ndata = mymalloc(sizeof(void *));
		} else {
			ndata = myrealloc(sd->data, sizeof(void *) * (1 + sd->cur_chunks));
		}
		if (ndata == NULL) {
			return -1;
		}

		sd->data = ndata;
		sd->data[sd->cur_chunks] = mymalloc(sd->type.slab_size * sd->type.slabs_per_resize);
		if (sd->data[sd->cur_chunks] == NULL)
			return -1;

		// data was successfully allocated, make updates.
		slab_allocator_init_free_list(sd, sd->cur_slabs, is_64_bit);
		sd->free_list_head = sd->cur_slabs;
		sd->cur_chunks += 1;

		return slab_data_allocate(sd, fptr, is_64_bit, zeroed);
	}
	return -1;
}

int slab_data_allocate(SlabData *sd, FatPtr *fptr, bool is_64_bit, bool zeroed) {
	u64 id = sd->free_list_head;
	if (is_64_bit && id == UINT64_MAX) {
		return slab_data_try_resize(sd, fptr, is_64_bit, zeroed);
	} else if (!is_64_bit && id == UINT32_MAX) {
		return slab_data_try_resize(sd, fptr, is_64_bit, zeroed);
	}

	if (is_64_bit) {
		sd->free_list_head = ((u64 *)sd->free_list)[id];

	} else {
		sd->free_list_head = ((u32 *)sd->free_list)[id];
	}

	// set to MAX-1 as a marker that this is allocated
	// if any freed without this value, it's an error
	if (is_64_bit)
		((u64 *)sd->free_list)[id] = (UINT64_MAX - 1);
	else
		((u32 *)sd->free_list)[id] = (UINT32_MAX - 1);

	u64 len = sd->type.slab_size;
	u64 slab_data_index = id / sd->type.slabs_per_resize;
	u64 offset_mod = id % sd->type.slabs_per_resize;

	fptr->data = sd->data[slab_data_index] + offset_mod * sd->type.slab_size;

	u64 data_len;
	u8 *data_ptr;
	if (is_64_bit) {
		FatPtr64Impl *fptr64 = fptr->data;
		fptr64->id = (id * 2) + 1;
		fptr64->len = len - 3 * sizeof(u64);
		fptr64->data = fptr->data + 3 * sizeof(u64);
		data_len = fptr64->len;
		data_ptr = fptr64->data;
	} else {
		FatPtr32Impl *fptr32 = fptr->data;
		fptr32->id = (id * 2);
		fptr32->len = len - (2 * sizeof(u32) + sizeof(u64));
		fptr32->data = fptr->data + 2 * sizeof(u32) + sizeof(u64);
		data_len = fptr32->len;
		data_ptr = fptr32->data;
	}

	sd->cur_slabs += 1;
	if (zeroed) {
		for (u64 i = 0; i < data_len; i++) {
			data_ptr[i] = 0;
		}
	}

	return 0;
}

void slab_data_free(SlabData *sd, const FatPtr *fptr, bool is_64_bit, bool zeroed) {
	u64 data_len;
	u8 *data_ptr;
	if (is_64_bit) {
		FatPtr64Impl *fptr64 = fptr->data;
		data_len = fptr64->len;
		data_ptr = fptr64->data;
		u64 id = (fptr64->id - 1) / 2;
		if (((u64 *)sd->free_list)[id] != (UINT64_MAX - 1)) {
			panic("Potential double free. Id = %llu.\n", id);
		}
		((u64 *)sd->free_list)[id] = sd->free_list_head;
		sd->free_list_head = id;
		if (sd->cur_slabs == 0)
			panic("Potential double free. Id = %llu. Freeing slabs when none are allocated.\n", id);
		sd->cur_slabs -= 1;
	} else {
		FatPtr32Impl *fptr32 = fptr->data;
		data_len = fptr32->len;
		data_ptr = fptr32->data;
		u32 id = (fptr32->id) / 2;
		if (((u32 *)sd->free_list)[id] != (UINT32_MAX - 1)) {
			panic("Potential double free. Id = %llu.\n", id);
		}
		((u32 *)sd->free_list)[id] = sd->free_list_head;
		sd->free_list_head = id;
		if (sd->cur_slabs == 0)
			panic("Potential double free. Id = %llu. Freeing slabs when none are allocated.\n", id);
		sd->cur_slabs -= 1;
	}

	if (zeroed) {
		for (u64 i = 0; i < data_len; i++) {
			data_ptr[i] = 0;
		}
	}
}

int slab_allocator_allocate(SlabAllocator *ptr, u32 size, FatPtr *fptr) {
	SlabAllocatorImpl *impl = ((SlabAllocatorImpl *)ptr->impl);
	bool is_64_bit = impl->is_64_bit;
	bool zeroed = impl->zeroed;
	u32 needed;
	if (is_64_bit)
		needed = size + 3 * sizeof(u64);
	else
		needed = size + 2 * sizeof(u32) + sizeof(u64);
	int index = slab_allocator_index(ptr, needed);
	if (index < 0) {
		return -1;
	}
	SlabData *sd = &impl->sd_arr[index];
	int ret = slab_data_allocate(sd, fptr, is_64_bit, zeroed);

	// we couldn't allocate via slabs. Use malloc if configured.
	if (ret && !impl->no_malloc) {
		void *val;
		u64 data_len;

		if (is_64_bit) {
			data_len = size + (3 * sizeof(u64));
			val = mymalloc(data_len);
		} else {
			data_len = size + (2 * sizeof(u32) + sizeof(u64));
			val = mymalloc(data_len);
		}

		// malloc error
		if (!val)
			return -1;

		if (zeroed) {
			for (u64 i = 0; i < data_len; i++) {
				((u8 *)val)[i] = 0;
			}
		}

		impl->cur_mallocs++;
		ret = 0;
		fptr->data = val;

		if (is_64_bit) {
			FatPtr64Impl *fptr64 = fptr->data;
			fptr64->id = MALLOC_64_ID;
			fptr64->len = size;
			fptr64->data = fptr->data + 3 * sizeof(u64);
		} else {
			FatPtr32Impl *fptr32 = fptr->data;
			fptr32->id = MALLOC_32_ID;
			fptr32->len = size;
			fptr32->data = fptr->data + 2 * sizeof(u32) + sizeof(u64);
		}
	}
	return ret;
}
void slab_allocator_free(SlabAllocator *ptr, const FatPtr *fptr) {
	SlabAllocatorImpl *impl = ptr->impl;
	bool is_64_bit = impl->is_64_bit;

	if (is_64_bit) {
		FatPtr64Impl *fptr64 = fptr->data;
		if (fptr64->id == MALLOC_64_ID) {
			if (fptr->data) {
				if (impl->zeroed) {
					for (u64 i = 0; i < fptr64->len + 3 * sizeof(u64); i++) {
						((u8 *)fptr->data)[i] = 0;
					}
				}
				if (impl->cur_mallocs == 0)
					panic("Freeing malloc allocated pointer when none are pending. Double free?");
				myfree(fptr->data);
				impl->cur_mallocs--;
			}
			return;
		}
	} else {
		FatPtr32Impl *fptr32 = fptr->data;
		if (fptr32->id == MALLOC_32_ID) {
			if (fptr->data) {
				if (impl->zeroed) {
					for (u64 i = 0; i < fptr32->len + 2 * sizeof(u32) + sizeof(u64); i++) {
						((u8 *)fptr->data)[i] = 0;
					}
				}
				if (impl->cur_mallocs == 0)
					panic("Freeing malloc allocated pointer when none are pending. Double free?");
				myfree(fptr->data);
				impl->cur_mallocs--;
			}
			return;
		}
	}

	u32 needed;
	if (is_64_bit)
		needed = fat_ptr_len(fptr) + 3 * sizeof(u64);
	else
		needed = fat_ptr_len(fptr) + 2 * sizeof(u32) + sizeof(u64);
	int index = slab_allocator_index(ptr, needed);
	if (index < 0)
		panic("Freeing a slab with an unknown slab size %llu.\n", needed);
	SlabData *sd = &((SlabAllocatorImpl *)ptr->impl)->sd_arr[index];
	if (needed != sd->type.slab_size)
		panic("Freeing a slab with an unknown slab size %llu.\n", needed);
	slab_data_free(sd, fptr, ((SlabAllocatorImpl *)ptr->impl)->is_64_bit,
				   ((SlabAllocatorImpl *)ptr->impl)->zeroed);
}
u64 slab_allocator_cur_slabs_allocated(const SlabAllocator *ptr) {
	SlabAllocatorImpl *impl = ptr->impl;
	u64 ret = impl->cur_mallocs;

	for (u64 i = 0; i < impl->sd_size; i++) {
		ret += impl->sd_arr[i].cur_slabs;
	}
	return ret;
}
