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

#include <base/tlmalloc.h>
#include <stdlib.h>

_Thread_local TLMallocStats THREAD_LOCAL_TLMALLOC = {0, 0, 0, 0};

void *tlmalloc(usize size) {
	void *ret;
	ret = malloc(size);

	if (ret) {
		THREAD_LOCAL_TLMALLOC.cur_bytes += size;
		THREAD_LOCAL_TLMALLOC.cur_allocs += 1;
	}
	return ret;
}
void *tlrealloc(void *ptr, usize size) {
	void *ret;
	ret = realloc(ptr, size);
	if (ret) {
		THREAD_LOCAL_TLMALLOC.cur_bytes += size;
		THREAD_LOCAL_TLMALLOC.cur_reallocs += 1;
	}
	return ret;
}

void tlfree(void *ptr) {
	THREAD_LOCAL_TLMALLOC.cur_frees += 1;
	free(ptr);
}

u128 cur_bytes_alloc() { return THREAD_LOCAL_TLMALLOC.cur_bytes; }
u64 alloc_count() { return THREAD_LOCAL_TLMALLOC.cur_allocs; }
u64 realloc_count() { return THREAD_LOCAL_TLMALLOC.cur_reallocs; }
u64 free_count() { return THREAD_LOCAL_TLMALLOC.cur_frees; }
