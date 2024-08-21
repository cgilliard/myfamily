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

#include <core/types.h>

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
 */
int heap_allocator_build(HeapAllocator *ptr, HeapAllocatorConfig *config,
			 int heap_data_params_count, ...);

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
void *fat_ptr_data(FatPtr *ptr);

/**
 * Returns the length in bytes of the specified #FatPtr.
 * @param ptr A pointer to the location of the #FatPtr.
 * @return the length in bytes that has been allocated to this #FatPtr.
 * @see [fat_ptr_data]
 */
u64 fat_ptr_len(FatPtr *ptr);

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
u64 fat_ptr_id(FatPtr *ptr);
#endif // TEST

#endif // _CORE_HEAP__
