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

#include <base/resources.h>
#include <stdio.h>
#include <util/panic.h>
#include <util/slabs.h>

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

bool is_little_endian() {
	u16 test = 0x1;
	return (*(u8 *)&test == 0x1);
}

// deteremine which kind of fat pointer this is. 32 bit fat pointers are restricted to having an ID
// that does not set the most significant bit. Therefore they must be less than 2^31 or the
// maximum signed int value (INT32_MAX). Conversely, the 64 bit ids must have this bit set.
// This is the slab allocator's responsibility to ensure the bit is set correctly.
// Users need not worry about these details as they are handled internally.
bool fat_ptr_is64(const FatPtr *ptr) {
	if (is_little_endian()) {
		return ((FatPtr32Impl *)ptr->data)->id & 0x1;
	} else {
		return ((FatPtr32Impl *)ptr->data)->id & 0x80000000;
	}
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
	ptr->data = mymalloc(2 * sizeof(u64) + len);
	FatPtr64Impl *fptr = ptr->data;
	fptr->id = id;
	fptr->len = len;
	fptr->data = ptr->data + 2 * sizeof(u64);
}

// Test function to allocate the 32 bit objects.
void fat_ptr_test_obj32(FatPtr *ptr, u32 id, u32 len) {
	ptr->data = mymalloc(2 * sizeof(u32) + len);
	FatPtr32Impl *fptr = ptr->data;
	fptr->id = id;
	fptr->len = len;
	fptr->data = ptr->data + 2 * sizeof(u32);
}

// map a sequential id to the 64 bit version for our slab allocator
// slab allocators should use this function to map ids sequentially when configured to 64 bit
// 0 -> 1, 1 -> 3, 2 -> 5,, ... slab allocators may use up to 2^63. Beyond that is out of bounds.
// on big endian systems the MSB is set
u64 map_id_64(u64 seq_id) {
	if (seq_id & 0x8000000000000000)
		panic("slab allocator: invalid 64 bit id");
	if (is_little_endian()) {
		return (seq_id * 2) + 1;
	} else {
		return seq_id | 0x8000000000000000;
	}
}

// map a sequential id to the 32 bit version for our slab allocator
// slab allocators should use this function to map ids sequentially when configured to 32 bit
// 0 -> 0, 1 -> 2, 2 -> 4,, ... slab allocators may use up to 2^31. Beyond that is out of bounds.
// on big endian systems the MSB is unset
u32 map_id_32(u32 seq_id) {
	if (seq_id & 0x80000000)
		panic("slab allocator: invalid 32 bit id");
	if (is_little_endian()) {
		return seq_id * 2;
	} else {
		return seq_id & 0x7FFFFFFF;
	}
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
	SlabType type;	// This slab data's type information (slab_size, slabs_per_resize, initial, max)
	void **data;	// Pointers to each chunk
	u32 *free_list; // The free list pointers
	u32 cur_chunks; // number of chunks currently allocated
	u32 cur_slabs;	// number of slabs currently allocated
	u32 free_list_head; // the free list head
} SlabData;

typedef struct SlabAllocatorImpl {
	u32 sd_size;	  // size of the SlabData array
	SlabData *sd_arr; // slab data array one for each SlabType
	u32 cur_malloc;	  // number of slabs allocated via malloc
	bool no_malloc;	  // config no_malloc
	bool zeroed;	  // config zeroed
	bool is_64_bit;	  // config 64 bit id/len

} SlabAllocatorImpl;

void slab_allocator_cleanup(SlabAllocatorNc *ptr) {
	if (ptr->impl) {
		SlabAllocatorImpl *impl = ptr->impl;
		if (impl->sd_arr) {
			for (u64 i = 0; i < impl->sd_size; i++) {
				for (int j = 0; j < impl->sd_arr[i].cur_chunks; j++) {
					myfree(impl->sd_arr[i].data[j]);
				}
				myfree(impl->sd_arr[i].data);
			}
			myfree(impl->sd_arr);
			impl->sd_arr = NULL;
		}
		myfree(ptr->impl);
		ptr->impl = NULL;
	}
}

int slab_allocator_init_slab_data(SlabAllocatorImpl *impl, SlabType st, u64 index) {
	impl->sd_arr[index].cur_slabs = 0;
	impl->sd_arr[index].cur_chunks = st.initial_chunks;
	impl->sd_arr[index].free_list_head = 0;
	impl->sd_arr[index].data = mymalloc(sizeof(void *) * st.initial_chunks);
	if (impl->sd_arr[index].data == NULL)
		return -1;
	for (u64 i = 0; i < st.initial_chunks; i++) {
		impl->sd_arr[index].data[i] = mymalloc(st.slab_size * st.slabs_per_resize);
		if (impl->sd_arr[index].data[i] == NULL) {
			for (u64 j = 0; j < i; j++) {
				myfree(impl->sd_arr[index].data[j]);
			}
			myfree(impl->sd_arr[index].data);
			return -1;
		}
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
	if (impl->sd_arr == NULL) {
		myfree(ptr->impl);
		return -1;
	}

	for (u64 i = 0; i < config->slab_types_count; i++) {
		if (slab_allocator_init_slab_data(impl, config->slab_types[i], i))
			return -1;
	}

	return 0;
}
int slab_allocator_allocate(SlabAllocator *ptr, u32 size, FatPtr *fptr) {
	return 0;
}
void slab_allocator_free(SlabAllocator *ptr, const FatPtr *fptr) {
}
u64 slab_allocator_cur_slabs_allocated(const SlabAllocator *ptr) {
	return 0;
}
