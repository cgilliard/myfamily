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

#include <base/lib.h>
#include <base/test.h>

Suite(base);

Test(colors) {
	byte buf[1024];
	int len;

	len = sprint(buf, 1024, "Colors: %sgreen%s %sdimmed%s %sred%s %smagenta%s",
				 GREEN, RESET, DIMMED, RESET, RED, RESET, MAGENTA, RESET);
	fam_assert_eq(len, 67);
	_debug_no_color__ = true;
	len = sprint(buf, 1024,
				 "Colors: %sgreen%s %sdimmed%s %sred%s %smagenta%s %sbred%s "
				 "%syellow%s %scyan%s %sblue%s",
				 GREEN, RESET, DIMMED, RESET, RED, RESET, MAGENTA, RESET,
				 BRIGHT_RED, RESET, YELLOW, RESET, CYAN, RESET, BLUE, RESET);
	fam_assert_eq(len, 54);
	test_reset_colors();
}

Test(bitmap) {
	BitMap b1;
	bitmap_init(&b1, 10);

	int64 last = 0;
	for (int i = 0; i < (2048 * 64) + 1; i++) {
		int64 v1 = bitmap_allocate(&b1);
		if (i) fam_assert_eq(v1, last + 1);
		last = v1;
	}

	bitmap_free(&b1, 100);
	bitmap_free(&b1, 200);
	bitmap_free(&b1, 300);

	int64 n = bitmap_allocate(&b1);
	fam_assert_eq(n, 100);
	fam_assert_eq(bitmap_allocate(&b1), 200);
	fam_assert_eq(bitmap_allocate(&b1), 300);
	fam_assert_eq(bitmap_allocate(&b1), (2048 * 64) + 1);

	bitmap_cleanup(&b1);
}

Test(alloc_max_slabs) {
	Alloc a1;
	int64 size = PAGE_SIZE * 8, len = 128;
	fam_assert(!alloc_init(&a1, len, PAGE_SIZE * 8));

	Slab *arr = (Slab *)map(1 + (sizeof(Slab) * size) / PAGE_SIZE);

	for (int64 i = 0; i < size; i++) {
		arr[i] = alloc(&a1);
		fam_assert_eq(arr[i].ptr, i);
		for (int j = 0; j < len; j++) {
			arr[i].data[j] = ((i + j) % 26) + 'a';
		}
	}
	Slab inv = alloc(&a1);
	fam_assert_eq(inv.ptr, 0);
	fam_assert_eq(inv.data, NULL);

	alloc_cleanup(&a1);
	unmap((byte *)arr, 1 + (sizeof(Slab) * size) / PAGE_SIZE);
}

Test(alloc_big) {
	Alloc a1;
	// int64 size = (1024LL * 1024LL * 1024LL * 4LL), len = 8;
	int64 size = 1000 * 100, len = 128;

	fam_assert(!alloc_init(&a1, len, UINT32_MAX));

	Slab *arr = (Slab *)map(1 + (sizeof(Slab) * size) / PAGE_SIZE);

	for (int64 i = 0; i < size; i++) {
		arr[i] = alloc(&a1);
		fam_assert_eq(arr[i].ptr, i);
		for (int j = 0; j < len; j++) {
			arr[i].data[j] = ((i + j) % 26) + 'a';
		}
	}

	// Slab inv = alloc(&a1);
	// fam_assert_eq(inv.ptr, 0);

	for (int64 i = 0; i < size; i++) {
		for (int j = 0; j < len; j++) {
			fam_assert_eq(arr[i].data[j], ((i + j) % 26) + 'a');
		}
		release(&a1, arr[i].ptr);
	}

	alloc_cleanup(&a1);
	unmap((byte *)arr, 1 + (sizeof(Slab) * size) / PAGE_SIZE);
}
