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

#ifndef _BASE_SLABS__
#define _BASE_SLABS__

#include <base/types.h>

// FatPtr data type
typedef struct FatPtr {
	void *data;
} FatPtr;

// Internal representation for null
typedef struct FatPtrZeroImpl {
	u128 internal;
} FatPtrZeroImpl;

// Global constant that points to a zeroed-out implementation of FatPtrImpl
static const FatPtrZeroImpl fatptr_impl_null = {0};
#define null                                                                                       \
	(const FatPtr) {                                                                               \
		.data = (void *)&fatptr_impl_null                                                          \
	}

#define nil(v) (fat_ptr_len(&v) == 0)

// A slab allocator will always allocate these, so the user only needs to
// be able to read the length and the data pointer.
u64 fat_ptr_len(const FatPtr *ptr);
void *fat_ptr_data(const FatPtr *ptr);

// Slab Allocator

// Slab Type definition
typedef struct SlabType {
	u64 slab_size;
	u32 slabs_per_resize;
	u32 initial_chunks;
	u64 max_slabs;
} SlabType;

// Slab Allocator configuration
typedef struct SlabAllocatorConfigNc {
	bool zeroed;
	bool no_malloc;
	bool is_64_bit;
	u64 slab_types_count;
	SlabType *slab_types;
} SlabAllocatorConfigNc;

void slab_allocator_config_cleanup(SlabAllocatorConfigNc *ptr);

#define SlabAllocatorConfig                                                                        \
	SlabAllocatorConfigNc                                                                          \
		__attribute__((warn_unused_result, cleanup(slab_allocator_config_cleanup)))

int slab_allocator_config_build(SlabAllocatorConfig *sc, bool zeored, bool no_malloc,
								bool is_64_bit);
int slab_allocator_config_add_type(SlabAllocatorConfig *sc, const SlabType *st);

typedef struct SlabAllocatorNc {
	void *impl;
} SlabAllocatorNc;

void slab_allocator_cleanup(SlabAllocatorNc *ptr);

#define SlabAllocator                                                                              \
	SlabAllocatorNc __attribute__((warn_unused_result, cleanup(slab_allocator_cleanup)))

int slab_allocator_build(SlabAllocator *ptr, const SlabAllocatorConfig *config);
int slab_allocator_allocate(SlabAllocator *ptr, u32 size, FatPtr *fptr);
void slab_allocator_free(SlabAllocator *ptr, FatPtr *fptr);
u64 slab_allocator_cur_slabs_allocated(const SlabAllocator *ptr);

// These are test helper functions
#ifdef TEST
u64 fat_ptr_id(const FatPtr *ptr);
void fat_ptr_test_obj64(FatPtr *ptr, u64 id, u64 len);
void fat_ptr_free_test_obj64(FatPtr *ptr);
void fat_ptr_test_obj32(FatPtr *ptr, u32 id, u32 len);
void fat_ptr_free_test_obj32(FatPtr *ptr);
#endif // TEST

#endif // _BASE_SLABS__
