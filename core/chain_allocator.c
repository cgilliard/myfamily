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

#include <core/chain_allocator.h>
#include <core/panic.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

HeapAllocator *__global_sync_allocator = NULL;

HeapAllocatorConfig __default_hconfig_ = {false, false};
HeapDataParamsConfig __default_hdpc_arr_value[] = {
    {8, 10, 0, UINT32_MAX},    {16, 10, 0, UINT32_MAX},
    {32, 10, 0, UINT32_MAX},   {48, 10, 0, UINT32_MAX},
    {64, 10, 0, UINT32_MAX},   {128, 10, 0, UINT32_MAX},
    {256, 10, 0, UINT32_MAX},  {512, 10, 0, UINT32_MAX},
    {1024, 10, 0, UINT32_MAX}, {2048, 10, 0, UINT32_MAX},
    {4096, 10, 0, UINT32_MAX}, {8192, 10, 0, UINT32_MAX},
    {16384, 5, 0, UINT32_MAX}, {32768, 5, 0, UINT32_MAX},
    {65536, 2, 0, UINT32_MAX},
};

HeapDataParamsConfig *__default_hdpc_arr_ = __default_hdpc_arr_value;
u64 __default_hdpc_arr_size =
    sizeof(__default_hdpc_arr_value) / sizeof(__default_hdpc_arr_value[0]);

void set_default_hconfig(HeapAllocatorConfig *hconfig) {
	__default_hconfig_ = *hconfig;
}

void set_default_hdpc_arr(HeapDataParamsConfig arr[], u64 size) {
	__default_hdpc_arr_ = arr;
	__default_hdpc_arr_size = size;
}

int build_default_heap_allocator(HeapAllocator *ptr) {
	return heap_allocator_build_arr(ptr, &__default_hconfig_,
					__default_hdpc_arr_,
					__default_hdpc_arr_size);
}

void global_sync_allocator_init() {
	__global_sync_allocator = malloc(sizeof(HeapAllocator));
	if (__global_sync_allocator == NULL) {
		panic("Could not initialize the global sync HeapAllocator");
	}

	if (build_default_heap_allocator(__global_sync_allocator)) {
		free(__global_sync_allocator);
		panic("Could not initialize the global sync HeapAllocator");
	}
}

typedef struct ChainGuardEntry {
	HeapAllocator *ha;
	bool is_sync;
} ChainGuardEntry;

#define MAX_CHAIN_ALLOCATOR_DEPTH 100

_Thread_local ChainGuardEntry
    __thread_local_chain_allocator[MAX_CHAIN_ALLOCATOR_DEPTH];
_Thread_local u64 __thread_local_chain_allocator_index = 0;

void thread_local_allocator_cleanup() {
	if (__thread_local_chain_allocator_index > 0)
		heap_allocator_cleanup(
		    __thread_local_chain_allocator
			[__thread_local_chain_allocator_index]
			    .ha);
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

	ChainGuardPtr ret = {__thread_local_chain_allocator_index, config->ha,
			     config->is_sync};

	__thread_local_chain_allocator[__thread_local_chain_allocator_index]
	    .ha = config->ha;
	__thread_local_chain_allocator[__thread_local_chain_allocator_index]
	    .is_sync = config->is_sync;

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
		__thread_local_chain_allocator[0].ha = ha;
		__thread_local_chain_allocator[0].is_sync = false;
		__thread_local_chain_allocator_index++;
	}
	if (size == 0)
		return -1;

	u64 index = __thread_local_chain_allocator_index - 1;
	bool is_sync = __thread_local_chain_allocator[index].is_sync;
	// TODO: implement sync

	return heap_allocator_allocate(__thread_local_chain_allocator[index].ha,
				       size, ptr);
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
	// TODO: implement sync

	if (heap_allocator_allocate(__thread_local_chain_allocator[index].ha,
				    size, dst))
		return -1;

	void *src_data = fat_ptr_data(src);
	void *dst_data = fat_ptr_data(dst);
	u64 len = fat_ptr_len(src);
	memcpy(dst_data, src_data, len);
	int ret = chain_free(src);

	return ret;
}
int chain_free(FatPtr *ptr) {
	if (__thread_local_chain_allocator_index == 0) {
		errno = ERANGE;
		return -1;
	}
	u64 index = __thread_local_chain_allocator_index - 1;
	bool is_sync = __thread_local_chain_allocator[index].is_sync;
	// TODO: implement sync
	return heap_allocator_free(__thread_local_chain_allocator[index].ha,
				   ptr);
}
