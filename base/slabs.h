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

#include <base/lock.h>
#include <base/types.h>

// Slab Allocator
typedef struct SlabImpl *Slab;
typedef struct SlabAllocator {
	Slab head;
	Slab tail;
	unsigned long long free_size;
	unsigned long long max_free_slabs;
	unsigned long long total_slabs;
	unsigned long long max_total_slabs;
	unsigned int slab_size;
	bool free_check;
} SlabAllocator;

byte *slab_get(Slab s);
unsigned long long *slab_aux(Slab s);

int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						unsigned long long max_free_slabs,
						unsigned long long max_total_slabs, bool free_check);
void slab_allocator_cleanup(SlabAllocator *sa);
Slab slab_allocator_allocate(SlabAllocator *sa);
void slab_allocator_free(SlabAllocator *sa, Slab slab);

#ifdef TEST
unsigned long long slab_allocator_free_size(SlabAllocator *sa);
unsigned long long slab_allocator_total_slabs(SlabAllocator *sa);
#endif	// TEST

#endif	// _BASE_SLABS__
