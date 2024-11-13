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

#include <base/mmap.h>
#include <base/print_util.h>
#include <base/types.h>
#include <sys/mman.h>

int64 _allocation_sum = 0;
static int mmap_fail = -1;

size_t getpagesize();

void *mmap_allocate(unsigned long long size) {
	if (mmap_fail >= 0) {
		if (mmap_fail == 0) {
			mmap_fail--;
			return NULL;
		}
		mmap_fail--;
	}
	unsigned long long aligned_size = mmap_aligned_size(size);
	void *ret = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ret) _allocation_sum += aligned_size;
	return ret;
}

void mmap_free(void *ptr, unsigned long long size) {
	unsigned long long aligned_size = mmap_aligned_size(size);
	_allocation_sum -= aligned_size;

	if (munmap(ptr, aligned_size)) panic("unexpected mmap error!");
}

unsigned long long mmap_aligned_size(unsigned long long size) {
	unsigned long long page_size = getpagesize();
	return (size + (page_size - 1)) & ~(page_size - 1);
}

void set_mmap_fail(int count) {
	mmap_fail = count;
}
