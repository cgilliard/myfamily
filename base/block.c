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
#include <base/sys.h>

// Will round up array size to 2048 (based on power of 2 and load factor)
#define CACHE_SIZE 1530
#define LOAD_FACTOR 0.75

bool block_is_init = false;
Lock block_init_lock = INIT_LOCK;
Cache global_cache;

void block_init() {
	lockw(&block_init_lock);
	if (cache_init(&global_cache, CACHE_SIZE, LOAD_FACTOR))
		panic("Could not initialize cache!");
	block_is_init = true;
	unlock(&block_init_lock);
}

Block block_load(int64 id) {
	if (!block_is_init) block_init();
	const CacheItem *ci = cache_find(&global_cache, id);
	Block ret = {.id = id};
	if (ci)
		ret.addr = ci->addr;
	else {
		ret.addr = fmap(id);
	}
	return ret;
}

void block_release(Block block) {
	if (!block_is_init) block_init();
}
