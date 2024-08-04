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

#include <core/std.h>
#include <criterion/criterion.h>

Test(core, slab_data) {
	SlabData sd;
	SlabDataParams p;
	Slab slab;
	slab.len = 10;
	p.slab_size = 10;
	p.slab_count = 10;
	p.max_slabs = 35;
	p.slabs_per_resize = 10;
	p.free_list_head = 0;
	cr_assert_eq(slab_data_build(&sd, p), 0);

	slab_data_access(&sd, &slab, 0);
	for (int i = 0; i < 10; i++)
		((char *)slab.data)[i] = i;
	slab_data_access(&sd, &slab, 20);
	for (int i = 0; i < 10; i++)
		((char *)slab.data)[i] = i + 10;

	slab_data_access(&sd, &slab, 0);
	for (int i = 0; i < 10; i++)
		cr_assert_eq(((char *)slab.data)[i], i);

	slab_data_access(&sd, &slab, 20);
	for (int i = 0; i < 10; i++)
		cr_assert_eq(((char *)slab.data)[i], i + 10);
}

Test(core, slab_allocator) {
	SlabAllocator sa;
	SlabDataParams sdp1, sdp2;
	sdp1.slab_size = 10;
	sdp1.slab_count = 10;
	sdp1.max_slabs = 30;
	sdp1.slabs_per_resize = 5;
	sdp1.free_list_head = 0;

	sdp2.slab_size = 20;
	sdp2.slab_count = 10;
	sdp2.max_slabs = 50;
	sdp2.slabs_per_resize = 10;
	sdp2.free_list_head = 0;

	slab_allocator_build(&sa, true, 2, sdp1, sdp2);

	u64 id = slab_allocator_allocate(&sa, 10);
	cr_assert_eq(id, 0);
	Slab slab;
	slab_allocator_get(&sa, &slab, 0);
	for (int i = 0; i < 10; i++)
		((char *)slab.data)[i] = i;

	id = slab_allocator_allocate(&sa, 10);
	cr_assert_eq(id, 1);
	slab_allocator_get(&sa, &slab, 1);
	for (int i = 0; i < 10; i++)
		((char *)slab.data)[i] = 10 + i;

	slab_allocator_get(&sa, &slab, 0);
	for (int i = 0; i < 10; i++)
		cr_assert_eq(((char *)slab.data)[i], i);

	slab_allocator_get(&sa, &slab, 1);
	for (int i = 0; i < 10; i++)
		cr_assert_eq(((char *)slab.data)[i], 10 + i);

	slab_allocator_free(&sa, 1);

	id = slab_allocator_allocate(&sa, 10);
	cr_assert_eq(id, 1);

	slab_allocator_get(&sa, &slab, 1);
	for (int i = 0; i < 10; i++) {
		cr_assert_eq(((char *)slab.data)[i], 0);
	}
}

Test(core, slab_allocator_resize) {
	SlabAllocator sa;
	SlabDataParams sdp1, sdp2;
	sdp1.slab_size = 10;
	sdp1.slab_count = 10;
	sdp1.max_slabs = 35;
	sdp1.slabs_per_resize = 5;
	sdp1.free_list_head = 0;

	sdp2.slab_size = 20;
	sdp2.slab_count = 10;
	sdp2.max_slabs = 50;
	sdp2.slabs_per_resize = 10;
	sdp2.free_list_head = 0;

	slab_allocator_build(&sa, false, 2, sdp2, sdp1);
	cr_assert_eq(sa.slab_data_arr[0].sdp.slab_size, 10);

	for (u64 i = 0; i < 35; i++) {
		u64 id = slab_allocator_allocate(&sa, 10);
		cr_assert_eq(id, i);
	}

	u64 id = slab_allocator_allocate(&sa, 10);
	cr_assert_eq(id, UINT64_MAX);

	for (u64 i = 0; i < 50; i++) {
		u64 id = slab_allocator_allocate(&sa, 20);
		cr_assert_eq(id, i | (u64)0x1 << 56);
	}

	id = slab_allocator_allocate(&sa, 20);
	cr_assert_eq(id, UINT64_MAX);
}

Test(core, slab_allocator_params) {
	SlabAllocator sa;
	slab_allocator_build(&sa, false, 2,
			     SLAB_PARAMS(SlabSize(10), SlabCount(20)),
			     SLAB_PARAMS(SlabSize(50), SlabCount(30)));

	SlabAllocator sa2 =
	    SLABS(true, SLAB_PARAMS(SlabSize(50), SlabCount(20)),
		  SLAB_PARAMS(SlabSize(10), SlabCount(30)));
	cr_assert_eq(sa2.slab_data_arr_size, 2);
	cr_assert_eq(sa2.slab_data_arr[0].sdp.slab_size, 10);
	cr_assert_eq(sa2.slab_data_arr[1].sdp.slab_size, 50);

	// check defaults
	SlabDataParams sdp1 = SLAB_PARAMS();
	cr_assert_eq(sdp1.slab_size, 512);
	cr_assert_eq(sdp1.max_slabs, 100);
	cr_assert_eq(sdp1.slabs_per_resize, 10);
	cr_assert_eq(sdp1.slab_count, 10);
	cr_assert_eq(sdp1.free_list_head, 0);

	SlabDataParams sdp2 = SLAB_PARAMS(SlabSize(256));
	cr_assert_eq(sdp2.slab_size, 256);
	cr_assert_eq(sdp2.max_slabs, 100);
	cr_assert_eq(sdp2.slabs_per_resize, 10);
	cr_assert_eq(sdp2.slab_count, 10);
	cr_assert_eq(sdp2.free_list_head, 0);

	SlabDataParams sdp3 = SLAB_PARAMS(SlabCount(300), MaxSlabs(400));
	cr_assert_eq(sdp3.slab_size, 512);
	cr_assert_eq(sdp3.max_slabs, 400);
	cr_assert_eq(sdp3.slabs_per_resize, 10);
	cr_assert_eq(sdp3.slab_count, 300);
	cr_assert_eq(sdp3.free_list_head, 0);

	SlabData sd;
	slab_data_build(&sd, sdp3);
}

Test(core, test_mymalloc) {
	ResourceStats init_stats = get_resource_stats();

	{

		// init slab allocator and define some slabs
		SlabAllocator sa;
		Slab slab1, slab2, slab3, slab4, slab5, slab6;
		sa = SLABS(false, SLAB_PARAMS(SlabSize(10), SlabCount(20)),
			   SLAB_PARAMS(SlabSize(50), SlabCount(30)),
			   SLAB_PARAMS(SlabSize(25), SlabCount(20)));

		// allocate several slabs
		cr_assert_eq(mymalloc(&sa, &slab1, 10), 0);
		cr_assert_eq(mymalloc(&sa, &slab2, 10), 0);
		cr_assert_eq(mymalloc(&sa, &slab3, 10), 0);

		// value slab2's id
		cr_assert_eq(slab2.id, 1);
		cr_assert_eq(slab2.len, 10);
		cr_assert_eq(myfree(&sa, &slab2), 0);
		cr_assert_eq(mymalloc(&sa, &slab4, 10), 0);
		cr_assert_eq(mymalloc(&sa, &slab5, 10), 0);
		cr_assert_eq(mymalloc(&sa, &slab6, 11), 0);

		// check ids
		cr_assert_eq(slab1.id, 0);
		cr_assert_eq(slab3.id, 2);
		cr_assert_eq(slab4.id, 1);
		cr_assert_eq(slab5.id, 3);
		cr_assert_eq(slab6.id, UINT64_MAX);

		// check len
		cr_assert_eq(slab1.len, 10);
		cr_assert_eq(slab3.len, 10);
		cr_assert_eq(slab4.len, 10);
		cr_assert_eq(slab5.len, 10);
		cr_assert_eq(slab6.len, 11);

		// ensure freeing slabs doesn't result in an error
		cr_assert_eq(myfree(&sa, &slab1), 0);
		cr_assert_eq(myfree(&sa, &slab3), 0);
		cr_assert_eq(myfree(&sa, &slab4), 0);
		cr_assert_eq(myfree(&sa, &slab5), 0);
		cr_assert_eq(myfree(&sa, &slab6), 0);

		// allocate other sizes
		cr_assert_eq(mymalloc(&sa, &slab1, 25), 0);
		// slab1 has index = 1 so shift 1 << 56
		cr_assert_eq(slab1.id, (u64)0x1 << 56);

		cr_assert_eq(mymalloc(&sa, &slab2, 50), 0);
		// slab2 has index = 2 so shift 2 << 56
		cr_assert_eq(slab2.id, (u64)0x2 << 56);

		cr_assert_eq(myfree(&sa, &slab1), 0);
		cr_assert_eq(myfree(&sa, &slab2), 0);
	}

	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_realloc) {
	ResourceStats init_stats = get_resource_stats();

	{
		SlabAllocator sa;
		Slab slab1;
		sa = SLABS(false, SLAB_PARAMS(SlabSize(10), SlabCount(20)),
			   SLAB_PARAMS(SlabSize(50), SlabCount(30)),
			   SLAB_PARAMS(SlabSize(25), SlabCount(20)));

		cr_assert_eq(mymalloc(&sa, &slab1, 10), 0);
		((char *)slab1.data)[0] = 1;
		((char *)slab1.data)[1] = 2;
		((char *)slab1.data)[2] = 3;
		cr_assert_eq(slab1.len, 10);
		cr_assert_eq(myrealloc(&sa, &slab1, 25), 0);
		cr_assert_eq(((char *)slab1.data)[0], 1);
		cr_assert_eq(((char *)slab1.data)[1], 2);
		cr_assert_eq(((char *)slab1.data)[2], 3);
		((char *)slab1.data)[0] = 4;
		((char *)slab1.data)[1] = 5;
		((char *)slab1.data)[2] = 6;
		cr_assert_eq(slab1.len, 25);
		cr_assert_eq(myrealloc(&sa, &slab1, 50), 0);
		cr_assert_eq(((char *)slab1.data)[0], 4);
		cr_assert_eq(((char *)slab1.data)[1], 5);
		cr_assert_eq(((char *)slab1.data)[2], 6);
		cr_assert_eq(slab1.len, 50);
		cr_assert_eq(myrealloc(&sa, &slab1, 60), 0);
		cr_assert_eq(((char *)slab1.data)[0], 4);
		cr_assert_eq(((char *)slab1.data)[1], 5);
		cr_assert_eq(((char *)slab1.data)[2], 6);
		cr_assert_eq(slab1.len, 60);

		cr_assert_eq(myfree(&sa, &slab1), 0);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}
