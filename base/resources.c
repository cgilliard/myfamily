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
#include <base/types.h>
#include <stdio.h>
#include <stdlib.h>

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};
#ifdef TEST
bool __is_debug_malloc = false;
bool __is_debug_realloc = false;
#endif // TEST

void *mymalloc(u64 size) {
#ifdef TEST
	if (__is_debug_malloc) {
		return NULL;
	}
#endif // TEST
	void *ret;
	ret = malloc(size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.malloc_sum += 1;
	}
	return ret;
}
void *myrealloc(void *ptr, u64 size) {
#ifdef TEST
	if (__is_debug_realloc) {
		return NULL;
	}
#endif // TEST
	void *ret;
	ret = realloc(ptr, size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.realloc_sum += 1;
	}
	return ret;
}
void myfree(void *ptr) {
	THREAD_LOCAL_RESOURCE_STATS.free_sum += 1;
	free(ptr);
}
MYFILE *myfopen(const Path *path, const char *mode) {
	const char *path_str = path_to_string(path);
	FILE *ret = fopen(path_str, mode);
	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.fopen_sum += 1;
	}
	return (MYFILE *)ret;
}
void myfclose(MYFILE *ptr) {
	THREAD_LOCAL_RESOURCE_STATS.fclose_sum += 1;
	fclose((FILE *)ptr);
}
u64 mymalloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.malloc_sum;
}
u64 myrealloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.realloc_sum;
}
u64 myfree_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.free_sum;
}
u64 myfopen_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.fopen_sum;
}
u64 myfclose_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.fclose_sum;
}
