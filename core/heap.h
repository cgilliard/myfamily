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

#ifndef _CORE_HEAP__
#define _CORE_HEAP__

#include <core/macro_utils.h>
#include <core/types.h>
#include <limits.h>
#include <stdlib.h>

typedef struct HeapDataParams {
	size_t slab_size;
	size_t slabs_per_resize;
	size_t initial_chunks;
	size_t max_slabs;
	size_t free_list_head;
} HeapDataParams;

typedef struct HeapDataPtr {
	void **data;
	size_t count;
	size_t cur_slabs;
	HeapDataParams sdp;
} HeapDataPtr;

void heap_data_cleanup(HeapDataPtr *ptr);

#define HeapData                                                               \
	HeapDataPtr                                                            \
	    __attribute__((warn_unused_result, cleanup(heap_data_cleanup)))

typedef struct FatPtr {
	size_t id;
	void *data;
	size_t len;
} FatPtr;

int heap_data_build(HeapData *ptr, HeapDataParams sdp);
int heap_data_access(HeapData *ptr, FatPtr *slab, size_t offset);
int heap_data_resize(HeapData *ptr);

void heap_data_cleanup(HeapData *ptr);
int heap_data_build(HeapData *ptr, HeapDataParams sdp);
int heap_data_access(HeapData *ptr, FatPtr *slab, size_t offset);
int heap_data_resize(HeapData *ptr);

typedef struct HeapAllocatorPtr {
	bool initialized;
	HeapDataPtr *heap_data_arr;
	size_t heap_data_arr_size;
	bool zeroed;
	struct HeapAllocatorPtr *prev;
	bool no_malloc;
} HeapAllocatorPtr;

void heap_allocator_cleanup(HeapAllocatorPtr *ptr);

#define HeapAllocator                                                          \
	HeapAllocatorPtr __attribute__((warn_unused_result,                    \
					cleanup(heap_allocator_cleanup)))

#define SLAB_ALLOCATOR_UNINIT {false}

int heap_allocator_build(HeapAllocator *ptr, bool zeroed, int heap_data_count,
			 ...);
size_t heap_allocator_allocate(HeapAllocator *ptr, size_t size);
int heap_allocator_get(HeapAllocator *ptr, FatPtr *slab, size_t id);
int heap_allocator_free(HeapAllocator *ptr, size_t id);

#define InitialChunks(num) ({ _sdp__.initial_chunks = num; })

#define SlabSize(num) ({ _sdp__.slab_size = num; })

#define SlabsPerResize(num) ({ _sdp__.slabs_per_resize = num; })

#define MaxSlabs(num) ({ _sdp__.max_slabs = num; })

#define UPDATE_SLAB_PARAMS(value) ({ value; })

#define SLAB_PARAMS(...)                                                       \
	({                                                                     \
		HeapDataParams _sdp__;                                         \
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
		HeapAllocatorPtr _sa__;                                        \
		bool __no_malloc___ = false;                                   \
		int __counter___ = 0;                                          \
		EXPAND(FOR_EACH(COUNT_ARGS, __VA_ARGS__));                     \
		heap_allocator_build(&_sa__, zeroed,                           \
				     __counter___ __VA_OPT__(, ) __VA_ARGS__); \
		_sa__.no_malloc = false;                                       \
		_sa__;                                                         \
	})

#define ALLOCATE_SLAB(size)                                                    \
	({                                                                     \
		({                                                             \
			FatPtr _ret_allocate_heap__;                           \
			if (mymalloc(&_ret_allocate_heap__, size))             \
				return STATIC_ALLOC_ERROR_RESULT;              \
			_ret_allocate_heap__;                                  \
		});                                                            \
	})

#define ALLOCATE_SLABP(size)                                                   \
	({                                                                     \
		({                                                             \
			FatPtr _ret_allocate_heap__;                           \
			if (mymalloc(&_ret_allocate_heap__, size))             \
				panic(                                         \
				    "Could not allocate sufficient memory!");  \
			_ret_allocate_heap__;                                  \
		});                                                            \
	})

#endif // _CORE_HEAP__
