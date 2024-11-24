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

#ifndef _BASE_CACHE__
#define _BASE_CACHE__

#include <base/types.h>

#define CACHE_IMPL_SIZE 64
typedef struct Cache {
	byte impl[CACHE_IMPL_SIZE];
} Cache;

typedef struct Block {
	struct Block *next;
	struct Block *prev;
	struct Block *chain_next;
	int64 id;
	void *addr;
	int64 ref_count;
} Block;

int cache_init(Cache *cache, int64 capacity, float load_factor);
Block *cache_insert(Cache *cache, Block *item);
int cache_move_to_head(Cache *cache, const Block *item);
Block *cache_find(const Cache *cache, int64 id);
void cache_cleanup(Cache *cache, bool unmap_addr);

#endif	// _BASE_CACHE__
