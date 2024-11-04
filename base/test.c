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

MyTest(base, test_ptr) {
	Ptr ptr = ptr_direct_alloc(100);
	cr_assert_eq(ptr_len(ptr), slab_getpagesize() - slab_overhead());
	char *test = ptr_data(ptr);
	for (int i = 0; i < ptr_len(ptr); i++) {
		test[i] = i % 128;
	}
	for (int i = 0; i < ptr_len(ptr); i++) {
		cr_assert_eq(test[i], i % 128);
	}
	cr_assert(!nil(ptr));
	cr_assert(ok(ptr));
	ptr_direct_release(ptr);
	ptr = NULL;
	cr_assert(nil(ptr));
	cr_assert(!ok(ptr));

	Ptr next = ptr_direct_alloc(1000);
	cr_assert_eq(ptr_len(next), slab_getpagesize() - slab_overhead());
	unsigned int initial_size = ptr_len(next);

	char *buf = ptr_data(next);
	for (int i = 0; i < 1000; i++) buf[i] = 'a' + (i % 26);

	Ptr nptr = ptr_direct_resize(next, 100000);
	cr_assert(nptr);
	cr_assert(ptr_len(nptr) > initial_size);
	int max_len = ptr_len(nptr);
	char *buf2 = ptr_data(nptr);
	for (int i = 0; i < 1000; i++) cr_assert_eq(buf2[i], 'a' + (i % 26));

	Ptr nptr2 = ptr_direct_resize(nptr, 500);
	cr_assert(nptr2);
	cr_assert(ptr_len(nptr2) >= 500);
	cr_assert(ptr_len(nptr2) < max_len);
	char *buf3 = ptr_data(nptr2);
	for (int i = 0; i < 500; i++) cr_assert_eq(buf3[i], 'a' + (i % 26));

	ptr_direct_release(nptr2);
}

MyTest(base, test_ptr_macros) {
	Ptr ptr = ptr_direct_alloc(100);
	cr_assert(ok(ptr));
	cr_assert(!nil(ptr));
	cr_assert_eq($len(ptr), slab_getpagesize() - slab_overhead());
	char *test = $(ptr);
	for (int i = 0; i < $len(ptr); i++) {
		test[i] = i % 128;
	}
	for (int i = 0; i < $len(ptr); i++) {
		cr_assert_eq(test[i], i % 128);
	}
	cr_assert(!nil(ptr));
	cr_assert(ok(ptr));
	ptr_direct_release(ptr);
	ptr = NULL;
	cr_assert(nil(ptr));
	cr_assert(!ok(ptr));

	Ptr next = ptr_direct_alloc(1000);
	cr_assert_eq($len(next), slab_getpagesize() - slab_overhead());
	unsigned int initial_size = $len(next);

	char *buf = $(next);
	for (int i = 0; i < 1000; i++) buf[i] = 'a' + (i % 26);

	Ptr nptr = ptr_direct_resize(next, 100000);
	cr_assert(nptr);
	cr_assert($len(nptr) > initial_size);
	int max_len = $len(nptr);
	char *buf2 = $(nptr);
	for (int i = 0; i < 1000; i++) cr_assert_eq(buf2[i], 'a' + (i % 26));

	Ptr nptr2 = ptr_direct_resize(nptr, 500);
	cr_assert(nptr2);
	cr_assert($len(nptr2) >= 500);
	cr_assert($len(nptr2) < max_len);
	char *buf3 = $(nptr2);
	for (int i = 0; i < 500; i++) cr_assert_eq(buf3[i], 'a' + (i % 26));

	cr_assert(initialized(nptr2));
	ptr_direct_release(nptr2);

	Ptr test_null = NULL;
	cr_assert(!initialized(test_null));
	cr_assert(nil(test_null));
	cr_assert(!ok(test_null));
}

MyTest(base, test_limits) {
	cr_assert_eq(INT64_MAX, 9223372036854775807LL);
	cr_assert_eq(INT64_MAX_IMPL, 9223372036854775807LL);
	cr_assert_eq(INT_MAX, 2147483647);
	cr_assert_eq(INT_MAX_IMPL, 2147483647);
	cr_assert_eq(BYTE_MAX, 255);
	cr_assert_eq(BYTE_MAX_IMPL, 255);
	cr_assert_eq(INT_MAX, 2147483647);
	cr_assert_eq(INT_MIN, -2147483648);
	cr_assert_eq(INT_MIN_IMPL, -2147483648);
	cr_assert_eq(UINT32_MAX, 4294967295);
	cr_assert_eq(UINT32_MAX_IMPL, 4294967295);
}

MyTest(base, test_slab_allocator) {
	{
		SlabAllocator sa = slab_allocator_create(true);
		Ptr ptr = slab_allocator_allocate(sa, 100);
		cr_assert($len(ptr) >= 100);
		byte *arr = $(ptr);
		for (int i = 0; i < 100; i++) {
			arr[i] = i;
		}
		for (int i = 0; i < 100; i++) {
			cr_assert_eq(arr[i], i);
		}
		cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 1);
		slab_allocator_free(sa, ptr);
		cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);
	}
	cr_assert(alloc_sum() > 0);

	{
		SlabAllocator sa = slab_allocator_create(true);
		for (int i = 0; i < 1000; i++) {
			Ptr ptr = slab_allocator_allocate(sa, i);
			cr_assert($len(ptr) >= i);
			byte *arr = $(ptr);
			for (int j = 0; j < 100; j++) {
				arr[j] = j;
			}
			cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 1);
			slab_allocator_free(sa, ptr);
			cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);
		}
	}
	cr_assert(alloc_sum() > 0);
}

MyTest(base, test_big) {
	SlabAllocator sa = slab_allocator_create(false);
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);
	// unsigned int size = ((1024LL * 1024LL * 1024LL * 4) - 128);
	unsigned int size = (10LL * 1024LL * 1LL);
	int ss = 16;
	Ptr *arr = malloc(sizeof(Ptr) * (long long)size);
	for (unsigned int i = 0; i < size; i++) {
		if (i % 10000 == 0) println("i=%u", i);
		arr[i] = slab_allocator_allocate(sa, ss);
		if (nil(arr[i])) printf("nil ptr at %u\n", i);
		cr_assert(!nil(arr[i]));
		cr_assert_eq(ptr_id(arr[i]), i);
		cr_assert_eq($len(arr[i]), ss);
		byte *data = $(arr[i]);
		for (int j = 0; j < ss; j++) data[j] = (j + i * 3) % 256;
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), size);

	for (unsigned int i = 0; i < size; i++) {
		if (i % 10000 == 0) println("check i=%u", i);
		if (nil(arr[i])) printf("nil ptr at %u\n", i);
		cr_assert(!nil(arr[i]));
		cr_assert_eq(ptr_id(arr[i]), i);
		cr_assert_eq($len(arr[i]), ss);
		byte *data = $(arr[i]);
		for (int j = 0; j < ss; j++) data[j] = (j + i * 3) % 256;
	}

	for (unsigned int i = 0; i < size; i++) {
		if (i % 10000 == 0) println("free i=%u", i);
		cr_assert_eq(ptr_id(arr[i]), i);
		cr_assert_eq($len(arr[i]), ss);
		byte *data = $(arr[i]);

		for (int j = 0; j < ss; j++) cr_assert_eq(data[j], (j + i * 3) % 256);

		slab_allocator_free(sa, arr[i]);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);

	Ptr p = slab_allocator_allocate(sa, ss);
	cr_assert_eq(ptr_id(p), size - 1);
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 1);
	slab_allocator_free(sa, p);
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);

	/////	release(alloc_res);
	free(arr);
}

MyTest(base, test_slab_sizes) {
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

MyTest(base, test_fam_alloc) {
	Ptr test = NULL;
	cr_assert(nil(test));
	cr_assert(!initialized(test));
	test = $alloc(100);
	cr_assert(!nil(test));
	cr_assert(ok(test));
	$release(test);
	cr_assert(nil(test));
	cr_assert(!ok(test));

	Ptr resize = NULL;
	cr_assert(nil(resize));
	resize = $alloc(200);
	cr_assert($len(resize) >= 200);
	int initial_size = $len(resize);
	Ptr resize2 = $resize(resize, 300);
	cr_assert($len(resize2) > initial_size);

	Ptr resize3 = $resize(resize2, 100000);
	cr_assert($len(resize3) > 100000);
	for (int i = 0; i < 100000; i++) ((byte *)$(resize3))[i] = 'a' + (i % 26);

	for (int i = 0; i < 100000; i++)
		cr_assert_eq(((byte *)$(resize3))[i], 'a' + (i % 26));

	Ptr resize4 = $resize(resize3, 200000);
	for (int i = 0; i < 100000; i++)
		cr_assert_eq(((byte *)$(resize4))[i], 'a' + (i % 26));

	Ptr resize5 = $resize(resize4, 500);
	for (int i = 0; i < 500; i++)
		cr_assert_eq(((byte *)$(resize5))[i], 'a' + (i % 26));

	$release(resize5);
}

MyTest(base, test_queue) {
	/*
		Queue q = queue_create();

		Ptr v1 = $alloc(sizeof(int64));
		println("v1.id=%u, %u\n", ptr_id(v1), ptr_len(v1));
		*(int64 *)$(v1) = 3;

		Ptr v2 = $alloc(sizeof(int64));
		println("v2.id=%u,%u\n", ptr_id(v2), ptr_len(v2));
		println("1");
		*(int64 *)$(v2) = 4;
		println("2");

		println("3");
		queue_enqueue(q, v1);
		println("4");
		// queue_enqueue(q, v2);

		$release(v1);
		$release(v2);
	*/
}
