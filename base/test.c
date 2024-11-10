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
#include <crypto/cpsrng.h>

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

MyTest(base, test_slab_allocator_recycle) {
	long long size = 1024 * 1024 * 1;
	int count = 5;
	int alloc_size = 64;

	SlabAllocator sa1;
	cr_assert(!slab_allocator_init(&sa1, alloc_size, count + 5, count + 5));
	Ptr slabs[count];

	for (long long i = 0; i < size; i++) {
		for (int j = 0; j < count; j++) {
			slabs[j] = slab_allocator_allocate(&sa1);
			cr_assert(slabs[j] != null);
		}
		for (int j = 0; j < count; j++) {
			slab_allocator_free(&sa1, slabs[j]);
		}
	}

	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_malloc_recycle) {
	long long size = 1024 * 1024 * 1;
	int count = 5;
	int alloc_size = 64;

	byte *slabs[count];

	for (long long i = 0; i < size; i++) {
		for (int j = 0; j < count; j++) {
			slabs[j] = malloc(alloc_size);
			cr_assert(slabs[j] != NULL);
		}

		for (int j = 0; j < count; j++) {
			free(slabs[j]);
		}
	}
}

MyTest(base, test_slab_allocator_adv) {
	SlabAllocator sa1;
	int slab_len = 128;
	cr_assert(!slab_allocator_init(&sa1, slab_len, 100000, 2000000));
	int size = 50000;
	Ptr *arr = malloc(sizeof(Ptr) * size);

	for (int i = 0; i < size; i++) {
		Ptr p = slab_allocator_allocate(&sa1);
		arr[i] = p;
		byte *parr = slab_get(&sa1, p);
		parr[0] = (i + 2) % 256;
		for (int j = 1; j < slab_len; j++) {
			parr[j] = (((i + 2) + j) % 26) + 'a';
		}
	}

	for (int i = 0; i < size; i++) {
		byte *parr = slab_get(&sa1, arr[i]);
		cr_assert_eq(parr[0], (i + 2) % 256);
		for (int j = 1; j < slab_len; j++)
			cr_assert_eq(parr[j], (((i + 2) + j) % 26) + 'a');
		slab_allocator_free(&sa1, arr[i]);
	}

	slab_allocator_cleanup(&sa1);
	free(arr);
}

MyTest(base, test_util) {
	char *x = "1234567";
	char y[10];
	for (int i = 0; i < 10; i++) y[i] = '\0';
	copy_bytes(y, x, 3);
	cr_assert_eq(y[0], '1');
	cr_assert_eq(y[1], '2');
	cr_assert_eq(y[2], '3');
	cr_assert_eq(y[3], '\0');
	cr_assert_eq(y[4], '\0');
	cr_assert_eq(y[5], '\0');
	cr_assert_eq(y[6], '\0');
	cr_assert_eq(y[7], '\0');
	cr_assert_eq(y[8], '\0');
	cr_assert_eq(y[9], '\0');

	char z[10];
	for (int i = 0; i < 10; i++) z[i] = 'a' + i;
	set_bytes(z, 'z', 5);
	cr_assert_eq(z[0], 'z');
	cr_assert_eq(z[1], 'z');
	cr_assert_eq(z[2], 'z');
	cr_assert_eq(z[3], 'z');
	cr_assert_eq(z[4], 'z');
	cr_assert_eq(z[5], 'a' + 5);
	cr_assert_eq(z[6], 'a' + 6);
	cr_assert_eq(z[7], 'a' + 7);
	cr_assert_eq(z[8], 'a' + 8);
	cr_assert_eq(z[9], 'a' + 9);
}
