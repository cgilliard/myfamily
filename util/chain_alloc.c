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

#include <base/resources.h>
#include <stdio.h>
#include <util/chain_alloc.h>
#include <util/panic.h>

typedef struct ChainGuardEntry {
	struct ChainGuardEntry *next;
	struct ChainGuardEntry *prev;
	SlabAllocator *sa;
	bool sync;
} ChainGuardEntry;

_Thread_local ChainGuardEntry *chain_guard_entry_root = NULL;
_Thread_local ChainGuardEntry *chain_guard_entry_cur = NULL;

SlabAllocator *init_default_slab_allocator() {
	SlabAllocatorNc *sa = mymalloc_no_stat(sizeof(SlabAllocator));
	SlabAllocatorConfig sac;

	// default slab allocator no_malloc = false, zeroed = false, is_64_bit = false
	slab_allocator_config_build(&sac, false, false, false);

	for (i32 i = 0; i < 10; i++) {
		SlabType st = {.slab_size = (i + 3) * 8,
					   .slabs_per_resize = 10,
					   .initial_chunks = 0,
					   .max_slabs = 1000000L};
		slab_allocator_config_add_type(&sac, &st);
	}

	if (slab_allocator_build(sa, &sac))
		panic("Could not initialize default slab allocator");

	return sa;
}

void cleanup_default_slab_allocator() {
	if (chain_guard_entry_root) {
		slab_allocator_cleanup(chain_guard_entry_root->sa);
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
}

ChainGuard chain_guard_init(SlabAllocator *sa, bool is_sync) {
	if (chain_guard_entry_root == NULL) {
		chain_guard_init_root();
		ChainGuardNc ret = {chain_guard_entry_root};
		return ret;
	} else {
		ChainGuardNc ret = {NULL};
		return ret;
	}
}

int chain_malloc(FatPtr *ptr, u64 size) {
	if (chain_guard_entry_root == NULL) {
		chain_guard_init_root();
	}
	int ret = slab_allocator_allocate(chain_guard_entry_cur->sa, size, ptr);
	return ret;
}
int chain_realloc(FatPtr *ptr, u64 size) {
	return 0;
}
void chain_free(FatPtr *ptr) {
	slab_allocator_free(chain_guard_entry_cur->sa, ptr);
}
