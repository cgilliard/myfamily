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

typedef struct HeapDataParamsConfig {
	u32 slab_size;
	u32 slabs_per_resize;
	u32 initial_chunks;
	u32 max_slabs;
} HeapDataParamsConfig;

typedef struct HeapAllocatorConfig {
	bool zeroed;
	bool no_malloc;
} HeapAllocatorConfig;

typedef struct FatPtr {
	u64 id;
	void *data;
	u64 len;
} FatPtr;

typedef struct HeapAllocatorImpl HeapAllocatorImpl;

typedef struct HeapAllocator {
	HeapAllocatorImpl *impl;
} HeapAllocator;

int heap_allocator_build(HeapAllocator *ptr, HeapAllocatorConfig *config,
			 int heap_data_params_count, ...);
int heap_allocator_allocate(HeapAllocator *ptr, u64 size, FatPtr *fptr);
int heap_allocator_free(HeapAllocator *ptr, FatPtr *fptr);
int heap_allocator_cleanup(HeapAllocator *ptr);

void *fat_ptr_data(FatPtr *ptr);
u64 fat_ptr_len(FatPtr *ptr);

#endif // _CORE_HEAP__
