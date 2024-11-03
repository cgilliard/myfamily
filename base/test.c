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
// #include <criterion/criterion.h>
#include <base/test.h>
#include <stdio.h>

MySuite(base);

void test_spawn() {
	printf("test spawn\n");
}

void test_spawn2() {
	printf("test spawn2\n");
}

void test_main() {
	printf("test_main\n");

	Topic t = spawn(test_spawn);
	Topic t2 = spawn(test_spawn2);

	printf("ret from spawn\n");
}

MyTest(base, test_base) {
	init(test_main);
}

MyTest(base, test_alloc) {
	Alloc test1 = alloc(100);
	cr_assert_eq(test1.size, slabs_page_size);
	release(test1);
}

MyTest(base, test_ptr) {
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
	ptr_direct_release(ptr);
	ptr = NULL;
	cr_assert(nil(ptr));
	cr_assert(!ok(ptr));

	Ptr next = ptr_direct_alloc(1000);
	cr_assert_eq(ptr_len(next), slabs_page_size - slab_overhead());
	unsigned int initial_size = ptr_len(next);

	char *buf = ptr_data(next);
	for (int i = 0; i < 1000; i++) buf[i] = 'a' + (i % 26);

	Ptr nptr = ptr_direct_resize(next, 100000);
	cr_assert(nptr);
	cr_assert(ptr_len(nptr) > initial_size);
	char *buf2 = ptr_data(nptr);
	for (int i = 0; i < 1000; i++) cr_assert_eq(buf2[i], 'a' + (i % 26));

	ptr_direct_release(nptr);
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

MyTest(base, test_slab_allocator) {
	{
		SlabAllocator sa = slab_allocator_create();
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
		SlabAllocator sa = slab_allocator_create();
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

MyTest(base, test_fam_alloc) {
	Ptr ptr1 = $alloc(10);
	cr_assert($len(ptr1) >= 10);
	for (int i = 0; i < $len(ptr1); i++) {
		((byte *)$(ptr1))[i] = i;
	}
	Ptr ptr2 = fam_ptr_for(ptr_id(ptr1), ptr_len(ptr1));
	cr_assert(!nil(ptr2));
	for (int i = 0; i < $len(ptr2); i++) {
		cr_assert_eq(((byte *)$(ptr1))[i], i);
	}

	cr_assert_eq($len(ptr2), $len(ptr1));
	cr_assert(!nil(ptr1));
	cr_assert(!nil(ptr2));
	cr_assert(ok(ptr1));
	cr_assert(ok(ptr2));
	$release(ptr1);
	cr_assert(nil(ptr1));
	cr_assert(!ok(ptr1));

	Ptr ptr3 = $alloc(100);
	int old_size = $len(ptr3);
	cr_assert(old_size >= 100);
	Ptr ptr4 = $resize(ptr3, 200);
	cr_assert(!nil(ptr4));
	cr_assert(nil(ptr3));
	cr_assert($len(ptr4) > old_size);
	$release(ptr4);

	Ptr direct1 = $alloc(100000);
	for (int i = 0; i < 100000; i++) {
		byte *arr = $(direct1);
		arr[i] = i % 100;
	}

	Ptr direct2 = $resize(direct1, 200000);
	for (int i = 0; i < 100000; i++) {
		byte *arr = $(direct2);
		cr_assert_eq(arr[i], i % 100);
	}

	cr_assert(nil(direct1));

	Ptr direct3 = $alloc(100);
	for (int i = 0; i < 100; i++) {
		byte *arr = $(direct3);
		arr[i] = i % 100;
	}

	Ptr direct4 = $resize(direct3, 200);
	for (int i = 0; i < 100; i++) {
		byte *arr = $(direct4);
		cr_assert_eq(arr[i], i % 100);
	}
	cr_assert(nil(direct3));

	$release(direct2);
	$release(direct4);
}

MyTest(base, test_queue) {
	Queue queue = queue_create();
}

MyTest(base, test_lock) {
	Lock l1 = lock_create();
	cr_assert_eq(lock_get_state(l1), 0);
	lock_read(l1);
	unsigned long long state = lock_get_state(l1);
	// upper bits incremented by 1.
	cr_assert_eq(state >> 32, 1);
	// lower bits also incremeneted by 1.
	cr_assert_eq(state & 0xFFFFFFFF, 1);

	lock_unlock(l1);
	state = lock_get_state(l1);
	// upper bits incremented by 1.
	cr_assert_eq(state >> 32, 2);
	// lower bits also decremented by 1.
	cr_assert_eq(state & 0xFFFFFFFF, 0);

	lock_write(l1);
	state = lock_get_state(l1);
	// upper bits incremented by 1.
	cr_assert_eq(state >> 32, 3);
	// lower bits high bit set
	cr_assert_eq(state & 0xFFFFFFFF, 0x80000000);

	lock_unlock(l1);

	state = lock_get_state(l1);
	// upper bits incremented by 1.
	cr_assert_eq(state >> 32, 4);
	// lower bits are set to 0 now
	cr_assert_eq(state & 0xFFFFFFFF, 0x00000000);
}

Lock l1;
volatile int state = 0;
volatile int state2 = 0;

static void *test_thread_start(void *arg) {
	while (true) {
		lockw(l1);
		if (state != 0) {
			state2 += 1;
			unlock(l1);
			break;
		}
		unlock(l1);
	}
	return NULL;
}

MyTest(base, test_lock2) {
	l1 = lock();
	pthread_t cThread;
	cr_assert(!pthread_create(&cThread, NULL, test_thread_start, NULL));
	sleep(1);

	lockw(l1);
	state = 1;
	unlock(l1);

	pthread_join(cThread, NULL);
	cr_assert_eq(state2, 1);
	Lock_cleanup(&l1);
}

MyTest(base, test_rsync_wsync) {
	Lock l1 = lock();
	int state = 1;
	cr_assert_eq(state, 1);

	for (int i = 0; i < 10; i++) {
		rsync(l1, { state = 2; });
		cr_assert_eq(state, 2);

		wsync(l1, { state = 3; });
		cr_assert_eq(state, 3);
	}
	cr_assert_eq(state, 3);
}
