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
#include <base/osdef.h>

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};

void *alloc(number size, number zeroed) {
	if (!size) {
		SetErr(IllegalArgument);
		return NULL;
	}
	void *ret;
	ret = malloc(size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.alloc_sum += 1;
		if (zeroed)
			memset(ret, 0, size);
	} else
		SetErr(AllocErr);
	return ret;
}
void *resize(void *ptr, number size) {
	if (!ptr || !size) {
		SetErr(IllegalArgument);
		return NULL;
	}
	void *ret;
	ret = realloc(ptr, size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.resize_sum += 1;
	} else
		SetErr(AllocErr);
	return ret;
}

void release(void *ptr) {
	if (!ptr) {
		SetErr(IllegalArgument);
		return;
	}
	THREAD_LOCAL_RESOURCE_STATS.release_sum += 1;
	free(ptr);
}

number alloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.alloc_sum;
}
number resize_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.resize_sum;
}
number release_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.release_sum;
}

char *getenv(const char *name);

byte *env(const byte *name) {
	return getenv(name);
}
