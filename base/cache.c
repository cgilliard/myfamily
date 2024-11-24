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

#include <base/cache.h>
#include <base/err.h>
#include <base/lock.h>
#include <base/macros.h>
#include <base/murmurhash.h>
#include <base/print_util.h>
#include <base/sys.h>

#define MURMUR_HASH_SEED 2024

typedef struct CacheImpl {
	CacheItem **arr;
	int64 capacity;
	int64 size;
	int64 arr_size;
	CacheItem *head;
	CacheItem *tail;
	Lock lock;
	byte padding[8];
} CacheImpl;

unsigned int cache_next_power_of_two(unsigned int n) {
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n + 1;
}

void __attribute__((constructor)) __check_cache_sizes() {
	if (sizeof(CacheImpl) != sizeof(Cache))
		panic("sizeof(CacheImpl) (%i) != sizeof(Cache) (%i)", sizeof(CacheImpl),
			  sizeof(Cache));
}

int cache_init(Cache *cache, int64 capacity, float load_factor) {
	if (load_factor >= 1 || load_factor <= 0 || capacity < 1) {
		SetErr(IllegalArgument);
		return -1;
	}
	CacheImpl *impl = (CacheImpl *)cache;
	impl->capacity = capacity;
	impl->size = 0;
	impl->head = impl->tail = NULL;
	impl->arr_size = capacity / load_factor;
	impl->lock = INIT_LOCK;
	if (impl->arr_size > UINT32_MAX) {
		SetErr(TooBig);
		return -1;
	}
	impl->arr_size = cache_next_power_of_two(impl->arr_size);

	impl->arr = map((sizeof(CacheItem *) * ((impl->arr_size / PAGE_SIZE) + 1)));

	if (impl->arr == NULL) {
		SetErr(AllocErr);
		return -1;
	}

	return 0;
}

#define HASH(id)                                                        \
	(murmurhash(((const byte *)&id), sizeof(int64), MURMUR_HASH_SEED) % \
	 impl->arr_size)

const CacheItem *cache_evict(CacheImpl *impl) {
	unsigned int index = HASH(impl->tail->id);
	CacheItem *cur = impl->arr[index], *prev = NULL;
	while (cur) {
		if (impl->tail->id == cur->id) {
			if (prev) prev->next = cur->next;
			impl->tail->prev->next = NULL;
			impl->tail = impl->tail->prev;
			if (cur == impl->arr[index]) impl->arr[index] = cur->chain_next;
			return cur;
		}
		prev = cur;
		cur = cur->chain_next;
	}
	panic("illegal state in cache. tail not found!");
	return NULL;
}

const CacheItem *cache_insert(Cache *cache, CacheItem *item) {
	CacheImpl *impl = (CacheImpl *)cache;
	unsigned int index = HASH(item->id);

	lockr(&impl->lock);
	item->next = impl->head;
	item->chain_next = item->prev = NULL;

	if (impl->arr[index]) {
		CacheItem *cur = impl->arr[index];
		while (cur && cur->chain_next) {
			if (cur->id == item->id) {
				unlock(&impl->lock);
				return NULL;
			}
			cur = cur->chain_next;
		}
		if (cur->id == item->id) {
			unlock(&impl->lock);
			return NULL;
		}
		locku(&impl->lock);
		cur->chain_next = item;
	} else {
		locku(&impl->lock);
		impl->arr[index] = item;
	}

	if (impl->head) impl->head->prev = item;
	impl->head = item;
	if (impl->tail == NULL) impl->tail = item;

	const CacheItem *ret = NULL;
	if (impl->size >= impl->capacity)
		ret = cache_evict(impl);
	else
		impl->size++;
	unlock(&impl->lock);

	return ret;
}
CacheItem *cache_find(const Cache *cache, int64 id) {
	CacheImpl *impl = (CacheImpl *)cache;
	lockr(&impl->lock);
	CacheItem *cur = impl->arr[HASH(id)];
	while (cur && cur->id != id) cur = cur->chain_next;
	unlock(&impl->lock);
	return cur;
}

int cache_move_to_head(Cache *cache, const CacheItem *item) {
	int ret = -1;
	CacheImpl *impl = (CacheImpl *)cache;
	lockr(&impl->lock);
	CacheItem *cur = impl->arr[HASH(item->id)];
	while (cur && cur->id != item->id) cur = cur->chain_next;
	if (cur) {
		locku(&impl->lock);
		if (impl->tail == cur) impl->tail = cur->prev;
		if (impl->head == cur) impl->head = cur->next;
		if (cur->prev) cur->prev->next = cur->next;
		if (cur->next) cur->next->prev = cur->prev;
		ret = 0;
	}
	unlock(&impl->lock);
	return ret;
}

void cache_cleanup(Cache *cache, bool unmap_addr) {
	CacheImpl *impl = (CacheImpl *)cache;
	CacheItem *itt = impl->head;
	while (itt) {
		CacheItem *to_delete = itt;
		itt = itt->next;
		if (unmap_addr) unmap(to_delete->addr, 1);
	}
	unmap(impl->arr,
		  (sizeof(CacheItem *) * ((impl->arr_size / PAGE_SIZE) + 1)));
}
