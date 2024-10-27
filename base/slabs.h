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
extern const struct PtrImpl null;

typedef struct Object {
	Ptr data;	// Pointer to slab-allocated data
	byte flags; // Object-specific flags
} Object;

int ptr_len(const Ptr ptr);
int ptr_id(const Ptr ptr);
void *ptr_data(const Ptr ptr);
bool ptr_flag_check(const Ptr ptr, byte flag);
void ptr_flag_set(Ptr ptr, byte flag, bool value);
bool ptr_is_nil(const Ptr ptr);
void ptr_count1_set(Ptr ptr, int value);
void ptr_count2_set(Ptr ptr, int value);
void ptr_count1_incr(Ptr ptr, int value);
void ptr_count2_incr(Ptr ptr, int value);
void ptr_count1_decr(Ptr ptr, int value);
void ptr_count2_decr(Ptr ptr, int value);
int ptr_count1_get(Ptr ptr);
int ptr_count2_get(Ptr ptr);

#ifdef TEST
Ptr ptr_test_obj(int64 id, int64 len, byte flags);
void ptr_free_test_obj(Ptr ptr);
#endif // TEST

// Slab Allocator Config

// Slab Type definition
typedef struct SlabType {
	int slab_size;
	int64 slabs_per_resize;
	int64 initial_chunks;
	unsigned int max_slabs;
} SlabType;

typedef struct SlabAllocatorConfigImpl *SlabAllocatorConfigNc;

void slab_allocator_config_cleanup(SlabAllocatorConfigNc *ptr);

#define INIT_SLAB_ALLOCAOTR_CONFIG {NULL}
#define SlabAllocatorConfig                                                                        \
	SlabAllocatorConfigNc                                                                          \
		__attribute__((warn_unused_result, cleanup(slab_allocator_config_cleanup)))

SlabAllocatorConfig slab_allocator_config_create();
int64 slab_allocator_config_add_type(SlabAllocatorConfig sc, const SlabType *st);

// Slab Allocator

typedef struct SlabAllocatorImpl *SlabAllocatorNc;

void slab_allocator_cleanup(SlabAllocatorNc *ptr);

#define SlabAllocator                                                                              \
	SlabAllocatorNc __attribute__((warn_unused_result, cleanup(slab_allocator_cleanup)))

SlabAllocator slab_allocator_create();
Ptr slab_allocator_allocate(SlabAllocator sa, int64 size);
void slab_allocator_free(SlabAllocator sa, Ptr ptr);
int64 slab_allocator_cur_slabs_allocated(const SlabAllocator sa);

#endif // _BASE_SLABS__
