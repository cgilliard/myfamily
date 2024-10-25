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

#include <base/fam_err.h>
#include <base/os.h>

#include <unistd.h>

u8 *getenv(const u8 *name);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};

void *alloc(u64 size, bool zeroed) {
	void *ret;
	ret = malloc(size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.alloc_sum += 1;
	} else
		SetErr(AllocErr);
	return ret;
}
void *resize(void *ptr, u64 size, bool zeroed) {
	void *ret;
	ret = realloc(ptr, size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.resize_sum += 1;
	} else
		SetErr(AllocErr);
	return ret;
}

void release(void *ptr) {
	THREAD_LOCAL_RESOURCE_STATS.release_sum += 1;
	free(ptr);
}

u64 alloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.alloc_sum;
}
u64 resize_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.resize_sum;
}
u64 release_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.release_sum;
}

// Persistence
void persistent_set_root(const u8 *path) {
}
void *persistent_alloc(const u8 *name, u64 size, bool zeroed) {
	return NULL;
}
void *persistent_resize(const u8 *name, u64 size) {
	return NULL;
}
void persistent_sync(void *ptr) {
}
void persistent_delete(const u8 *name) {
}

u8 *env(const u8 *name) {
	return getenv(name);
}

i32 write_impl(i32 fd, const void *buf, u64 len) {
	return write(fd, buf, len);
}
