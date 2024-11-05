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

MyTest(base, test_lock) {
	Lock l1 = INIT_LOCK;
	lock_read(&l1);
	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 1);
	// lower bits also incremeneted by 1.
	cr_assert_eq(l1 & 0xFFFFFFFF, 1);

	lock_unlock(&l1);
	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 2);
	// lower bits also decremented by 1.
	cr_assert_eq(l1 & 0xFFFFFFFF, 0);

	lock_write(&l1);
	// upper bits incremented by 2.
	cr_assert_eq(l1 >> 32, 4);
	// lower bits high bit set
	cr_assert_eq(l1 & 0xFFFFFFFF, 0x80000000);

	lock_unlock(&l1);

	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 5);
	// lower bits are set to 0 now
	cr_assert_eq(l1 & 0xFFFFFFFF, 0x00000000);
}

MyTest(base, test_lock_macros) {
	int x = 0;
	Lock l = INIT_LOCK;
	wsync(l, { x++; });
	cr_assert(x == 1);
	int y;
	rsync(l, { y = x; });
	cr_assert(y == 1);
}

MyTest(base, test_alloc) {
	Alloc test1 = alloc(100);
	cr_assert_eq(test1.size, page_aligned_size(100));
	cr_assert(test1.ptr != NULL);
	release(test1);
}

MyTest(base, test_slab_allocator) {
	SlabAllocator sa1;
	cr_assert(!slab_allocator_init(&sa1, 16, 100, 200));
	Slab s1 = slab_allocator_allocate(&sa1);
	cr_assert(s1 != NULL);
	Slab s2 = slab_allocator_allocate(&sa1);
	cr_assert(s2 != NULL);
	Slab s3 = slab_allocator_allocate(&sa1);
	cr_assert(s3 != NULL);

	slab_allocator_free(&sa1, s3);
	slab_allocator_free(&sa1, s2);
	slab_allocator_free(&sa1, s1);

	s2 = slab_allocator_allocate(&sa1);
	cr_assert(s2 != NULL);
	s1 = slab_allocator_allocate(&sa1);
	cr_assert(s1 != NULL);
	s3 = slab_allocator_allocate(&sa1);
	cr_assert(s3 != NULL);

	Slab s4 = slab_allocator_allocate(&sa1);
	cr_assert(s4 != NULL);

	slab_allocator_free(&sa1, s4);
	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_slab_allocator_perf) {
	SlabAllocator sa1;
	// long long size = 4 * 1024L * 1024L * 1024L;
	long long size = 100;
	cr_assert(!slab_allocator_init(&sa1, 8, size, size));
	Slab *arr = malloc(sizeof(Slab *) * size);

	for (long long i = 0; i < size; i++) {
		// if (i % 1000000 == 0) println("i=%lli", i);
		arr[i] = slab_allocator_allocate(&sa1);
		cr_assert(arr[i] != NULL);
	}

	for (long long i = 0; i < size; i++) {
		// if (i % 1000000 == 0) println("free i=%lli", i);
		slab_allocator_free(&sa1, arr[i]);
	}

	free(arr);
	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_slab_allocator_recycle) {
	long long size = 10000;
	int count = 5;
	int alloc_size = 8;

	SlabAllocator sa1;
	cr_assert(!slab_allocator_init(&sa1, alloc_size, count + 5, count + 5));

	Slab s1 = slab_allocator_allocate(&sa1);
	Slab slabs[count];

	for (long long i = 0; i < size; i++) {
		for (int j = 0; j < count; j++) {
			slabs[j] = slab_allocator_allocate(&sa1);
			cr_assert(slabs[j] != NULL);
			byte *test = slab_get(slabs[j]);
			for (int j = 0; j < alloc_size; j++) test[j] = j;
		}
		for (int j = 0; j < count; j++) {
			byte *test = slab_get(slabs[j]);
			for (int j = 0; j < alloc_size; j++) cr_assert(test[j] == j);
			slab_allocator_free(&sa1, slabs[j]);
		}

		// there is one slab allocated in init (required) then s1 + count
		// while these slabs are released to the slab allocator, they are not
		// 'freed' because our max_free_slabs is count + 5.
		// at the end of the loop, all of these are in the free list except s1.
		cr_assert_eq(slab_allocator_total_slabs(&sa1), count + 2);
		cr_assert_eq(slab_allocator_free_size(&sa1), count + 1);
	}

	cr_assert_eq(slab_allocator_total_slabs(&sa1), count + 2);
	cr_assert_eq(slab_allocator_free_size(&sa1), count + 1);
	slab_allocator_free(&sa1, s1);
	cr_assert_eq(slab_allocator_total_slabs(&sa1), count + 2);
	cr_assert_eq(slab_allocator_free_size(&sa1), count + 2);

	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_malloc_recycle) {
	long long size = 1000;
	int count = 5;
	int alloc_size = 8;

	byte *slabs[count];

	for (long long i = 0; i < size; i++) {
		for (int j = 0; j < count; j++) {
			slabs[j] = malloc(alloc_size);
			cr_assert(slabs[j] != NULL);
			for (int k = 0; k < alloc_size; k++) slabs[j][k] = j;
		}

		for (int j = 0; j < count; j++) {
			for (int k = 0; k < alloc_size; k++) cr_assert(slabs[j][k] == j);
			free(slabs[j]);
		}
	}
}
