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

MyTest(base, test_memmap) {
	MemMap mm1;
	int size = 64;
	int count = 3000;
	memmap_init(&mm1, size);

	for (int i = 0; i < count; i++) {
		Ptr p = memmap_allocate(&mm1);
		if (p != i + 2) println("p=%u,i+1=%i", p, i + 2);
		cr_assert_eq(p, i + 2);
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			data[j] = 'a' + (j % 26);
		}
	}

	for (int i = 0; i < count; i++) {
		Ptr p = i + 2;
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			cr_assert_eq(data[j], 'a' + (j % 26));
		}
	}

	memmap_free(&mm1, 7);
	memmap_free(&mm1, 8);
	memmap_free(&mm1, 167);

	Ptr p = memmap_allocate(&mm1);
	cr_assert_eq(p, 7);
	p = memmap_allocate(&mm1);
	cr_assert_eq(p, 8);
	p = memmap_allocate(&mm1);
	cr_assert_eq(p, 167);

	p = memmap_allocate(&mm1);
	cr_assert_eq(p, count + 2);

	memmap_cleanup(&mm1);
}

MyTest(base, test_memmap_recycle) {
	MemMap mm1;
	int size = 64;
	int count = 3000000;
	int itt = 4;
	memmap_init(&mm1, size);
	Ptr ptrs[itt];

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < itt; j++) {
			ptrs[j] = memmap_allocate(&mm1);
			cr_assert(ptrs[j] == 2 + j);
		}

		for (int j = 0; j < itt; j++) {
			cr_assert(ptrs[j] == 2 + j);
			memmap_free(&mm1, ptrs[j]);
		}
	}
	memmap_cleanup(&mm1);
}

MyTest(base, test_slab_allocator) {
	SlabAllocator sa1;
	cr_assert(!slab_allocator_init(&sa1, 16, 100, 200));
	int size = 5;
	Ptr arr[size];

	for (int i = 0; i < size; i++) {
		Ptr p = slab_allocator_allocate(&sa1);
		arr[i] = p;
		byte *parr = slab_get(&sa1, p);
		parr[0] = i + 2;
	}

	for (int i = 0; i < size; i++) {
		byte *parr = slab_get(&sa1, arr[i]);
		cr_assert_eq(parr[0], i + 2);
		slab_allocator_free(&sa1, arr[i]);
	}

	slab_allocator_cleanup(&sa1);
}

/*
MyTest(base, test_slab_allocator) {
	SlabAllocator sa1;
	cr_assert(!slab_allocator_init(&sa1, 16, 100, 200, true));
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

	slab_allocator_free(&sa1, s1);
	slab_allocator_free(&sa1, s2);
	slab_allocator_free(&sa1, s3);
	slab_allocator_free(&sa1, s4);
	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_slab_allocator_perf) {
	SlabAllocator sa1;
	// long long size = 4 * 1024L * 1024L * 1024L;
	long long size = 100;
	cr_assert(!slab_allocator_init(&sa1, 8, size, size, true));
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
	long long size = 1024 * 1024 * 10;
	int count = 4;
	int alloc_size = 8;

	SlabAllocator sa1;
	cr_assert(
		!slab_allocator_init(&sa1, alloc_size, count + 5, count + 5, true));

	Slab s1 = slab_allocator_allocate(&sa1);
	Slab slabs[count];

	for (long long i = 0; i < size; i++) {
		// if (i % 1000000 == 0) println("i=%lli", i);
		for (int j = 0; j < count; j++) {
			slabs[j] = slab_allocator_allocate(&sa1);
			cr_assert(slabs[j] != NULL);
			byte *test = slab_get(&sa1, slabs[j]);
			for (int j = 0; j < alloc_size; j++) test[j] = j;
		}
		for (int j = 0; j < count; j++) {
			byte *test = slab_get(&sa1, slabs[j]);
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

	Slab s2 = slab_allocator_allocate(&sa1);
	cr_assert(s2 != NULL);
	byte *test = slab_get(&sa1, s2);
	for (int j = 0; j < alloc_size; j++) test[j] = j;
	cr_assert_eq(slab_allocator_total_slabs(&sa1), count + 2);
	cr_assert_eq(slab_allocator_free_size(&sa1), count + 1);
	slab_allocator_free(&sa1, s2);
	cr_assert_eq(slab_allocator_total_slabs(&sa1), count + 2);
	cr_assert_eq(slab_allocator_free_size(&sa1), count + 2);

	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_malloc_recycle) {
	long long size = 3000000;
	int count = 4;
	int alloc_size = 64;

	byte *slabs[count];

	for (long long i = 0; i < size; i++) {
		for (int j = 0; j < count; j++) {
			slabs[j] = malloc(alloc_size);
			cr_assert(slabs[j] != NULL);
			// for (int k = 0; k < alloc_size; k++) slabs[j][k] = j;
		}

		for (int j = 0; j < count; j++) {
			// for (int k = 0; k < alloc_size; k++) cr_assert(slabs[j][k] == j);
			free(slabs[j]);
		}
	}
}

MyTest(base, test_aux) {
	long long size = 1024 * 1024 * 10;
	int count = 4;
	int alloc_size = 8;

	SlabAllocator sa1;
	cr_assert(
		!slab_allocator_init(&sa1, alloc_size, count + 5, count + 5, false));

	Slab s1 = slab_allocator_allocate(&sa1);
	cr_assert(s1 != NULL);

	unsigned long long *ptr = slab_aux(s1);
	cr_assert(*ptr != 123);
	*ptr = 123;
	cr_assert(*ptr == 123);

	slab_allocator_free(&sa1, s1);

	Slab s2 = slab_allocator_allocate(&sa1);
	cr_assert(s2 != NULL);

	unsigned long long *ptr2 = slab_aux(s2);
	cr_assert(*ptr2 != 123);

	slab_allocator_free(&sa1, s2);
	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_memmap) {
	MemMap mm1;
	int size = 64;
	int count = 3000;
	memmap_init(&mm1, size);

	for (int i = 0; i < count; i++) {
		if (i % 100000 == 0) {
			// println("i=%i", i);
		}
		Ptr p = memmap_allocate(&mm1);
		if (p != i + 1) println("p=%u,i+1=%i", p, i + 1);
		cr_assert_eq(p, i + 1);
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			data[j] = 'a' + (j % 26);
		}
	}

	for (int i = 0; i < count; i++) {
		Ptr p = i + 1;
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			cr_assert_eq(data[j], 'a' + (j % 26));
		}
	}

	memmap_free(&mm1, 7);
	memmap_free(&mm1, 8);
	memmap_free(&mm1, 167);

	Ptr p = memmap_allocate(&mm1);
	cr_assert_eq(p, 7);
	p = memmap_allocate(&mm1);
	cr_assert_eq(p, 8);
	p = memmap_allocate(&mm1);
	cr_assert_eq(p, 167);

	p = memmap_allocate(&mm1);
	cr_assert_eq(p, count + 1);

	memmap_cleanup(&mm1);
}

MyTest(base, test_memmap_recycle) {
	MemMap mm1;
	int size = 64;
	int count = 3000000;
	int itt = 4;
	memmap_init(&mm1, size);
	Ptr ptrs[itt];

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < itt; j++) {
			ptrs[j] = memmap_allocate(&mm1);
			cr_assert(ptrs[j] == 1 + j);
		}

		for (int j = 0; j < itt; j++) {
			cr_assert(ptrs[j] == 1 + j);
			memmap_free(&mm1, ptrs[j]);
		}
	}
	memmap_cleanup(&mm1);
}
*/
