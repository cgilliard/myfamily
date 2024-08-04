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

#ifndef _CORE_MYMALLOC__
#define _CORE_MYMALLOC__

#include <core/macro_utils.h>
#include <core/slabs.h>
#include <stddef.h>

extern _Thread_local SlabAllocator TL_SLAB_ALLOCATOR;
extern _Thread_local SlabAllocatorPtr *local_slab_allocator;

static void set_local_slab_allocator(SlabAllocatorPtr *ptr) {
	local_slab_allocator = ptr;
}

#define SLAB_ALLOCATOR(sa) set_local_slab_allocator(sa)
#define UNSET_SLAB_ALLOCATOR() set_local_slab_allocator(NULL)

#define SPMAX(x)                                                               \
	SLAB_PARAMS(SlabSize(x), SlabCount(0), MaxSlabs(UINT64_MAX),           \
		    SlabsPerResize(10))

static void __attribute__((constructor)) init_slab_allocator() {

	TL_SLAB_ALLOCATOR = SLABS(
	    false, SPMAX(1), SPMAX(2), SPMAX(3), SPMAX(4), SPMAX(5), SPMAX(6),
	    SPMAX(7), SPMAX(8), SPMAX(9), SPMAX(10), SPMAX(11), SPMAX(12),
	    SPMAX(13), SPMAX(14), SPMAX(15), SPMAX(16), SPMAX(17), SPMAX(18),
	    SPMAX(19), SPMAX(20), SPMAX(21), SPMAX(22), SPMAX(23), SPMAX(24),
	    SPMAX(25), SPMAX(26), SPMAX(27), SPMAX(28), SPMAX(29), SPMAX(30),
	    SPMAX(31), SPMAX(32), SPMAX(33), SPMAX(34), SPMAX(35), SPMAX(36),
	    SPMAX(37), SPMAX(38), SPMAX(39), SPMAX(40), SPMAX(41), SPMAX(42),
	    SPMAX(43), SPMAX(44), SPMAX(45), SPMAX(46), SPMAX(47), SPMAX(48),
	    SPMAX(49), SPMAX(50), SPMAX(51), SPMAX(52), SPMAX(53), SPMAX(54),
	    SPMAX(55), SPMAX(56), SPMAX(57), SPMAX(58), SPMAX(59), SPMAX(60),
	    SPMAX(61), SPMAX(62), SPMAX(63), SPMAX(64), SPMAX(72), SPMAX(80),
	    SPMAX(88), SPMAX(96), SPMAX(104), SPMAX(112), SPMAX(120),
	    SPMAX(128), SPMAX(136), SPMAX(144), SPMAX(152), SPMAX(160),
	    SPMAX(168), SPMAX(176), SPMAX(184), SPMAX(192), SPMAX(200),
	    SPMAX(208), SPMAX(216), SPMAX(224), SPMAX(232), SPMAX(240),
	    SPMAX(248), SPMAX(256), SPMAX(512));
}

typedef struct ResourceStats {
	u64 malloc_sum;
	u64 realloc_sum;
	u64 free_sum;
	u64 fopen_sum;
	u64 fclose_sum;
} ResourceStats;

int mymalloc(Slab *slab, u64 size);
int myrealloc(Slab *slab, u64 size);
int myfree(Slab *slab);
FILE *myfopen(const char *path, const char *mode);
void myfclose(FILE *ptr);
ResourceStats get_resource_stats();

#endif // _CORE_MYMALLOC__
