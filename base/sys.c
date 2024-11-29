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

#include <base/print_util.h>
#include <base/sys.h>
#include <sys/mman.h>

void *map(long long pages) {
	if (pages == 0) return 0;
	void *ret = mmap(0, pages * PAGE_SIZE, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return ret;
}

void unmap(void *addr, long long pages) {
	if (pages == 0) return;
	munmap(addr, pages * PAGE_SIZE);
}

static void check_arch(char *type, int actual, int expected) {
	if (actual != expected)
		panic("{} must be {} bytes. It is {} bytes. Arch invalid.", type,
			  expected, actual);
}

#define arch(type, expected) check_arch(#type, sizeof(type), expected)

void __attribute__((constructor)) __check_sizes() {
	arch(int, 4);
	arch(long long, 8);
	arch(unsigned long long, 8);
	arch(unsigned long, 8);
	arch(__uint128_t, 16);
	arch(char, 1);
	arch(unsigned char, 1);
	arch(float, 4);
	arch(double, 8);
	if (__SIZEOF_SIZE_T__ != 8)
		panic("size_t must be 8 bytes. It is {} bytes. Arch invalid.",
			  __SIZEOF_SIZE_T__);

	// little endian check
	int test = 0x1;
	if (*(unsigned char *)&test != 0x1) panic("Big endian is not supported!");
}
