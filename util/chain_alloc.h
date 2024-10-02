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

/**
 * @file
 * @brief This file contains definitions for the chain_allocator.
 *
 * @section Overview
 * <p>The chain allocator is a data structure which allows for chaining of
 * the #SlabAllocator structure. These slab allocators can be added to the chain
 * and when the scope that they were added in ends, they are automatically removed
 * from the chain and the previous chain allocator becomes active once again.
 * The purpose is to allow for scope specific slab allocator. This allows data structures
 * to use a specific slab allocator during their function calls.</p><p>For instance, I might have
 * a Queue data type which has a synchronized configuration parameter. If synchronized is false,
 * the Queue can use the default thread local slab allocator to allocate memory. However, if the
 * queue is meant to be thread safe, the Queue may want to have either a global synchornized
 * SlabAllocator or a custom SlabAllocator that is only used for this Queue. All these options
 * are possible with the chain allocator and additionally, the previous stack of slab allocators
 * is restored after the Queue's functions end. This is accomplished with the 'ChainGuard' type
 * and its __attribute__ cleanup functon which automatically restores state when the guard goes
 * out of scope. So, let's say my Queue is configured to use a synchronized slab allocator, the
 * queue can use it in it's function call simply by calling:
 * ChainGuard guard = chain_guard_init(sync_slabs, true);
 * Then, when this guard goes out of scope (at the end of the Queue's function call), the previous
 * SlabAllocator is restored. If no additional slab allocators are left, the default slab allocator
 * remains. This way a data type can use a memory allocation method that meets its requirements
 * without having an effect on calling data types or data types it calls itself.
 */

#ifndef _UTIL_CHAIN_ALLOC__
#define _UTIL_CHAIN_ALLOC__

#include <util/slabs.h>

typedef struct ChainGuardNc {
	void *impl;
} ChainGuardNc;

void chain_guard_cleanup(ChainGuardNc *ptr);

#define ChainGuard ChainGuardNc __attribute__((warn_unused_result, cleanup(chain_guard_cleanup)))

// Initialize a chain_guard with the specified SlabAllocator and sync status.
// When the chain guard goes out of scope the the 'link' for this SlabAllocator is removed from
// the chain leaving the allocation chain in the state it was at before calling this function.
// Important: This function must not be called more than once per scope or behaviour is undefined.
// Example (Ok):
// {
//     ChainGuard c1 = chain_guard_init(&myallocator1, false);
//     {
//         ChainGuard c2 = chain_guard_init(&myallocator2, true);
//         // ...
//     }
// }
// Example (NOT Ok):
// {
//     ChainGuard c1 = chain_guard_init(&myallocator1, false);
//     ChainGuard c2 = chain_guard_init(&myallocator2, true);
//     // ...
// }
ChainGuard chain_guard_init(SlabAllocator *sa, bool is_sync);

// Allocate a 'FatPtr' with the specified size with the current SlabAllocator.
int chain_malloc(FatPtr *ptr, u64 size);
// Re-allocate a 'FatPtr' with the specified size with the current SlabAllocator.
int chain_realloc(FatPtr *ptr, u64 size);
// Free a 'FatPtr' to the current SlabAllocator.
void chain_free(FatPtr *ptr);

#ifdef TEST
void cleanup_default_slab_allocator();
#endif // TEST

#endif // _UTIL_CHAIN_ALLOC__
