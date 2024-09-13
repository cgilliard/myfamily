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

#ifndef _BASE_HEAP__
#define _BASE_HEAP__

/**
 * @file
 * @brief This file contains definitions for the custom heap allocator.
 *
 * @section Overview
 * The custom heap allocator aims to provide efficient memory management for
 * high-performance applications. It includes features such as:
 * - Slab allocation for reducing fragmentation
 * - Support for varying block sizes
 *
 * @section Design Decisions
 * - The allocator uses a slab-based approach to minimize fragmentation and
 *   improve allocation performance.
 * - It incorporates a thread-local storage mechanism to optimize for
 * multi-threaded scenarios.
 *
 * @section notes
 * - Note that the #HeapAllocator implementation does not provide any
 *   thread safety. Externaly thread safety techniques must be employed.
 * - Note that regarding alignment, the slab allocator allocates the free
 *   list independantly from the raw slab data. Free list entries are 4
 *   bytes long so they follow standard alignment practices for 4 bytes by
 *   dividing an array into 4 byte chunks for each free list entry.
 *   The data section is split up sequentially. If a 16 byte slab_size is
 *   configured with 10 slabs_per_resize, a chunk will be a contiguous block of
 *   160 bytes. When a slab allocated from this chunk it will be aligned to the
 *   16 byte offsets within this chunk.
 *
 * @section Usage
 * To use the heap allocator, initialize it with the desired parameters and
 * then use the provided API functions for allocation and deallocation.
 */

#include <base/types.h>

/**
 * The HeapDataParamsConfig specifies how individiaul slab_sizes
 * within the #HeapAllocator are assigned.
 * @param slab_size The size of the slabs within this #HeapDataParamsConfig.
 * @param slabs_per_resize The number of slabs that will be added when
 * the #HeapAllocator runs out of slabs with this configuration size.
 * @param initial_chunks The number of chunks (defined by the slabs_per_resize
 * param) to create on initialization of the #HeapAllocator. This value may be
 * 0, in which case the #HeapAllocator will not initialize any slabs for this
 * size and will only resize based on other configurations if a call to
 * heap_allocator_allocate is made.
 * @param max_slabs The maximum number of slabs to allocate for this particular
 * slab_size. If more slabs are requested, the resulting behavior is that unless
 * no_malloc is configured in the #HeapAllocatorConfig for this #HeapAllocator,
 * malloc will be called directly to attempt to allocate the required slabs.
 * @see [heap_allocator_build]
 */
typedef struct HeapDataParamsConfig {
	u32 slab_size;
	u32 slabs_per_resize;
	u32 initial_chunks;
	u32 max_slabs;
} HeapDataParamsConfig;

/**
 * Configuration for a #HeapAllocator. This type specifies whether or not
 * the heap allocated should be zeroed (All FatPtr types returned will have
 * all data in them set to 0), or if malloc should be enabled (If no_malloc
 * is set to true, malloc will not be called as a backup on this #HeapAllocator
 * and the inibility to allocate a slab for the call to heap_allocator_allocate
 * will result in an error.
 * @param zeroed if set to true zeroing as described above is enabled.
 * @param no_malloc if set to true no_malloc as described above is enabled.
 * @see [heap_allocator_build]
 */
typedef struct HeapAllocatorConfig {
	bool zeroed;
	bool no_malloc;
} HeapAllocatorConfig;

/**
 * A pointer structure which includes the length and ID such that
 * the #HeapAllocator implementation may assign and reuse freed
 * slabs and manage memory resources based on specifications.
 * #FatPtr fields should not be accessed directly and instead the
 * getters for data and len should be used.
 * @see [fat_ptr_data]
 * @see [fat_ptr_len]
 */
typedef struct FatPtr {
	u64 id;
	void *data;
	u64 len;
} FatPtr;

/**
 * Opaque pointer to the implementation of the HeapAllocator.
 * @see [heap_allocator_build]
 * @see [heap_allocator_allocate]
 * @see [heap_allocator_free]
 * @see [heap_allocator_cleanup]
 */
typedef struct HeapAllocatorImpl HeapAllocatorImpl;

/**
 * A wrapper that contains the opaque pointer to the #HeapAllocatorImpl
 * implementation of the HeapAllocator.
 * @see [heap_allocator_build]
 * @see [heap_allocator_allocate]
 * @see [heap_allocator_free]
 * @see [heap_allocator_cleanup]
 */
typedef struct HeapAllocator {
	HeapAllocatorImpl *impl;
} HeapAllocator;

/**
 * Builds a heap_allocator based on the specified #HeapAllocatorConfig and
 * #HeapDataParamsConfig configurations.
 * @param ptr A pointer to the location where the #HeapAllocator to be
 * configured is stored. This pointer must not be NULL.
 * @param config The configuration for this #HeapAllocator.
 * @param heap_data_params_count The number of variadic arguments being
 * passed to this function.
 * @return 0 on success -1 on error with the appropriate errno value set.
 * @see [heap_allocator_allocate]
 * @see [heap_allocator_free]
 * @see [heap_allocator_cleanup]
 * @see [heap_allocator_build_arr]
 */
int heap_allocator_build(
	HeapAllocator *ptr, HeapAllocatorConfig *config, int heap_data_params_count, ...);

/**
 * Builds a heap_allocator based on the specified #HeapAllocatorConfig and
 * #HeapDataParamsConfig configurations. This is the array based version of
 * build.
 * @param ptr A pointer to the location where the #HeapAllocator to be
 * configured is stored. This pointer must not be NULL.
 * @param config The configuration for this #HeapAllocator.
 * @param arr The array of HeapDataParamsConfigs which specify this
 * HeapAllocator.
 * @param heap_data_params_count The number of variadic arguments being
 * passed to this function.
 * @return 0 on success -1 on error with the appropriate errno value set.
 * @see [heap_allocator_allocate]
 * @see [heap_allocator_free]
 * @see [heap_allocator_cleanup]
 * @see [heap_allocator_build]
 */
int heap_allocator_build_arr(HeapAllocator *ptr, HeapAllocatorConfig *config,
	HeapDataParamsConfig arr[], u64 heap_data_params_count);

/**
 * Builds a heap_allocator based on the current default settings. This is the same as the global
 * heap allocator and the thread local heap allocators.
 * @param ptr A pointer to the location where the #HeapAllocator to be
 * configured is stored. This pointer must not be NULL.
 * @return 0 on success -1 on error with the appropriate errno value set.
 * @see [heap_allocator_allocate]
 * @see [heap_allocator_free]
 * @see [heap_allocator_cleanup]
 * @see [heap_allocator_build]
 */
int build_default_heap_allocator(HeapAllocator *ptr);

/**
 * Allocates a #FatPtr from the #HeapAllocator (if possible). Note that
 * The #HeapAllocator will try to first allocate one of it's existing slabs
 * for this request. If none are available, it will allocate additional slabs
 * up to it's limit which is specified by the configured max_slabs for this
 * #HeapDataParamsConfig. If no more slabs are allowed (or can be created due
 * to a failure to resize), an attempt to call malloc will be made (unless
 * this #HeapAllocator was configured with the HeapAllocatorConfig::no_malloc
 * in which case an error will be returned.
 * @param ptr A pointer to the location of the #HeapAllocator.
 * @param size The size, in bytes, of memory to allocate to this #FatPtr
 * @param fptr On success the #FatPtr pointed to by fptr will be updated
 * with the info of the assigned slab/memory location.
 * length of the allocation.
 * @return 0 on success -1 on error with the appropriate errno value set.
 * @see [heap_allocator_build]
 * @see [heap_allocator_free]
 * @see [heap_allocator_cleanup]
 */
int heap_allocator_allocate(HeapAllocator *ptr, u64 size, FatPtr *fptr);

/**
 * Return the total number of slabs allocated for this #HeapAllocator.
 */
u64 heap_allocator_cur_slabs_allocated(HeapAllocator *ptr);

/**
 * Free the data associated with this #FatPtr releasing it back to the
 * specified #HeapAllocator. Note that this memory will not be released back
 * to the operating system, unless this particular #FatPtr was allocated with
 * malloc.
 * @param ptr A pointer to the location of the #HeapAllocator that this #FatPtr
 * was assigned by. Note: if a #FatPtr is freed with a #HeapAllocator other
 * than the one that allocated it, undefined behavior will result.
 * @param fptr The #FatPtr to free.
 * @return 0 on success -1 on error with the appropriate errno value set.
 * @see [heap_allocator_cleanup]
 */
int heap_allocator_free(HeapAllocator *ptr, FatPtr *fptr);

/**
 * Cleanup up this #HeapAllocator freeing all memory resources associated
 * with it to the operating system.
 * @param ptr A pointer to the location of the #HeapAllocator.
 * @return 0 on success -1 on error with the appropriate errno value set.
 * @see [heap_allocator_free]
 */
int heap_allocator_cleanup(HeapAllocator *ptr);

/**
 * Returns the memory location of the specified #FatPtr.
 * @param ptr A pointer to the location of the #FatPtr.
 * @return a void pointer to the data location assicated to this #FatPtr
 * @see [fat_ptr_len]
 */
void *fat_ptr_data(const FatPtr *ptr);

/**
 * Returns the length in bytes of the specified #FatPtr.
 * @param ptr A pointer to the location of the #FatPtr.
 * @return the length in bytes that has been allocated to this #FatPtr.
 * @see [fat_ptr_data]
 */
u64 fat_ptr_len(const FatPtr *ptr);

#define FAT_PTR_INIT { 0, NULL, 0 };

#define HA_CONFIG_DEFAULT { false, false }
#define HAP_CONFIG(slab_size, slabs_per_resize, initial_chunks, max_slabs)                         \
	({                                                                                             \
		HeapDataParamsConfig _ret__ = {                                                            \
			slab_size,                                                                             \
			slabs_per_resize,                                                                      \
			initial_chunks,                                                                        \
			max_slabs,                                                                             \
		};                                                                                         \
		_ret__;                                                                                    \
	})

// make some private functions/variables visible for tests
#ifdef TEST
typedef struct FatPtr FatPtr;
extern u64 __malloc_count;
extern u64 __free_count;
extern bool __debug_build_allocator_malloc_fail1;
extern bool __debug_build_allocator_malloc_fail2;
extern bool __debug_build_allocator_malloc_fail3;
extern bool __debug_build_allocator_malloc_fail4;
extern bool __debug_build_allocator_malloc_fail5;
extern bool __debug_build_allocator_malloc_fail6;
extern bool __debug_build_allocator_malloc_fail7;
extern bool __debug_build_allocator_malloc_fail8;

void *do_malloc(size_t size);
void *do_realloc(void *ptr, size_t size);
void do_free(void *ptr);
u64 fat_ptr_id(const FatPtr *ptr);
#endif // TEST

#endif // _BASE_HEAP__
