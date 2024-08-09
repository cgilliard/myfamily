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

u64 next_greater_slab_size(u64 size);

static void set_local_slab_allocator(SlabAllocatorPtr *ptr, bool is_unset) {
	if (ptr && !is_unset)
		ptr->prev = local_slab_allocator;
	local_slab_allocator = ptr;
}

#define SLAB_ALLOCATOR(sa) set_local_slab_allocator(sa, false)
#define UNSET_SLAB_ALLOCATOR()                                                 \
	({                                                                     \
		if (local_slab_allocator)                                      \
			set_local_slab_allocator(local_slab_allocator->prev,   \
						 true);                        \
		else                                                           \
			set_local_slab_allocator(NULL, true);                  \
	})

#define SPMAX(x)                                                               \
	SLAB_PARAMS(SlabSize(x), InitialChunks(0), MaxSlabs(UINT64_MAX),       \
		    SlabsPerResize(10))

static u64 slab_sizes[] = {
    1,	 2,   3,   4,	5,   6,	  7,	8,    9,    10,	  11,	 12,   13,  14,
    15,	 16,  17,  18,	19,  20,  21,	22,   23,   24,	  25,	 26,   27,  28,
    29,	 30,  31,  32,	33,  34,  35,	36,   37,   38,	  39,	 40,   41,  42,
    43,	 44,  45,  46,	47,  48,  49,	50,   51,   52,	  53,	 54,   55,  56,
    57,	 58,  59,  60,	61,  62,  63,	64,   72,   80,	  88,	 96,   104, 112,
    120, 128, 136, 144, 152, 160, 168,	176,  184,  192,  200,	 208,  216, 224,
    232, 240, 248, 256, 272, 288, 304,	320,  336,  352,  368,	 384,  400, 416,
    432, 448, 464, 480, 496, 512, 1024, 2048, 4096, 8192, 16384, 65536};
static void init_tl_slab_allocator() {
	TL_SLAB_ALLOCATOR = SLABS(
	    false, SPMAX(slab_sizes[0]), SPMAX(slab_sizes[1]),
	    SPMAX(slab_sizes[2]), SPMAX(slab_sizes[3]), SPMAX(slab_sizes[4]),
	    SPMAX(slab_sizes[5]), SPMAX(slab_sizes[6]), SPMAX(slab_sizes[7]),
	    SPMAX(slab_sizes[8]), SPMAX(slab_sizes[9]), SPMAX(slab_sizes[10]),
	    SPMAX(slab_sizes[11]), SPMAX(slab_sizes[12]), SPMAX(slab_sizes[13]),
	    SPMAX(slab_sizes[14]), SPMAX(slab_sizes[15]), SPMAX(slab_sizes[16]),
	    SPMAX(slab_sizes[17]), SPMAX(slab_sizes[18]), SPMAX(slab_sizes[19]),
	    SPMAX(slab_sizes[20]), SPMAX(slab_sizes[21]), SPMAX(slab_sizes[22]),
	    SPMAX(slab_sizes[23]), SPMAX(slab_sizes[24]), SPMAX(slab_sizes[25]),
	    SPMAX(slab_sizes[26]), SPMAX(slab_sizes[27]), SPMAX(slab_sizes[28]),
	    SPMAX(slab_sizes[29]), SPMAX(slab_sizes[30]), SPMAX(slab_sizes[31]),
	    SPMAX(slab_sizes[32]), SPMAX(slab_sizes[33]), SPMAX(slab_sizes[34]),
	    SPMAX(slab_sizes[35]), SPMAX(slab_sizes[36]), SPMAX(slab_sizes[37]),
	    SPMAX(slab_sizes[38]), SPMAX(slab_sizes[39]), SPMAX(slab_sizes[40]),
	    SPMAX(slab_sizes[41]), SPMAX(slab_sizes[42]), SPMAX(slab_sizes[43]),
	    SPMAX(slab_sizes[44]), SPMAX(slab_sizes[45]), SPMAX(slab_sizes[46]),
	    SPMAX(slab_sizes[47]), SPMAX(slab_sizes[48]), SPMAX(slab_sizes[49]),
	    SPMAX(slab_sizes[50]), SPMAX(slab_sizes[51]), SPMAX(slab_sizes[52]),
	    SPMAX(slab_sizes[53]), SPMAX(slab_sizes[54]), SPMAX(slab_sizes[55]),
	    SPMAX(slab_sizes[56]), SPMAX(slab_sizes[57]), SPMAX(slab_sizes[58]),
	    SPMAX(slab_sizes[59]), SPMAX(slab_sizes[60]), SPMAX(slab_sizes[61]),
	    SPMAX(slab_sizes[62]), SPMAX(slab_sizes[63]), SPMAX(slab_sizes[64]),
	    SPMAX(slab_sizes[65]), SPMAX(slab_sizes[66]), SPMAX(slab_sizes[67]),
	    SPMAX(slab_sizes[68]), SPMAX(slab_sizes[69]), SPMAX(slab_sizes[70]),
	    SPMAX(slab_sizes[71]), SPMAX(slab_sizes[72]), SPMAX(slab_sizes[73]),
	    SPMAX(slab_sizes[74]), SPMAX(slab_sizes[75]), SPMAX(slab_sizes[76]),
	    SPMAX(slab_sizes[77]), SPMAX(slab_sizes[78]), SPMAX(slab_sizes[79]),
	    SPMAX(slab_sizes[80]), SPMAX(slab_sizes[81]), SPMAX(slab_sizes[82]),
	    SPMAX(slab_sizes[83]), SPMAX(slab_sizes[84]), SPMAX(slab_sizes[85]),
	    SPMAX(slab_sizes[86]), SPMAX(slab_sizes[87]), SPMAX(slab_sizes[88]),
	    SPMAX(slab_sizes[89]), SPMAX(slab_sizes[90]), SPMAX(slab_sizes[91]),
	    SPMAX(slab_sizes[92]), SPMAX(slab_sizes[93]), SPMAX(slab_sizes[94]),
	    SPMAX(slab_sizes[95]), SPMAX(slab_sizes[96]), SPMAX(slab_sizes[97]),
	    SPMAX(slab_sizes[98]), SPMAX(slab_sizes[99]),
	    SPMAX(slab_sizes[100]), SPMAX(slab_sizes[101]),
	    SPMAX(slab_sizes[102]), SPMAX(slab_sizes[103]),
	    SPMAX(slab_sizes[104]), SPMAX(slab_sizes[105]),
	    SPMAX(slab_sizes[106]), SPMAX(slab_sizes[107]),
	    SPMAX(slab_sizes[108]),
	    SLAB_PARAMS(SlabSize(65536), InitialChunks(0), MaxSlabs(UINT64_MAX),
			SlabsPerResize(1)));
}

typedef struct ResourceStats {
	u64 malloc_sum;
	u64 realloc_sum;
	u64 free_sum;
	u64 fopen_sum;
	u64 fclose_sum;
	u64 slab_misses;
} ResourceStats;

int mymalloc(Slab *slab, u64 size);
int myrealloc(Slab *slab, u64 size);
int myfree(Slab *slab);
FILE *myfopen(const char *path, const char *mode);
void myfclose(FILE *ptr);
ResourceStats get_resource_stats();

#endif // _CORE_MYMALLOC__
