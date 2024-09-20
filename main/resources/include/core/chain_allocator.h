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

#ifndef _CORE_CHAIN_ALLOCATOR__
#define _CORE_CHAIN_ALLOCATOR__

#include <core/chain_allocator.h>
#include <core/heap.h>
#include <core/lock.h>

#define MAX_CHAIN_ALLOCATOR_DEPTH 100

extern _Thread_local HeapAllocator *__default_tl_heap_allocator;

/**
 * @file
 * @brief This file contains definitions for the chain_allocator.
 *
 * @section Overview
 * <p>The chain allocator is a data structure which allows for chaining of
 * The #HeapAllocator data structure. Each time a new #HeapAllocator is
 * added to the chain, it as pushed to the top of an internal stack. When
 * these 'links' in the chain go out of scope, the previous #HeapAllocator
 * remains at the top of the stack and is used for allocations. The functions
 * chain_malloc, chain_realloc, and chain_free provide allocation functions
 * which access the underlying #HeapAllocator at the top of the stack.
 * </p><p>This structure allows for convenient usage of the #HeapAllocator,
 * enabling the usage of a default thread local #HeapAllocator which is used for
 * most allocations, with the ability for an application to push a custom
 * #HeapAllocator to the top of the stack for various purposes. Most frequently,
 * this functionality may be used for a global locking thread safe slab
 * allocator that can allocate and deallocate slabs from any thread. </p><p>For
 * instance a synchronous Hashtable might set the #HeapAllocator to the global
 * locking #HeapAllocator at the beginning of each of its function calls. When
 * the function call returns execution back to the calling function, the
 * previous #HeapAllocator will be at the top of the stack because the
 * #ChainGuard, which is returned by the #chain_guard function goes out of scope
 * and calls its cleanup function through the use of the __attribute__ cleanup
 * functionality in GCC / Clang.
 *
 * @section notes
 * - User must ensure that the appropriate ChainConfig is set during execution.
 * - If no #HeapAllocator has been added to the chain, The #chain_malloc call
 *   will create a default based on the configurable default parameters.
 * - The #GLOBAL_SYNC_ALLOCATOR will build a synchronous #HeapAllocator using
 * the configurable default parameters the first time it is called.
 * - Only one call to #chain_guard may be called per scope, otherwise, undefined
 *  behavior may result.
 *
 * @section Usage
 * In the below example the #GLOBAL_SYNC_ALLOCATOR macro is called
 * to obtain a global sync allocator that uses locking to ensure
 * allocations and deallocations are thread safe.
 * ```
 *  {
 *     FatPtr ptr;
 *     ChainGuard guard = GLOBAL_SYNC_ALLOCATOR();
 *     cr_assert_eq(chain_malloc(&ptr, 87), 0);
 *     cr_assert_eq(fat_ptr_len(&ptr), 128);
 *     chain_free(&ptr);
 * }
 * ```
 */

/** @cond */
extern HeapAllocator *__global_sync_allocator;
extern LockPtr *__global_sync_allocator_lock;
/** @endcond */

/**
 * The #ChainConfig specifies which #HeapAllocator will be used and whether
 * calls to #chain_malloc, #chain_realloc, and #chain_free should be performed
 * in a synchronous manner using locks.
 * @param ha The #HeapAllocator to push to the top of the stack.
 * @param is_sync Whether or not synchronous locking should be used.
 */
typedef struct ChainConfig {
	HeapAllocator *ha;
	bool is_sync;
	Lock *lock;
} ChainConfig;

/** @cond */
typedef struct ChainGuardImpl ChainGuardImpl;

typedef struct ChainGuardPtr {
	u64 index;
	HeapAllocator *ha;
	bool is_sync;
	Lock *lock;
} ChainGuardPtr;

void chain_guard_cleanup(ChainGuardPtr *ptr);
/** @endcond */

/**
 * The #ChainGuard is a data structure used to assist the Chain in knowing when
 * a particular link in the chain goes out of scope.
 */
#define ChainGuard ChainGuardPtr __attribute__((warn_unused_result, cleanup(chain_guard_cleanup)))

/**
 * The #chain_guard function is used to push a new #HeapAllocator to the top
 * of the stack. This function must only be called once per scope.
 * @param config The #ChainConfig to push to the top of the stack.
 * @return A ChainGuard which is used to indicate when the #HeapAllocator should
 * go out of scope and revert to the previous #HeapAllocator.
 */
ChainGuard chain_guard(ChainConfig *config);
/**
 * Initializes the global_sync_allocator. This happens automatically with the
 * first call to #GLOBAL_SYNC_ALLOCATOR.
 */
void global_sync_allocator_init();
/**
 * Sets a default #HeapAllocatorConfig to be used when a new thread local
 * #HeapAllocator or Global #HeapAllocator is created.
 */
void set_default_hconfig(HeapAllocatorConfig *hconfig);
/**
 * Sets the default #HeapDataParamsConfig array to be used when a new thread
 * local #HeapAllocator or Global #HeapAllocator is created.
 * @param arr The HeapDataParamsConfig array.
 * @param size The size of the HeapDataParamsConfig array.
 * @return 0 on success, otherwise returns -1 with appropriate errno value set.
 */
void set_default_hdpc_arr(HeapDataParamsConfig arr[], u64 size);
/**
 * Allocates a #FatPtr of the specified size using the underlying #HeapAllocator
 * at the top of the stack.
 * @param ptr The reference to the #FatPtr to allocate memory to.
 * @param size The number of bytes to allocate to this #FatPtr.
 * @return 0 on success, otherwise returns -1 with appropriate errno value set.
 */
int chain_malloc(FatPtr *ptr, u64 size);
/**
 * Reallocates a #FatPtr of the specified size and stores the resulting #FatPtr
 * in dst on success using the underlying #HeapAllocator at the top of the
 * stack. All data in src is copied to dst's data location.
 * @param dst The new #FatPtr to allocate and copy data to.
 * @param src The existing #FatPtr to reallocate.
 * @param size The new desired size of the reallocated #FatPtr.
 * @return 0 on success, otherwise returns -1 with appropriate errno value set.
 */
int chain_realloc(FatPtr *dst, FatPtr *src, u64 size);

/**
 * Frees the specified #FatPtr to the underlying #HeapAllocator at the top of
 * the stack releasing all memory to this #HeapAllocator for future usage.
 * @return 0 on success, otherwise returns -1 with appropriate errno value set.
 * @param ptr The reference to the #FatPtr to free.
 */
int chain_free(FatPtr *ptr);

/**
 * Cleans up the global sync allocator. This is mostly required for tests.
 */
void global_sync_allocator_cleanup();

/**
 * Cleans up the thread local allocator. This is mostly required for tests.
 */
void thread_local_allocator_cleanup();

#define SCOPED_ALLOCATOR(ha, is_sync)                                                              \
	({                                                                                             \
		ChainConfig _cfg__ = { ha, is_sync };                                                      \
		ChainGuardPtr ret = chain_guard(&_cfg__);                                                  \
		ret;                                                                                       \
	})

/**
 * The #GLOBAL_SYNC_ALLOCATOR Returns a #ChainGuard for the Global Sync
 * Allocator. This #HeapAllocator is configured with the is_sync parameter and
 * is safe to use with data structures that need to allocate/deallocate memory
 * in multiple threads. The first time #GLOBAL_SYNC_ALLOCATOR is called, it will
 * create a Global Sync Allocator using the default parameters which may be
 * specified with the #set_default_hdpc_arr and #set_default_hconfig functions.
 * Note: This function panics if sufficient memory cannot be allocated to create
 * the required data structures.
 */
#define GLOBAL_SYNC_ALLOCATOR()                                                                    \
	({                                                                                             \
		if (__global_sync_allocator == NULL)                                                       \
			global_sync_allocator_init();                                                          \
		ChainConfig _cfg__ = { __global_sync_allocator, true, __global_sync_allocator_lock };      \
		ChainGuardPtr ret = chain_guard(&_cfg__);                                                  \
		ret;                                                                                       \
	})

#endif // _CORE_CHAIN_ALLOCATOR__
