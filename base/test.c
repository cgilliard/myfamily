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
	cr_assert(!slab_allocator_init(&sa1, 16));
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

	/*
		slab_allocator_free(&sa1, s1);

		Slab s3 = slab_allocator_allocate(&sa1);
		println("second alloc");
		Slab s4 = slab_allocator_allocate(&sa1);
		println("second alloc end");
		slab_allocator_free(&sa1, s3);

		Slab s5 = slab_allocator_allocate(&sa1);
		// slab_allocator_free(&sa1, s4);
	*/
}
