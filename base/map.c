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

#include <base/map.h>
#include <base/print_util.h>
#include <errno.h>
#include <sys/mman.h>

int64 _alloc_sum = 0;

char *strerror(int err);

void *map(int64 pages) {
	if (pages == 0) return NULL;
	_alloc_sum += pages;
	return mmap(NULL, pages * PAGE_SIZE, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void unmap(void *addr, int64 pages) {
	_alloc_sum -= pages;

	if (munmap(addr, pages * PAGE_SIZE))
		panic("munmap error: %s", strerror(errno));
}
