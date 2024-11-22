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

#include <base/test.h>

Suite(base);

Test(bitmap) {
	BitMap b1;
	void *ptrs = map(10);
	bitmap_init(&b1, 10, ptrs);
	void *ptr0 = map(1);
	bitmap_extend(&b1, ptr0);
	void *ptr1 = map(1);
	bitmap_extend(&b1, ptr1);
	int size = PAGE_SIZE * 10;
	for (int64 i = 0; i < size; i++) {
		int64 v = bitmap_allocate(&b1);
		if (v != i) println("v=%lli,i=%lli", v, i);
		fam_assert_eq(v, i);
	}

	bitmap_free(&b1, 1);
	bitmap_free(&b1, 77);
	bitmap_free(&b1, PAGE_SIZE * 9 + 5);

	int64 v1 = bitmap_allocate(&b1);
	int64 v2 = bitmap_allocate(&b1);
	int64 v3 = bitmap_allocate(&b1);
	int64 v4 = bitmap_allocate(&b1);
	int64 v5 = bitmap_allocate(&b1);

	fam_assert_eq(v1, 1);
	fam_assert_eq(v2, 77);
	fam_assert_eq(v3, PAGE_SIZE * 9 + 5);
	fam_assert_eq(v4, size);
	fam_assert_eq(v5, size + 1);

	unmap(ptrs, 10);
	unmap(ptr0, 1);
	unmap(ptr1, 1);
}

Test(bitmap_sync) {
	BitMap b1, b2;
	void *ptrs1 = map(1);
	void *ptrs2 = map(1);
	void *ptrs10 = map(1);
	void *ptrs20 = map(1);

	bitmap_init(&b1, 10, ptrs1);
	bitmap_init(&b2, 10, ptrs2);
	bitmap_extend(&b1, ptrs10);
	bitmap_extend(&b2, ptrs20);

	int64 x1 = bitmap_allocate(&b1);
	int64 x2 = bitmap_allocate(&b1);
	int64 x3 = bitmap_allocate(&b1);
	int64 x4 = bitmap_allocate(&b1);
	int64 x5 = bitmap_allocate(&b1);

	fam_assert_eq(x1, 0);
	fam_assert_eq(x2, 1);
	fam_assert_eq(x3, 2);
	fam_assert_eq(x4, 3);
	fam_assert_eq(x5, 4);

	bitmap_sync(&b2, &b1, false);
	bitmap_clean(&b1);

	int64 x6 = bitmap_allocate(&b2);
	fam_assert_eq(x6, 5);

	x6 = bitmap_allocate(&b1);
	fam_assert_eq(x6, 5);

	unmap(ptrs1, 1);
	unmap(ptrs2, 1);
	unmap(ptrs10, 1);
	unmap(ptrs20, 1);
}

/*
Test(sys) {
	int64 x1 = allocate_block();
	int64 x2 = allocate_block();

	flush();

	int64 x3 = allocate_block();
	flush();
	shutdown_sys();
	init_sys("./.sys.fam/.fam.dat");
	int64 x4 = allocate_block();

	fam_assert_eq(x1 + 1, x2);
	fam_assert_eq(x2 + 1, x3);
	fam_assert_eq(x3 + 1, x4);
}
*/
