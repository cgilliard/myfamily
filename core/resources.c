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

#include <core/resources.h>
#include <stdlib.h>

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};

void *mymalloc(usize size) {
	void *ret;
	ret = malloc(size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.malloc_sum += 1;
	}
	return ret;
}
void *myrealloc(void *ptr, usize size) {
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
FILE *myfopen(char *path, const char *mode) {
	FILE *ret = fopen(path, mode);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.fopen_sum += 1;
	}
	return ret;
}
void myfclose(FILE *ptr) {
	THREAD_LOCAL_RESOURCE_STATS.fclose_sum += 1;
	fclose(ptr);
}
u64 mymalloc_sum() { return THREAD_LOCAL_RESOURCE_STATS.malloc_sum; }
u64 myrealloc_sum() { return THREAD_LOCAL_RESOURCE_STATS.realloc_sum; }
u64 myfree_sum() { return THREAD_LOCAL_RESOURCE_STATS.free_sum; }
u64 myfopen_sum() { return THREAD_LOCAL_RESOURCE_STATS.fopen_sum; }
u64 myfclose_sum() { return THREAD_LOCAL_RESOURCE_STATS.fclose_sum; }
