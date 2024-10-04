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

#include <base/chain_alloc.h>
#include <base/panic.h>
#include <base/resources.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

typedef struct ChainGuardEntry {
	struct ChainGuardEntry *next;
	struct ChainGuardEntry *prev;
	SlabAllocator *sa;
	bool sync;
	pthread_mutex_t lock;
	bool is_locked; // used in case of a panic on cleanup to determine if we're locked
} ChainGuardEntry;

_Thread_local ChainGuardEntry *chain_guard_entry_root = NULL;
_Thread_local ChainGuardEntry *chain_guard_entry_cur = NULL;
SlabAllocator *global_sync_allocator = NULL;

SlabAllocator *init_default_slab_allocator() {
	SlabAllocatorNc *sa = mymalloc(sizeof(SlabAllocator));
	if (sa == NULL)
		panic("Could not initialize default slab allocator");
	SlabAllocatorConfig sac;

	// default slab allocator no_malloc = false, zeroed = false, is_64_bit = false
	if (slab_allocator_config_build(&sac, false, false, false))
		panic("Could not initialize config for default slab allocator");

	u64 max_slabs = (INT32_MAX / 100) * 100;
	for (i32 i = 0; i < 128; i++) {
		SlabType st = {.slab_size = (i + 3) * 8,
					   .slabs_per_resize = 100,
					   .initial_chunks = 0,
					   .max_slabs = max_slabs};
		if (slab_allocator_config_add_type(&sac, &st))
			panic("Could not add slab type to default slab allocator");
	}
	for (i32 i = 0; i < 128; i++) {
		SlabType st = {.slab_size = (128 + 3) * 8 + (i + 1) * 1024,
					   .slabs_per_resize = 100,
					   .initial_chunks = 0,
					   .max_slabs = max_slabs};
		if (slab_allocator_config_add_type(&sac, &st))
			panic("Could not add config to default slab allocator");
	}

	if (slab_allocator_build(sa, &sac))
		panic("Could not initialize default slab allocator");

	return sa;
}

SlabAllocator *get_global_sync_allocator() {
	if (global_sync_allocator == NULL)
		global_sync_allocator = init_default_slab_allocator();
	return global_sync_allocator;
}

u64 alloc_count_global_sync_allocator() {
	if (global_sync_allocator == NULL)
		return 0;
	return slab_allocator_cur_slabs_allocated(global_sync_allocator);
}

u64 alloc_count_default_slab_allocator() {
	if (chain_guard_entry_root == NULL)
		return 0;
	return slab_allocator_cur_slabs_allocated(chain_guard_entry_root->sa);
}

void cleanup_default_slab_allocator() {
	if (chain_guard_entry_root) {
		slab_allocator_cleanup(chain_guard_entry_root->sa);
		myfree(chain_guard_entry_root->sa);
		myfree(chain_guard_entry_root);
		chain_guard_entry_root = NULL;
	}
	if (global_sync_allocator) {
		slab_allocator_cleanup(global_sync_allocator);
		myfree(global_sync_allocator);
		global_sync_allocator = NULL;
	}
}

void chain_guard_init_root() {
	chain_guard_entry_root = mymalloc(sizeof(ChainGuardEntry));
	chain_guard_entry_cur = chain_guard_entry_root;
	chain_guard_entry_root->next = NULL;
	chain_guard_entry_root->prev = NULL;
	chain_guard_entry_root->sync = false;
	chain_guard_entry_root->sa = init_default_slab_allocator();
}

void chain_guard_cleanup(ChainGuardNc *ptr) {
	if (ptr->impl) {
		ChainGuardEntry *entry = ptr->impl;
		chain_guard_entry_cur = entry->prev;
		chain_guard_entry_cur->next = NULL;
		if (chain_guard_entry_cur->sync) {
			pthread_mutex_destroy(&chain_guard_entry_cur->lock);
		}
		myfree(entry);
	}
}

ChainGuard set_slab_allocator(SlabAllocator *sa, bool sync) {
	if (chain_guard_entry_root == NULL) {
		chain_guard_init_root();
	}
	ChainGuardEntry *entry = mymalloc(sizeof(ChainGuardEntry));
	entry->sa = sa;
	entry->sync = sync;
	if (entry->sync) {
		pthread_mutex_init(&entry->lock, NULL);
	}
	entry->is_locked = false;
	entry->next = NULL;
	entry->prev = chain_guard_entry_cur;
	chain_guard_entry_cur->next = entry;
	chain_guard_entry_cur = entry;
	ChainGuardNc ret = {entry};
	return ret;
}

int chain_malloc(FatPtr *ptr, u64 size) {
	if (ptr == NULL || size == 0) {
		errno = EINVAL;
		return -1;
	}
	if (chain_guard_entry_root == NULL) {
		chain_guard_init_root();
	}
	if (chain_guard_entry_cur->sync) {
	}
	int ret;
	if (chain_guard_entry_cur->sync) {
		pthread_mutex_lock(&chain_guard_entry_cur->lock);
		chain_guard_entry_cur->is_locked = true;
		ret = slab_allocator_allocate(chain_guard_entry_cur->sa, size, ptr);
		chain_guard_entry_cur->is_locked = false;
		pthread_mutex_unlock(&chain_guard_entry_cur->lock);
	} else {
		ret = slab_allocator_allocate(chain_guard_entry_cur->sa, size, ptr);
	}
	return ret;
}
int chain_realloc(FatPtr *ptr, u64 size) {
	if (chain_guard_entry_root == NULL)
		panic("Reallocating a FatPtr when it was never allocated!");

	if (ptr == NULL || nil(*ptr) || size == 0) {
		errno = EINVAL;
		return -1;
	}

	int ret = 0;

	if (chain_guard_entry_cur->sync) {
		pthread_mutex_lock(&chain_guard_entry_cur->lock);
		chain_guard_entry_cur->is_locked = true;
	}

	FatPtr tmp = null;
	chain_malloc(&tmp, size);
	if (nil(tmp)) {
		ret = -1;
	} else {

		u64 len = fat_ptr_len(ptr);
		u64 nlen = fat_ptr_len(&tmp);
		if (nlen < len) {
			len = nlen;
		}

		memcpy(fat_ptr_data(&tmp), fat_ptr_data(ptr), len);

		chain_free(ptr);
		*ptr = tmp;
	}
	if (chain_guard_entry_cur->sync) {
		chain_guard_entry_cur->is_locked = false;
		pthread_mutex_unlock(&chain_guard_entry_cur->lock);
	}
	return 0;
}
void chain_free(FatPtr *ptr) {
	if (ptr == NULL) {
		panic("attempt to free a NULL FatPtr");
	}
	if (chain_guard_entry_root == NULL)
		panic("Freeing a FatPtr when it was never allocated!");

	if (chain_guard_entry_cur->sync) {
		pthread_mutex_lock(&chain_guard_entry_cur->lock);
		chain_guard_entry_cur->is_locked = true;
		slab_allocator_free(chain_guard_entry_cur->sa, ptr);
		chain_guard_entry_cur->is_locked = false;
		pthread_mutex_unlock(&chain_guard_entry_cur->lock);
	} else {
		slab_allocator_free(chain_guard_entry_cur->sa, ptr);
	}
}
