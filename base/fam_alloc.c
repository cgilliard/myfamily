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

#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/panic.h>
#include <base/resources.h>
#include <base/slabs.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

SlabAllocator *global_allocator = NULL;
_Thread_local SlabAllocator *thread_local_slab_allocator = NULL;

#define MAX_FAM_GUARDS 255

pthread_mutex_t global_allocator_lock;

typedef struct FamGuardEntry {
	bool send;
} FamGuardEntry;

_Thread_local static FamGuardEntry fam_guard_entries[MAX_FAM_GUARDS];
_Thread_local static u8 fam_guard_sp = 0;
_Thread_local static bool fam_guard_is_init = false;

SlabAllocator *initialize_default_slab_allocator(bool global) {
	SlabAllocatorNc *sa = mymalloc(sizeof(SlabAllocator));
	if (sa == NULL)
		return NULL;
	SlabAllocatorConfig sac;

	// default slab allocator zeroed = false
	if (slab_allocator_config_build(&sac, false, global)) {
		myfree(sa);
		return NULL;
	}

	u32 max_slabs = MAX_SLABS;
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

void init_fam_guards() {
	if (!fam_guard_is_init) {
		fam_guard_is_init = true;
		fam_guard_entries[0].send = false;
		if (thread_local_slab_allocator == NULL)
			thread_local_slab_allocator = initialize_default_slab_allocator(false);
		if (global_allocator == NULL)
			global_allocator = initialize_default_slab_allocator(true);
		if (pthread_mutex_init(&global_allocator_lock, NULL))
			panic("Could not init pthread_mutex");
	}
}

void send_guard_cleanup(SendStateGuardNc *ptr) {
	fam_guard_sp--;
}

int fam_alloc(FatPtr *ptr, u64 size) {
	init_fam_guards();
	bool send = fam_guard_entries[fam_guard_sp].send;

	SlabAllocatorNc *sa;

	if (send)
		sa = global_allocator;
	else
		sa = thread_local_slab_allocator;

	int ret;
	if (send)
		pthread_mutex_lock(&global_allocator_lock);
	ret = slab_allocator_allocate(sa, size, ptr);
	if (send)
		pthread_mutex_unlock(&global_allocator_lock);

	if (ret) { // try malloc
		ret = fat_ptr_mallocate(ptr, size);
	}

	return ret;
}
int fam_realloc(FatPtr *ptr, u64 size) {
	if (fat_ptr_is_pin(ptr)) {
		fam_err = IllegalState;
		return -1;
	}
	init_fam_guards();
	int ret = 0;
	bool send = fam_guard_entries[fam_guard_sp].send;

	FatPtr tmp = null;

	if (send)
		pthread_mutex_lock(&global_allocator_lock);

	ret = fam_alloc(&tmp, size);
	if (ret || nil(tmp)) {
		fam_err = AllocErr;
		ret = -1;
	} else {
		u64 len = $size(*ptr);
		u64 nlen = $size(tmp);
		if (nlen < len) {
			len = nlen;
		}

		memcpy(fat_ptr_data(&tmp), fat_ptr_data(ptr), len);

		fam_free(ptr);
		*ptr = tmp;
	}
	if (send)
		pthread_mutex_unlock(&global_allocator_lock);

	return ret;
}
void fam_free(FatPtr *ptr) {
	if (fat_ptr_is_malloc(ptr)) {
		fat_ptr_malloc_free(ptr);
	} else {
		init_fam_guards();

		bool global = fat_ptr_is_global(ptr);

		SlabAllocatorNc *sa;

		if (global)
			sa = global_allocator;
		else
			sa = thread_local_slab_allocator;

		if (global)
			pthread_mutex_lock(&global_allocator_lock);
		slab_allocator_free(sa, ptr);
		if (global)
			pthread_mutex_unlock(&global_allocator_lock);
	}
}
void fam_alloc_increment_state(bool send) {
	init_fam_guards();
	fam_guard_sp++;
	fam_guard_entries[fam_guard_sp].send = send;
}

#ifdef TEST
void fam_alloc_cleanup() {
	if (fam_guard_is_init) {
		slab_allocator_cleanup(thread_local_slab_allocator);
		pthread_mutex_lock(&global_allocator_lock);
		slab_allocator_cleanup(global_allocator);
		pthread_mutex_unlock(&global_allocator_lock);
		myfree(global_allocator);
		myfree(thread_local_slab_allocator);
	}
}

u64 fam_alloc_count_tl_slab_allocator() {
	if (!fam_guard_is_init)
		return 0;
	return slab_allocator_cur_slabs_allocated(thread_local_slab_allocator);
}
u64 fam_alloc_count_global_allocator() {
	if (!fam_guard_is_init)
		return 0;
	pthread_mutex_lock(&global_allocator_lock);
	u64 ret = slab_allocator_cur_slabs_allocated(global_allocator);
	pthread_mutex_unlock(&global_allocator_lock);

	return ret;
}
#endif // TEST
