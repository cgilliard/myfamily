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

#include <base/chain_allocator.h>
#include <base/panic.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// the default heap allocator for the current thread
_Thread_local HeapAllocator *__default_tl_heap_allocator = NULL;

// the global heap allocator (used when the memory will potentially be
// deallocated in a different thread than it was created in
HeapAllocator *__global_sync_allocator = NULL;
// lock for the global heap allocator
Lock *__global_sync_allocator_lock = NULL;

// default heap allocator config (no_malloc and zeroed are disabled)
HeapAllocatorConfig __default_hconfig_ = {false, false};

// create default slab sizes for the heap allocator with alignment in mind
#define HD_CFG(size) {size, 10, 0, UINT32_MAX}
HeapDataParamsConfig __default_hdpc_arr_value[] = {HD_CFG(8),
												   HD_CFG(16),
												   HD_CFG(32),
												   HD_CFG(48),
												   HD_CFG(64),
												   HD_CFG(80),
												   HD_CFG(96),
												   HD_CFG(112),
												   HD_CFG(128),
												   HD_CFG(144),
												   HD_CFG(160),
												   HD_CFG(176),
												   HD_CFG(192),
												   HD_CFG(208),
												   HD_CFG(224),
												   HD_CFG(240),
												   HD_CFG(256),
												   HD_CFG(272),
												   HD_CFG(288),
												   HD_CFG(304),
												   HD_CFG(320),
												   HD_CFG(336),
												   HD_CFG(352),
												   HD_CFG(368),
												   HD_CFG(384),
												   HD_CFG(400),
												   HD_CFG(416),
												   HD_CFG(432),
												   HD_CFG(448),
												   HD_CFG(464),
												   HD_CFG(480),
												   HD_CFG(496),
												   HD_CFG(512),
												   HD_CFG(1024),
												   HD_CFG(2048),
												   HD_CFG(4096),
												   HD_CFG(8192),
												   {16384, 5, 0, UINT32_MAX},
												   {32768, 5, 0, UINT32_MAX},
												   {65536, 2, 0, UINT32_MAX}};

// heap allocator config
HeapDataParamsConfig *__default_hdpc_arr_ = __default_hdpc_arr_value;

// the size of the default array
u64 __default_hdpc_arr_size =
	sizeof(__default_hdpc_arr_value) / sizeof(__default_hdpc_arr_value[0]);

// allows for overwriting the hconfig
void set_default_hconfig(HeapAllocatorConfig *hconfig) {
	__default_hconfig_ = *hconfig;
}

// allows for overwriting of the default hdpc array
void set_default_hdpc_arr(HeapDataParamsConfig arr[], u64 size) {
	__default_hdpc_arr_ = arr;
	__default_hdpc_arr_size = size;
}

// build the using the default heap allocator config
int build_default_heap_allocator(HeapAllocator *ptr) {
	return heap_allocator_build_arr(ptr, &__default_hconfig_, __default_hdpc_arr_,
									__default_hdpc_arr_size);
}

// init the global sync allocator
void global_sync_allocator_init() {
	// allocate for the data structure
	__global_sync_allocator = malloc(sizeof(HeapAllocator));
	__global_sync_allocator_lock = malloc(sizeof(Lock));

	// check for failures
	if (__global_sync_allocator_lock == NULL || __global_sync_allocator == NULL) {
		// deallocate and panic
		if (__global_sync_allocator_lock != NULL)
			free(__global_sync_allocator_lock);
		if (__global_sync_allocator != NULL)
			free(__global_sync_allocator);
		panic("Could not initialize the global sync HeapAllocator");
	}

	// setup the lock
	*__global_sync_allocator_lock = LOCK();

	// build with the default configurations
	if (build_default_heap_allocator(__global_sync_allocator)) {
		free(__global_sync_allocator);
		panic("Could not initialize the global sync HeapAllocator");
	}
}

// Entry for the chain allocator stack.
typedef struct ChainGuardEntry {
	HeapAllocator *ha;
	bool is_sync;
	Lock *lock;
} ChainGuardEntry;

// stack for chain allocators
_Thread_local ChainGuardEntry __thread_local_chain_allocator[MAX_CHAIN_ALLOCATOR_DEPTH];
_Thread_local u64 __thread_local_chain_allocator_index = 0;

// cleanup the local thread allocator (used when threads terminate)
void thread_local_allocator_cleanup() {
	if (__thread_local_chain_allocator_index > 0)
		heap_allocator_cleanup(
			__thread_local_chain_allocator[__thread_local_chain_allocator_index].ha);
}

void global_sync_allocator_cleanup() {
	if (__global_sync_allocator != NULL)
		heap_allocator_cleanup(__global_sync_allocator);
}

void chain_guard_cleanup(ChainGuard *ptr) {
	__thread_local_chain_allocator_index--;
}

ChainGuard chain_guard(ChainConfig *config) {
	if (__thread_local_chain_allocator_index >= MAX_CHAIN_ALLOCATOR_DEPTH)
		panic("too many chain allocators");

	ChainGuardPtr ret = {__thread_local_chain_allocator_index, config->ha, config->is_sync,
						 config->lock};

	__thread_local_chain_allocator[__thread_local_chain_allocator_index].ha = config->ha;
	__thread_local_chain_allocator[__thread_local_chain_allocator_index].is_sync = config->is_sync;
	__thread_local_chain_allocator[__thread_local_chain_allocator_index].lock = config->lock;

	__thread_local_chain_allocator_index++;
	return ret;
}

int chain_malloc(FatPtr *ptr, u64 size) {
	if (__thread_local_chain_allocator_index == 0) {
		// thread local allocator has not been initialized. Create it
		// now.
		HeapAllocator *ha = malloc(sizeof(HeapAllocator));
		if (!ha)
			return -1;

		if (build_default_heap_allocator(ha)) {
			free(ha);
			return -1;
		};

		__default_tl_heap_allocator = ha;
		__thread_local_chain_allocator[0].ha = ha;
		__thread_local_chain_allocator[0].is_sync = false;
		__thread_local_chain_allocator_index++;
	}
	if (size == 0)
		return -1;

	u64 index = __thread_local_chain_allocator_index - 1;
	bool is_sync = __thread_local_chain_allocator[index].is_sync;

	int ret;
	{
		LockGuard lg = LOCK_GUARD_CLEANUP_SAFE;
		if (is_sync)
			lg = lock(__thread_local_chain_allocator[index].lock);

		ret = heap_allocator_allocate(__thread_local_chain_allocator[index].ha, size, ptr);
	}

	return ret;
}
int chain_realloc(FatPtr *dst, FatPtr *src, u64 size) {
	if (__thread_local_chain_allocator_index == 0) {
		return -1;
	}
	if (size == 0) {
		errno = EINVAL;
		return -1;
	}
	u64 index = __thread_local_chain_allocator_index - 1;
	bool is_sync = __thread_local_chain_allocator[index].is_sync;

	int ret;
	{
		LockGuard lg = LOCK_GUARD_CLEANUP_SAFE;
		if (is_sync)
			lg = lock(__thread_local_chain_allocator[index].lock);

		ret = heap_allocator_allocate(__thread_local_chain_allocator[index].ha, size, dst);
		if (!ret) {
			void *src_data = fat_ptr_data(src);
			void *dst_data = fat_ptr_data(dst);
			u64 len = fat_ptr_len(src);
			memcpy(dst_data, src_data, len);
			ret = chain_free(src);
		}
	}

	return ret;
}
int chain_free(FatPtr *ptr) {
	if (__thread_local_chain_allocator_index == 0) {
		errno = ERANGE;
		return -1;
	}
	u64 index = __thread_local_chain_allocator_index - 1;
	bool is_sync = __thread_local_chain_allocator[index].is_sync;

	int ret;
	{
		LockGuard lg = LOCK_GUARD_CLEANUP_SAFE;
		if (is_sync)
			lg = lock(__thread_local_chain_allocator[index].lock);

		ret = heap_allocator_free(__thread_local_chain_allocator[index].ha, ptr);
	}
	return ret;
}
