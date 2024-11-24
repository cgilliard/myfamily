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

/*
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
*/

Test(cache) {
	Cache c1;
	cache_init(&c1, 10, 0.75);
	int size = 12;
	Block arr[size];
	for (int i = 0; i < size; i++) {
		arr[i].id = i;
		cache_insert(&c1, &arr[i]);
	}
	fam_assert(cache_find(&c1, 0) == NULL);
	fam_assert(cache_find(&c1, 1) == NULL);
	fam_assert(cache_find(&c1, 2) != NULL);
	fam_assert(cache_find(&c1, size + 1) == NULL);
	cache_move_to_head(&c1, &arr[2]);
	Block evictor = {.id = size + 1};
	cache_insert(&c1, &evictor);
	fam_assert(cache_find(&c1, size + 1) != NULL);
	fam_assert(cache_find(&c1, 2) != NULL);
	fam_assert(cache_find(&c1, 3) == NULL);

	cache_cleanup(&c1, false);
}

Test(block) {
	Block *item1 = block_load(0);
	byte *ref;

	ref = item1->addr;
	for (int i = 0; i < 10; i++) ref[i] = i + 'a';

	Block *item2 = block_load(1);
	ref = item2->addr;
	for (int i = 0; i < 10; i++) ref[i] = i + 'A';

	Block *item3 = block_load(0);
	ref = item3->addr;
	for (int i = 0; i < 10; i++) fam_assert_eq(ref[i], i + 'a');

	Block *item4 = block_load(1);
	ref = item4->addr;
	for (int i = 0; i < 10; i++) fam_assert_eq(ref[i], i + 'A');

	Block *item5 = block_load(0);
	ref = item5->addr;
	for (int i = 0; i < 10; i++) fam_assert_eq(ref[i], i + 'a');

	Block *item6 = block_load(1);
	ref = item6->addr;
	for (int i = 0; i < 10; i++) fam_assert_eq(ref[i], i + 'A');
}

int count = 5000;
int size = 50;
int alloc_size = 64 - SLAB_LIST_SIZE;

Test(slab_allocator) {
	SlabAllocator sa1;
	fam_assert(!slab_allocator_init(&sa1, alloc_size, size + 5, size + 5));
	byte **arr = map(1 + (size * sizeof(byte *)) / PAGE_SIZE);

	for (int64 i = 0; i < count; i++) {
		for (int64 j = 0; j < size; j++) {
			arr[j] = slab_allocator_allocate(&sa1);
			for (int k = 0; k < alloc_size; k++)
				arr[j][k] = ((i + j + k) % 26) + 'a';
		}

		for (int64 j = 0; j < size; j++) {
			for (int k = 0; k < alloc_size; k++) {
				fam_assert_eq(arr[j][k], ((i + j + k) % 26) + 'a');
			}
			slab_allocator_free(&sa1, arr[j]);
		}
	}

	slab_allocator_cleanup(&sa1);
	unmap(arr, 1 + (size * sizeof(byte *)) / PAGE_SIZE);
}

#include <stdlib.h>

Test(malloc) {
	byte **arr = map(1 + (size * sizeof(byte *)) / PAGE_SIZE);

	for (int64 i = 0; i < count; i++) {
		for (int64 j = 0; j < size; j++) {
			arr[j] = malloc(alloc_size);
			for (int k = 0; k < alloc_size; k++)
				arr[j][k] = ((i + j + k) % 26) + 'a';
		}

		for (int64 j = 0; j < size; j++) {
			for (int k = 0; k < alloc_size; k++) {
				fam_assert_eq(arr[j][k], ((i + j + k) % 26) + 'a');
			}
			free(arr[j]);
		}
	}

	unmap(arr, 1 + (size * sizeof(byte *)) / PAGE_SIZE);
}
