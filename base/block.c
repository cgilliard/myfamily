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

#include <base/block.h>
#include <base/cache.h>
#include <base/lock.h>
#include <base/macros.h>
#include <base/print_util.h>
#include <base/slabs.h>
#include <base/sys.h>

// Will round up array size to 2048 (based on power of 2 and load factor)
#define CACHE_SIZE 1530
#define LOAD_FACTOR 0.75

SlabAllocator block_slab_allocator;

typedef struct Block {
	int64 id;
	byte padding[24];
	byte data[];
} Block;

bool block_is_init = false;
Lock block_init_lock = INIT_LOCK;
Cache global_cache;

void block_init() {
	lockw(&block_init_lock);
	if (cache_init(&global_cache, CACHE_SIZE, LOAD_FACTOR))
		panic("Could not initialize cache!");
	block_is_init = true;
	slab_allocator_init(&block_slab_allocator, 128 - SLAB_LIST_SIZE, CACHE_SIZE,
						UINT32_MAX);
	unlock(&block_init_lock);
}

CacheItem *block_load(int64 id) {
	if (!block_is_init) block_init();
	CacheItem *ci = cache_find(&global_cache, id);
	if (ci == NULL) {
		void *addr = fmap(id);
		if (addr) {
			ci = slab_allocator_allocate(&block_slab_allocator);
			ci->addr = addr;
			ci->id = id;
			const CacheItem *to_delete = cache_insert(&global_cache, ci);
			if (to_delete) unmap(to_delete->addr, 1);
		}
	} else
		cache_move_to_head(&global_cache, ci);
	return ci;
}

void block_free(CacheItem *item) {
	if (!block_is_init) block_init();
}

void block_cleanup() {
	if (block_is_init) {
		cache_cleanup(&global_cache, true);
		slab_allocator_cleanup(&block_slab_allocator);
		block_is_init = false;
	}
}
