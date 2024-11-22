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

	bitmap_cleanup(&b1);
}

Test(bitmap_dirty) {
	BitMap b1;
	void *ptrs1 = map(10);
	void *ptrs10 = map(1);
	void *ptrs20 = map(1);
	void *ptrs30 = map(1);

	bitmap_init(&b1, 10, ptrs1);
	bitmap_extend(&b1, ptrs10);
	bitmap_extend(&b1, ptrs20);
	bitmap_extend(&b1, ptrs30);

	int size = PAGE_SIZE * 17;
	for (int64 i = 0; i < size; i++) {
		int64 v = bitmap_allocate(&b1);
		if (v != i) println("v=%lli,i=%lli", v, i);
		fam_assert_eq(v, i);
	}

	PageIndexPair pip = bitmap_next_dirty(&b1, -1);
	fam_assert_eq(pip.ptr_index, 0);
	pip = bitmap_next_dirty(&b1, 0);
	fam_assert_eq(pip.ptr_index, 1);
	pip = bitmap_next_dirty(&b1, 1);
	fam_assert_eq(pip.ptr_index, 2);
	pip = bitmap_next_dirty(&b1, 2);
	fam_assert_eq(pip.ptr_index, -1);

	bitmap_clean(&b1);
	pip = bitmap_next_dirty(&b1, -1);
	fam_assert_eq(pip.ptr_index, -1);

	bitmap_free(&b1, PAGE_SIZE * 14);

	pip = bitmap_next_dirty(&b1, -1);
	pip = bitmap_next_dirty(&b1, 1);

	pip = bitmap_next_dirty(&b1, 1);
	fam_assert_eq(pip.ptr_index, -1);

	bitmap_cleanup(&b1);
}

Test(cache) {
	Cache c1;
	cache_init(&c1, 10, 0.75);
	int size = 12;
	CacheItem arr[size];

	for (int i = 0; i < size; i++) {
		arr[i].id = i;
		cache_insert(&c1, &arr[i]);
	}

	fam_assert(cache_find(&c1, 0) == NULL);
	fam_assert(cache_find(&c1, 1) == NULL);
	fam_assert(cache_find(&c1, 2) != NULL);
	fam_assert(cache_find(&c1, size + 1) == NULL);
	cache_move_to_head(&c1, &arr[2]);
	CacheItem evictor = {.id = size + 1};
	cache_insert(&c1, &evictor);
	fam_assert(cache_find(&c1, size + 1) != NULL);
	fam_assert(cache_find(&c1, 2) != NULL);
	fam_assert(cache_find(&c1, 3) == NULL);

	cache_cleanup(&c1);
}

/*
Test(sys) {
	int test_dir_len = cstring_len(test_dir);
	char file[test_dir_len + 10];
	copy_bytes(file, test_dir, test_dir_len);
	copy_bytes(file + test_dir_len, "/.fam.dat", 9);
	file[test_dir_len + 9] = 0;

	int64 x1 = allocate_block();
	int64 x2 = allocate_block();

	flush();

	int64 x3 = allocate_block();
	flush();
	shutdown_sys();
	init_sys(file);
	int64 x4 = allocate_block();

	fam_assert_eq(x1 + 1, x2);
	fam_assert_eq(x2 + 1, x3);
	fam_assert_eq(x3 + 1, x4);
}

Test(sys_extend) {
	int test_dir_len = cstring_len(test_dir);
	char file[test_dir_len + 10];
	copy_bytes(file, test_dir, test_dir_len);
	copy_bytes(file + test_dir_len, "/.fam.dat", 9);
	file[test_dir_len + 9] = 0;
	int size = PAGE_SIZE * 8 - 63;
	int64 last = 0;
	for (int64 i = 0; i < size; i++) {
		int64 v = allocate_block();
		if (i && v != last + 1) println("v=%lli,last=%lli", v, last);
		if (i) fam_assert_eq(v, last + 1);
		last = v;
	}
}
*/
