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
#include <base/osdef.h>
#include <criterion/criterion.h>
#include <stdio.h>

void test_spawn() {
	printf("test spawn\n");
}

void test_spawn2() {
	printf("test spawn2\n");
}

void test_main() {
	printf("test_main\n");

	spawn(test_spawn);
	spawn(test_spawn2);

	printf("ret from spawn\n");
}

Test(base, test_base) {
	init(test_main);
}

Test(base, test_alloc) {
	cr_assert_eq(alloc_sum(), 0);
	Alloc test1 = alloc(100);
	cr_assert_eq(test1.size, slabs_page_size);
	release(test1);
	cr_assert_eq(alloc_sum(), 1);
	cr_assert_eq(alloc_sum(), release_sum());
}

Test(base, test_ptr) {
	cr_assert_eq(alloc_sum(), 0);
	Ptr ptr = ptr_direct_alloc(100);
	cr_assert_eq(ptr_len(ptr), slabs_page_size - slab_overhead());
	char *test = ptr_data(ptr);
	for (int i = 0; i < ptr_len(ptr); i++) {
		test[i] = i % 128;
	}
	for (int i = 0; i < ptr_len(ptr); i++) {
		cr_assert_eq(test[i], i % 128);
	}
	cr_assert(!nil(ptr));
	cr_assert(ok(ptr));
	$release(ptr);
	cr_assert(nil(ptr));
	cr_assert(!ok(ptr));
	cr_assert_eq(alloc_sum(), 1);
	cr_assert_eq(alloc_sum(), release_sum());

	Ptr next = ptr_direct_alloc(1000);
	cr_assert_eq(ptr_len(next), slabs_page_size - slab_overhead());
	unsigned int initial_size = ptr_len(next);

	char *buf = ptr_data(next);
	for (int i = 0; i < 1000; i++)
		buf[i] = 'a' + (i % 26);

	Ptr nptr = ptr_direct_resize(next, 100000);
	cr_assert(nptr);
	cr_assert(ptr_len(nptr) > initial_size);
	char *buf2 = ptr_data(nptr);
	for (int i = 0; i < 1000; i++)
		cr_assert_eq(buf2[i], 'a' + (i % 26));

	$release(nptr);
	cr_assert_eq(alloc_sum(), release_sum());
}

Test(base, test_slab_sizes) {
	int i = 0;
	int j;
	int k;
	int i_last = 0;
	int j_last = 0;
	int k_last = 0;
	while ((j = slab_allocator_get_index(i)) >= 0) {
		k = slab_allocator_get_size(j);
		// assert slab size is greater than or equal to needed bytes
		cr_assert(k >= i);

		cr_assert(i >= i_last);
		cr_assert(j >= j_last);
		cr_assert(k >= k_last);

		i_last = i;
		j_last = j;
		k_last = k;
		i++;
	}

	// largest slabs size is this - 1 (65536)
	cr_assert_eq(i, 65537);
}

Test(base, test_slab_allocator) {

	{ SlabAllocator sa = slab_allocator_create(); }
	cr_assert(alloc_sum() > 0);
	cr_assert_eq(alloc_sum(), release_sum());
}
