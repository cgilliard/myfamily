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

#define SLAB_ALLOCATOR_IMPL_SIZE 128
typedef struct SlabAllocator {
	byte impl[SLAB_ALLOCATOR_IMPL_SIZE];
} SlabAllocator;

#define SLAB_LIST_SIZE 32
typedef struct Slab {
	byte impl[SLAB_LIST_SIZE];
	byte data[];
} Slab;

int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						unsigned int max_free_slabs,
						unsigned int max_total_slabs);
void slab_allocator_cleanup(SlabAllocator *sa);
Slab *slab_allocator_allocate(SlabAllocator *sa);
void slab_allocator_free(SlabAllocator *sa, Slab *slab);

#endif	// _BASE_SLABS__
