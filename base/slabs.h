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
#include <base/ptr.h>
#include <base/types.h>

#define MAX_SLAB_SIZE 65536

// Slab Allocator
typedef struct SlabAllocatorImpl *SlabAllocatorNc;

void slab_allocator_cleanup(SlabAllocatorNc *ptr);

#define SlabAllocator \
	SlabAllocatorNc   \
		__attribute__((warn_unused_result, cleanup(slab_allocator_cleanup)))

SlabAllocator slab_allocator_create();
Ptr slab_allocator_allocate(SlabAllocator sa, unsigned int size);
void slab_allocator_free(SlabAllocator sa, Ptr ptr);
int64 slab_allocator_cur_slabs_allocated(const SlabAllocator sa);
Ptr ptr_for(SlabAllocator sa, unsigned int id, unsigned int len);

#ifdef TEST
int slab_allocator_get_size(unsigned int index);
int slab_allocator_get_index(unsigned int size);
int slab_getpagesize();
unsigned int slab_overhead();
#endif	// TEST

#endif	// _BASE_SLABS__
