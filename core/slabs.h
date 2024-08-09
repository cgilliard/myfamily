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

#ifndef _CORE_SLABS__
#define _CORE_SLABS__

#include <core/macro_utils.h>
#include <core/types.h>
#include <limits.h>
#include <stdlib.h>

typedef struct SlabDataParams {
	u64 slab_size;
	u64 slabs_per_resize;
	u64 initial_chunks;
	u64 max_slabs;
	u64 free_list_head;
} SlabDataParams;

typedef struct SlabDataPtr {
	void **data;
	u64 count;
	u64 cur_slabs;
	SlabDataParams sdp;
} SlabDataPtr;

#define SlabData                                                               \
	SlabDataPtr                                                            \
	    __attribute__((warn_unused_result, cleanup(slab_data_cleanup)))

typedef struct Slab {
	u64 id;
	void *data;
	u64 len;
} Slab;

void slab_data_cleanup(SlabData *ptr);
int slab_data_build(SlabData *ptr, SlabDataParams sdp);
int slab_data_access(SlabData *ptr, Slab *slab, u64 offset);
int slab_data_resize(SlabData *ptr);

void slab_data_cleanup(SlabData *ptr);
int slab_data_build(SlabData *ptr, SlabDataParams sdp);
int slab_data_access(SlabData *ptr, Slab *slab, u64 offset);
int slab_data_resize(SlabData *ptr);

typedef struct SlabAllocatorPtr {
	bool initialized;
	SlabDataPtr *slab_data_arr;
	u64 slab_data_arr_size;
	bool zeroed;
	struct SlabAllocatorPtr *prev;
	bool no_malloc;
} SlabAllocatorPtr;
#define SlabAllocator                                                          \
	SlabAllocatorPtr __attribute__((warn_unused_result,                    \
					cleanup(slab_allocator_cleanup)))

#define SLAB_ALLOCATOR_UNINIT {false}

void slab_allocator_cleanup(SlabAllocator *ptr);
int slab_allocator_build(SlabAllocator *ptr, bool zeroed, int slab_data_count,
			 ...);
u64 slab_allocator_allocate(SlabAllocator *ptr, u64 size);
int slab_allocator_get(SlabAllocator *ptr, Slab *slab, u64 id);
int slab_allocator_free(SlabAllocator *ptr, u64 id);

typedef enum SlabAllocatorConfigType {
	SASlabCount = 0,
	SASlabSize = 1,
	SASlabsPerResize = 2,
	SAMaxSlabs = 3,
} SlabAllocatorConfigType;

typedef struct SlabAllocatorConfig {
	SlabAllocatorConfigType type;
	u64 value;
} SlabAllocatorConfig;

#define SlabCount(num) ({ _sdp__.initial_chunks = num; })

#define SlabSize(num) ({ _sdp__.slab_size = num; })

#define SlabsPerResize(num) ({ _sdp__.slabs_per_resize = num; })

#define MaxSlabs(num) ({ _sdp__.max_slabs = num; })

#define UPDATE_SLAB_PARAMS(value) ({ value; })

#define SLAB_PARAMS(...)                                                       \
	({                                                                     \
		SlabDataParams _sdp__;                                         \
		_sdp__.free_list_head = 0;                                     \
		_sdp__.slab_size = 512;                                        \
		_sdp__.initial_chunks = 1;                                     \
		_sdp__.max_slabs = 100;                                        \
		_sdp__.slabs_per_resize = 10;                                  \
		EXPAND(FOR_EACH(UPDATE_SLAB_PARAMS, __VA_ARGS__));             \
		_sdp__;                                                        \
	})

#define SLABS(zeroed, ...)                                                     \
	({                                                                     \
		SlabAllocatorPtr _sa__;                                        \
		bool __no_malloc___ = false;                                   \
		int __counter___ = 0;                                          \
		EXPAND(FOR_EACH(COUNT_ARGS, __VA_ARGS__));                     \
		slab_allocator_build(&_sa__, zeroed,                           \
				     __counter___ __VA_OPT__(, ) __VA_ARGS__); \
		_sa__.no_malloc = false;                                       \
		_sa__;                                                         \
	})

#endif // _CORE_SLABS__
