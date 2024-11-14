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
#include <base/memmap.h>
#include <base/types.h>

// Slab Allocator
#define SA_IMPL_SIZE 52
typedef struct SlabAllocator {
	byte impl[SA_IMPL_SIZE];
} SlabAllocator;

byte *slab_get(const SlabAllocator *sa, Ptr p);
unsigned long long *slab_aux(SlabAllocator *sa, Ptr p);

int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						unsigned long long max_free_slabs,
						unsigned long long max_total_slabs);
void slab_allocator_cleanup(SlabAllocator *sa);
Ptr slab_allocator_allocate(SlabAllocator *sa);
void slab_allocator_free(SlabAllocator *sa, Ptr ptr);

#ifdef TEST
unsigned long long slab_allocator_free_size(SlabAllocator *sa);
unsigned long long slab_allocator_total_slabs(SlabAllocator *sa);
void set_debug_sa1();
void set_debug_sa2();
#endif	// TEST

#endif	// _BASE_SLABS__
