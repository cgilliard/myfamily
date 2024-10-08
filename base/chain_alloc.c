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

#define MAX_CHAIN_GUARDS 128

typedef struct ChainGuardEntry {
	SlabAllocator *sa;
	bool sync;
	pthread_mutex_t lock;
	bool is_locked; // used in case of a panic on cleanup to determine if we're locked
} ChainGuardEntry;

_Thread_local static ChainGuardEntry chain_guard_entries[MAX_CHAIN_GUARDS];
_Thread_local static u8 chain_guard_sp = 0;
_Thread_local static bool chain_guard_is_init = false;

SlabAllocator *global_sync_allocator = NULL;

SlabAllocator *init_default_slab_allocator() {
	SlabAllocatorNc *sa = mymalloc(sizeof(SlabAllocator));
	printf("mymalloc slab allocator\n");
	if (sa == NULL)
		return NULL;
	SlabAllocatorConfig sac;

	// default slab allocator no_malloc = false, zeroed = false, is_64_bit = false
	if (slab_allocator_config_build(&sac, false, false, false)) {
		myfree(sa);
		return NULL;
	}

	u64 max_slabs = (INT32_MAX / 100) * 100;
	for (i32 i = 0; i < 128; i++) {
		SlabType st = {.slab_size = (i + 3) * 8,
					   .slabs_per_resize = 100,
					   .initial_chunks = 0,
					   .max_slabs = max_slabs};
		if (slab_allocator_config_add_type(&sac, &st)) {
			myfree(sa);
			return NULL;
		}
	}
	for (i32 i = 0; i < 128; i++) {
		SlabType st = {.slab_size = (128 + 3) * 8 + (i + 1) * 1024,
					   .slabs_per_resize = 100,
					   .initial_chunks = 0,
					   .max_slabs = max_slabs};
		if (slab_allocator_config_add_type(&sac, &st)) {
			myfree(sa);
			return NULL;
		}
	}

	if (slab_allocator_build(sa, &sac)) {
		myfree(sa);
		return NULL;
	}

	return sa;
}

static void __attribute__((constructor)) __init_chain_guard_array() {
	for (int i = 0; i < MAX_CHAIN_GUARDS; i++) {
		if (pthread_mutex_init(&chain_guard_entries[i].lock, NULL))
			panic("Could not init pthread_mutex");
	}
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
	if (!chain_guard_is_init || chain_guard_entries[chain_guard_sp].sa == NULL)
		return 0;
	return slab_allocator_cur_slabs_allocated(chain_guard_entries[chain_guard_sp].sa);
}

void cleanup_default_slab_allocator() {
	if (chain_guard_entries[chain_guard_sp].sa != NULL) {
		slab_allocator_cleanup(chain_guard_entries[chain_guard_sp].sa);
		printf("my free slab allocator\n");
		myfree(chain_guard_entries[chain_guard_sp].sa);
	}
	if (global_sync_allocator) {
		slab_allocator_cleanup(global_sync_allocator);
		printf("my free global sync  allocator\n");
		myfree(global_sync_allocator);
		global_sync_allocator = NULL;
	}
}

void chain_guard_cleanup(ChainGuardNc *ptr) {
	if (ptr->impl) {
		chain_guard_sp--;
	}
}

ChainGuard set_slab_allocator(SlabAllocator *sa, bool sync) {
	if (!chain_guard_is_init) {
		chain_guard_entries[chain_guard_sp].sa = init_default_slab_allocator();
		if (chain_guard_entries[chain_guard_sp].sa == NULL)
			panic("Could not init default slab allocator");
		chain_guard_is_init = true;
	}
	chain_guard_sp++;
	ChainGuardEntry *entry = &chain_guard_entries[chain_guard_sp];
	entry->sa = sa;
	entry->sync = sync;
	entry->is_locked = false;
	ChainGuardNc ret = {entry};
	return ret;
}

int chain_malloc(FatPtr *ptr, u64 size) {
	if (!chain_guard_is_init) {
		chain_guard_entries[chain_guard_sp].sa = init_default_slab_allocator();
		if (chain_guard_entries[chain_guard_sp].sa == NULL)
			return -1;
		chain_guard_is_init = true;
	}

	if (ptr == NULL || size == 0) {
		errno = EINVAL;
		return -1;
	}
	int ret;
	if (chain_guard_entries[chain_guard_sp].sync) {
		pthread_mutex_lock(&chain_guard_entries[chain_guard_sp].lock);
		chain_guard_entries[chain_guard_sp].is_locked = true;
		ret = slab_allocator_allocate(chain_guard_entries[chain_guard_sp].sa, size, ptr);
		chain_guard_entries[chain_guard_sp].is_locked = false;
		pthread_mutex_unlock(&chain_guard_entries[chain_guard_sp].lock);
	} else {
		ret = slab_allocator_allocate(chain_guard_entries[chain_guard_sp].sa, size, ptr);
	}
	return ret;
}
int chain_realloc(FatPtr *ptr, u64 size) {
	if (!chain_guard_is_init) {
		chain_guard_entries[chain_guard_sp].sa = init_default_slab_allocator();
		if (chain_guard_entries[chain_guard_sp].sa == NULL)
			return -1;
		chain_guard_is_init = true;
	}

	if (ptr == NULL || nil(*ptr) || size == 0) {
		errno = EINVAL;
		return -1;
	}

	int ret = 0;

	if (chain_guard_entries[chain_guard_sp].sync) {
		pthread_mutex_lock(&chain_guard_entries[chain_guard_sp].lock);
		chain_guard_entries[chain_guard_sp].is_locked = true;
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
	if (chain_guard_entries[chain_guard_sp].sync) {
		chain_guard_entries[chain_guard_sp].is_locked = false;
		pthread_mutex_unlock(&chain_guard_entries[chain_guard_sp].lock);
	}
	return 0;
}

void chain_free(FatPtr *ptr) {
	if (!chain_guard_is_init) {
		panic("free called when default slab allocator has not been initialized");
	}
	if (ptr == NULL) {
		panic("attempt to free a NULL FatPtr");
	}

	if (chain_guard_entries[chain_guard_sp].sync) {
		pthread_mutex_lock(&chain_guard_entries[chain_guard_sp].lock);
		chain_guard_entries[chain_guard_sp].is_locked = true;
		slab_allocator_free(chain_guard_entries[chain_guard_sp].sa, ptr);
		chain_guard_entries[chain_guard_sp].is_locked = false;
		pthread_mutex_unlock(&chain_guard_entries[chain_guard_sp].lock);
	} else {
		slab_allocator_free(chain_guard_entries[chain_guard_sp].sa, ptr);
	}
}
