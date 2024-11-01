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

#define MAX_SLAB_SIZE 4096

// Ptr data type
typedef struct Type *Ptr;
extern const struct Type *null;
extern const struct Type *Ok;

unsigned int ptr_len(const Ptr ptr);
unsigned int ptr_id(const Ptr ptr);
void *ptr_data(const Ptr ptr);
void *ptr_aux(const Ptr ptr);

// Direct alloc
Ptr ptr_direct_alloc(unsigned int size);
void ptr_direct_release(Ptr ptr);

// Slab Allocator
typedef struct SlabAllocatorImpl *SlabAllocatorNc;

void slab_allocator_cleanup(SlabAllocatorNc *ptr);

#define SlabAllocator                                                                              \
	SlabAllocatorNc __attribute__((warn_unused_result, cleanup(slab_allocator_cleanup)))

SlabAllocator slab_allocator_create();
Ptr slab_allocator_allocate(SlabAllocator sa, unsigned int size);
void slab_allocator_free(SlabAllocator sa, Ptr ptr);
int64 slab_allocator_cur_slabs_allocated(const SlabAllocator sa);
Ptr ptr_for(SlabAllocator sa, unsigned int id, unsigned int len);
unsigned int slab_overhead();

#endif // _BASE_SLABS__
