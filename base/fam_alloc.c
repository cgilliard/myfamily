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
#include <base/osdef.h>
#include <base/print_util.h>
#include <base/string.h>
#include <pthread.h>

#define PTR_FLAGS_SEND 0
#define PTR_FLAGS_DIRECT 1

_Thread_local SlabAllocator tl_slab_allocator = NULL;
SlabAllocator global_slab_allocator = NULL;
pthread_mutex_t global_allocator_lock = PTHREAD_MUTEX_INITIALIZER;

int check_initialize_default_slab_allocator() {
	if (tl_slab_allocator == NULL)
		tl_slab_allocator = slab_allocator_create();
	if (global_slab_allocator == NULL) {
		pthread_mutex_lock(&global_allocator_lock);
		if (global_slab_allocator == NULL)
			global_slab_allocator = slab_allocator_create();
		pthread_mutex_unlock(&global_allocator_lock);
	}
	return tl_slab_allocator && global_slab_allocator;
}

Ptr fam_alloc(unsigned int size, bool send) {
	if (!check_initialize_default_slab_allocator())
		return NULL;
	Ptr ret;
	if (size > MAX_SLAB_SIZE)
		ret = ptr_direct_alloc(size);
	else if (send) {
		pthread_mutex_lock(&global_allocator_lock);
		ret = slab_allocator_allocate(global_slab_allocator, size);
		pthread_mutex_unlock(&global_allocator_lock);
	} else
		ret = slab_allocator_allocate(tl_slab_allocator, size);

	if (ret) {
		ptr_flag_set(ret, PTR_FLAGS_SEND, send);
		ptr_flag_set(ret, PTR_FLAGS_DIRECT, size > MAX_SLAB_SIZE);
	}

	return ret;
}
Ptr fam_resize(Ptr ptr, unsigned int size) {
	if (!check_initialize_default_slab_allocator())
		return NULL;

	if (ptr == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}

	bool send = ptr_flag_check(ptr, PTR_FLAGS_SEND);
	Ptr ret = fam_alloc(size, send);
	if (ret) {
		unsigned int len = $len(ptr);
		if (len > size)
			len = size;
		memcpy($(ret), $(ptr), len);
	}

	return ret;
}

void fam_release(Ptr *ptr) {
	if (nil(ptr)) {
		panic("fam_free on nil ptr!");
	} else if (ptr_flag_check(*ptr, PTR_FLAGS_DIRECT)) {
		ptr_direct_release(*ptr);
	} else if (ptr_flag_check(*ptr, PTR_FLAGS_SEND)) {
		pthread_mutex_lock(&global_allocator_lock);
		slab_allocator_free(global_slab_allocator, *ptr);
		pthread_mutex_unlock(&global_allocator_lock);
	} else
		slab_allocator_free(tl_slab_allocator, *ptr);
}

void fam_alloc_thread_local_cleanup() {
	slab_allocator_cleanup(&tl_slab_allocator);
}

#ifdef TEST
void fam_alloc_global_cleanup() {
	slab_allocator_cleanup(&global_slab_allocator);
}

int64 fam_alloc_count_tl_slab_allocator() {
	if (tl_slab_allocator == NULL)
		return 0;
	return slab_allocator_cur_slabs_allocated(tl_slab_allocator);
}
int64 fam_alloc_count_global_allocator() {
	if (global_slab_allocator == NULL)
		return 0;
	return slab_allocator_cur_slabs_allocated(global_slab_allocator);
}
#endif // TEST
