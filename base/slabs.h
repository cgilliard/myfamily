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

#include <base/macro_util.h>
#include <base/types.h>

// Ptr data type

typedef struct PtrImpl *Ptr;

int ptr_len(const Ptr ptr);
number ptr_id(const Ptr ptr);
void *ptr_data(const Ptr ptr);
bool ptr_flag_check(const Ptr ptr, byte flag);
void ptr_flag_set(const Ptr ptr, byte flag, bool value);
bool ptr_is_nil(const Ptr ptr);

#ifdef TEST
Ptr ptr_test_obj(int id, int len, byte flags);
void ptr_free_test_obj(Ptr ptr);
#endif // TEST

// Slab Allocator

// Slab Type definition
typedef struct SlabType {
	number slab_size;
	number slabs_per_resize;
	number initial_chunks;
	number max_slabs;
} SlabType;

// Slab Allocator configuration
typedef struct SlabAllocatorConfigNc {
	number slab_types_count;
	SlabType *slab_types;
} SlabAllocatorConfigNc;

void slab_allocator_config_cleanup(SlabAllocatorConfigNc *ptr);

#define SlabAllocatorConfig                                                                        \
	SlabAllocatorConfigNc                                                                          \
		__attribute__((warn_unused_result, cleanup(slab_allocator_config_cleanup)))

int slab_allocator_config_build(SlabAllocatorConfig *sc);
int slab_allocator_config_add_type(SlabAllocatorConfig *sc, const SlabType *st);

typedef struct SlabAllocatorImpl *SlabAllocatorNc;

void slab_allocator_cleanup(SlabAllocatorNc *ptr);

#define SlabAllocator                                                                              \
	SlabAllocatorNc __attribute__((warn_unused_result, cleanup(slab_allocator_cleanup)))

SlabAllocator slab_allocator_create(SlabAllocatorConfig *sc);
void slab_allocator_print(SlabAllocator ptr);

#endif // _BASE_SLABS__
