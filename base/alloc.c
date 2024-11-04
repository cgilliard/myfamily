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

#include <base/alloc.h>
#include <base/fam_err.h>
#include <base/osdef.h>
#include <base/print_util.h>

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};

unsigned int page_aligned_size(unsigned int size) {
	size_t slabs_page_size = getpagesize();
	size_t aligned_size =
		((size_t)size + slabs_page_size - 1) & ~(slabs_page_size - 1);
	if (aligned_size >= UINT32_MAX) {
		SetErr(Overflow);
		return UINT32_MAX;
	}
	return aligned_size;
}

Alloc alloc(unsigned int size) {
	unsigned int aligned_size = page_aligned_size(size);
	if (aligned_size >= UINT32_MAX) {
		SetErr(Overflow);
		Alloc ret = {};
		return ret;
	}
	void *ret = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.alloc_sum += 1;
	} else {
		SetErr(AllocErr);
	}

	Alloc aret = {.ptr = ret, .size = aligned_size};
	return aret;
}

void release(Alloc ptr) {
	int code;
	if ((code = munmap(ptr.ptr, ptr.size))) {
		panic("munmap error = %i\n", code);
	}
	THREAD_LOCAL_RESOURCE_STATS.release_sum += 1;
}

int64 alloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.alloc_sum;
}
int64 resize_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.resize_sum;
}
int64 release_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.release_sum;
}
