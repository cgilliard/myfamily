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
#include <pthread.h>
#include <unistd.h>

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
		SLAB_ALLOCATOR(&sa);
		{

			// allocate several slabs
			cr_assert_eq(mymalloc(&slab1, 10), 0);
			cr_assert_eq(mymalloc(&slab2, 10), 0);
			cr_assert_eq(mymalloc(&slab3, 10), 0);

			// value slab2's id
			cr_assert_eq(slab2.id, 1);
			cr_assert_eq(slab2.len, 10);
			cr_assert_eq(myfree(&slab2), 0);
			cr_assert_eq(mymalloc(&slab4, 10), 0);
			cr_assert_eq(mymalloc(&slab5, 10), 0);
			cr_assert_eq(mymalloc(&slab6, 11), 0);

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
			cr_assert_eq(myfree(&slab1), 0);
			cr_assert_eq(myfree(&slab3), 0);
			cr_assert_eq(myfree(&slab4), 0);
			cr_assert_eq(myfree(&slab5), 0);
			cr_assert_eq(myfree(&slab6), 0);

			// allocate other sizes
			cr_assert_eq(mymalloc(&slab1, 25), 0);
			// slab1 has index = 1 so shift 1 << 56
			cr_assert_eq(slab1.id, (u64)0x1 << 56);

			cr_assert_eq(mymalloc(&slab2, 50), 0);
			// slab2 has index = 2 so shift 2 << 56
			cr_assert_eq(slab2.id, (u64)0x2 << 56);

			cr_assert_eq(myfree(&slab1), 0);
			cr_assert_eq(myfree(&slab2), 0);
		}
		UNSET_SLAB_ALLOCATOR();
	}

	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_realloc) {
	ResourceStats init_stats = get_resource_stats();

	{
		Slab slab1;
		cr_assert_eq(mymalloc(&slab1, 10), 0);
		// ensure it's the global slab allocator and not the one we
		// define below
		cr_assert_neq(slab1.id, 0);
		cr_assert_eq(myfree(&slab1), 0);

		SlabAllocator sa =
		    SLABS(false, SLAB_PARAMS(SlabSize(10), SlabCount(20)),
			  SLAB_PARAMS(SlabSize(50), SlabCount(30)),
			  SLAB_PARAMS(SlabSize(25), SlabCount(20)));
		SLAB_ALLOCATOR(&sa);
		{

			cr_assert_eq(mymalloc(&slab1, 10), 0);
			((char *)slab1.data)[0] = 1;
			((char *)slab1.data)[1] = 2;
			((char *)slab1.data)[2] = 3;
			cr_assert_eq(slab1.len, 10);
			cr_assert_eq(myrealloc(&slab1, 25), 0);
			cr_assert_eq(((char *)slab1.data)[0], 1);
			cr_assert_eq(((char *)slab1.data)[1], 2);
			cr_assert_eq(((char *)slab1.data)[2], 3);
			((char *)slab1.data)[0] = 4;
			((char *)slab1.data)[1] = 5;
			((char *)slab1.data)[2] = 6;
			cr_assert_eq(slab1.len, 25);
			cr_assert_eq(myrealloc(&slab1, 50), 0);
			cr_assert_eq(((char *)slab1.data)[0], 4);
			cr_assert_eq(((char *)slab1.data)[1], 5);
			cr_assert_eq(((char *)slab1.data)[2], 6);
			cr_assert_eq(slab1.len, 50);
			cr_assert_eq(myrealloc(&slab1, 60), 0);
			cr_assert_eq(((char *)slab1.data)[0], 4);
			cr_assert_eq(((char *)slab1.data)[1], 5);
			cr_assert_eq(((char *)slab1.data)[2], 6);
			cr_assert_eq(slab1.len, 60);

			cr_assert_eq(myfree(&slab1), 0);
		}
		UNSET_SLAB_ALLOCATOR();
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_0size_initial_sa) {
	ResourceStats init_stats = get_resource_stats();

	{
		SlabAllocator sa = SLABS(
		    false, SLAB_PARAMS(SlabSize(1), SlabCount(0), MaxSlabs(100),
				       SlabsPerResize(10)));
		SLAB_ALLOCATOR(&sa);

		{

			Slab slab1;
			slab1.data = 0;
			cr_assert_eq(mymalloc(&slab1, 1), 0);
			cr_assert_eq(slab1.id, 0);
			cr_assert_eq(slab1.len, 1);
			cr_assert_neq(slab1.data, 0);

			myfree(&slab1);
		}
		UNSET_SLAB_ALLOCATOR();
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_nested_slabs) {
	ResourceStats init_stats = get_resource_stats();

	{
		SlabAllocator sa1 = SLABS(
		    false, SLAB_PARAMS(SlabSize(10), SlabCount(10),
				       MaxSlabs(100), SlabsPerResize(10)));
		SLAB_ALLOCATOR(&sa1);
		{

			Slab slab1;
			cr_assert_eq(mymalloc(&slab1, 10), 0);
			cr_assert_eq(slab1.id, 0);
			cr_assert_eq(slab1.len, 10);
			myfree(&slab1);

			{

				SlabAllocator sa2 =
				    SLABS(false, SLAB_PARAMS(SlabSize(20),
							     SlabCount(10),
							     MaxSlabs(100)));
				SLAB_ALLOCATOR(&sa2);
				{
					Slab slab2;
					slab2.data = 0;
					cr_assert_eq(mymalloc(&slab2, 20), 0);
					cr_assert_eq(slab2.id, 0);
					cr_assert_eq(slab2.len, 20);
					myfree(&slab2);

					cr_assert_eq(mymalloc(&slab2, 10), 0);
					cr_assert_neq(slab2.id, 0);
					cr_assert_eq(slab2.len, 10);
					myfree(&slab2);
				}

				UNSET_SLAB_ALLOCATOR();
			}

			cr_assert_eq(mymalloc(&slab1, 10), 0);
			cr_assert_eq(slab1.id, 0);
			cr_assert_eq(slab1.len, 10);
			myfree(&slab1);

			cr_assert_eq(mymalloc(&slab1, 20), 0);
			cr_assert_neq(slab1.id, 0);
			cr_assert_eq(slab1.len, 20);
			myfree(&slab1);
		}

		UNSET_SLAB_ALLOCATOR();
	}

	Slab slab;
	slab.data = 0;
	cr_assert_eq(mymalloc(&slab, 1), 0);
	cr_assert_eq(slab.id, 0);
	cr_assert_eq(slab.len, 1);
	myfree(&slab);

	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

CLASS(MyClass, FIELD(u64, val))
#define MyClass DEFINE_CLASS(MyClass)

void MyClass_cleanup(MyClass *ptr) {}

GETTER(MyClass, val)

Test(core, test_class) {
	ResourceStats rs_start = get_resource_stats();
	MyClass x = BUILD(MyClass, 1);
	u64 val = GET(MyClass, &x, val);
	cr_assert_eq(val, 1);
	ResourceStats rs = get_resource_stats();
	cr_assert_eq(rs.slab_misses, rs_start.slab_misses);
}

void *myThreadFun(void *vargp) {
	ResourceStats init_stats = get_resource_stats();

	{
		Slab slab;
		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_eq(slab.id, 0);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);

		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_eq(slab.id, 0);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);

		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_eq(slab.id, 0);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);

		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_eq(slab.id, 0);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
	// deallocate the slab allocator for this thread for testing purposes
	slab_allocator_cleanup(&TL_SLAB_ALLOCATOR);
	return NULL;
}

Test(core, test_threads) {
	ResourceStats init_stats = get_resource_stats();
	{
		Slab slab;
		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.id, 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_neq(slab.data, 0);

		pthread_t thread_id;
		pthread_create(&thread_id, NULL, myThreadFun, NULL);
		pthread_join(thread_id, NULL);

		cr_assert_eq(myfree(&slab), 0);

		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.id, 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);

		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.id, 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);

		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.id, 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);

		slab.data = 0;
		cr_assert_eq(mymalloc(&slab, 1), 0);
		cr_assert_eq(slab.id, 0);
		cr_assert_eq(slab.len, 1);
		cr_assert_neq(slab.data, 0);
		cr_assert_eq(myfree(&slab), 0);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_next_greater_slab_size) {
	cr_assert_eq(next_greater_slab_size(1), 1);
	cr_assert_eq(next_greater_slab_size(3), 3);
	cr_assert_eq(next_greater_slab_size(100000), 100000);
	cr_assert_eq(next_greater_slab_size(191), 192);
	cr_assert_eq(next_greater_slab_size(192), 192);
	cr_assert_eq(next_greater_slab_size(193), 200);
	cr_assert_eq(next_greater_slab_size(199), 200);
	cr_assert_eq(next_greater_slab_size(200), 200);
	cr_assert_eq(next_greater_slab_size(201), 208);

	for (u64 i = 1; i < 50000; i++) {
		// ensure no infinite loops
		next_greater_slab_size(i);
	}
}

Test(core, test_rc) {
	ResourceStats init_stats = get_resource_stats();
	{
		Slab slab1;
		mymalloc(&slab1, sizeof(MyClass));
		MyClassPtr *ptr = slab1.data;
		BUILDPTR(ptr, MyClass);
		Rc rc1 = RC(slab1);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_rc_advanced) {
	ResourceStats init_stats = get_resource_stats();
	{
		Slab slab1;
		mymalloc(&slab1, sizeof(MyClass));
		MyClassPtr *ptr = slab1.data;
		BUILDPTR(ptr, MyClass);
		Rc rc1 = RC(slab1);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_prim) {
	u64 xu = 1234;
	U64 xv = BUILD(U64, xu);
	u64 xu_out = *(u64 *)unwrap(&xv);
	cr_assert_eq(xu_out, xu);
	Unit x;
	copy(&x, &UNIT);
	cr_assert(equal(&x, &UNIT));
}

Test(core, test_backtrace_entry) {
	BacktraceEntry e1 =
	    BUILD(BacktraceEntry, "test1", "test2", "test3", "test4");

	cr_assert(!strcmp(e1._name, "test1"));

	Backtrace bt = INIT_BACKTRACE;
	BACKTRACE(&bt);
	print(&bt);
}

CLASS(MyClass2, FIELD(Slab, slab))
#define MyClass2 DEFINE_CLASS(MyClass2)

void MyClass2_cleanup(MyClass2 *ptr) {}

Test(core, test_error) {
	ResourceStats init_stats = get_resource_stats();
	{
		Error err = ERR(ILLEGAL_ARGUMENT, "illegal arg found");
		// Result r1 = ErrP(err);
		// print(&err);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

/*
Result gen_result() {
	u64 x = 1234;
	// return Ok(x);
}
*/

CLASS(TestClassHeapify, FIELD(u64, v) FIELD(Slab, s))
#define TestClassHeapify DEFINE_CLASS(TestClassHeapify)
GETTER(TestClassHeapify, s)

static int cleanup_count = 0;

void TestClassHeapify_cleanup(TestClassHeapify *ptr) {
	cleanup_count++;
	Slab s = GET(TestClassHeapify, ptr, s);
	myfree(&s);
}

Test(core, test_heapify) {
	ResourceStats init_stats = get_resource_stats();
	{
		u64 x1 = 10;
		Rc x = HEAPIFY(x1);
		u64 x1_out = *(u64 *)unwrap(&x);
		cr_assert_eq(x1_out, x1);

		i32 x2 = -1;
		Rc rc2 = HEAPIFY(x2);
		i32 x2_out = *(i32 *)unwrap(&rc2);
		cr_assert_eq(x2_out, x2);

		Slab slab;
		mymalloc(&slab, 100);
		TestClassHeapify x3 = BUILD(TestClassHeapify, 555, slab);
		Rc rc3 = HEAPIFY(x3);
		TestClassHeapify x3_out = *(TestClassHeapify *)unwrap(&rc3);
		cr_assert_eq(x3_out._v, 555);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_rc_clone) {
	ResourceStats init_stats = get_resource_stats();
	{
		cleanup_count = 0;
		Slab slab;
		mymalloc(&slab, 100);
		TestClassHeapify x3 = BUILD(TestClassHeapify, 555, slab);
		Rc rc3 = HEAPIFY(x3);
		Rc rc4;
		myclone(&rc4, &rc3);
		TestClassHeapify x3_out = *(TestClassHeapify *)unwrap(&rc3);
		cr_assert_eq(x3_out._v, 555);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
	cr_assert_eq(cleanup_count, 1);
}

CLASS(MyClass3, FIELD(Slab, s) FIELD(u64, vv));
#define MyClass3 DEFINE_CLASS(MyClass3)

void MyClass3_cleanup(MyClass3 *ptr) { myfree(&ptr->_s); }

ENUM(MyEnum, VARIANTS(VV01, VV02, VV03), TYPES("u64", "u32", "MyClass3"))
#define MyEnum DEFINE_ENUM(MyEnum)

ENUM(MyEnum2, VARIANTS(V201, V202, V203, V204, V205, V206, V207),
     TYPES("i8", "i16", "i32", "i64", "i128", "bool", "f64"))
#define MyEnum2 DEFINE_ENUM(MyEnum2)

Test(core, test_enum) {
	ResourceStats init_stats = get_resource_stats();
	{
		u64 x1 = 10;
		MyEnum e1 = BUILD_ENUM(MyEnum, VV01, x1);
		u64 x1_out = ENUM_VALUE(x1_out, u64, e1);
		cr_assert_eq(x1_out, x1);

		Slab slab;
		mymalloc(&slab, 100);
		MyClass3 x2 = BUILD(MyClass3, slab);
		MyEnum e2 = BUILD_ENUM(MyEnum, VV03, x2);
		MyClass3 x2_out = ENUM_VALUE(x2_out, MyClass3, e2);

		u32 x3 = 20;
		MyEnum e3 = BUILD_ENUM(MyEnum, VV02, x3);
		u32 x3_out = ENUM_VALUE(x3_out, u32, e3);
		cr_assert_eq(x3_out, x3);

		i8 x4 = 9;
		MyEnum2 e4 = BUILD_ENUM(MyEnum2, V201, x4);
		i8 x4_out = ENUM_VALUE(x4_out, i8, e4);
		cr_assert_eq(x4_out, x4);

		bool x5 = false;
		MyEnum2 e5 = BUILD_ENUM(MyEnum2, V206, x5);
		bool x5_out = ENUM_VALUE(x5_out, bool, e5);
		cr_assert_eq(x5_out, x5);

		bool x6 = true;
		MyEnum2 e6 = BUILD_ENUM(MyEnum2, V206, x6);
		bool x6_out = ENUM_VALUE(x6_out, bool, e6);
		cr_assert_eq(x6_out, x6);

		f64 x7 = 1.234;
		MyEnum2 e7 = BUILD_ENUM(MyEnum2, V207, x7);
		f64 x7_out = ENUM_VALUE(x7_out, f64, e7);
		cr_assert_eq(x7_out, x7);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_local_slab_allocator) {
	u64 x = 1;
	loop {
		Slab slab1;
		cr_assert_eq(mymalloc(&slab1, x), 0);
		myfree(&slab1);
		x += 10;
		if (x > 20000)
			break;
	}
}

Test(core, test_slab_no_malloc) {
	ResourceStats init_stats = get_resource_stats();

	{

		// init slab allocator and define some slabs
		SlabAllocator sa;
		Slab slab1, slab2, slab3, slab4, slab5, slab6;
		sa = SLABS(
		    false,
		    SLAB_PARAMS(SlabSize(10), SlabCount(10), MaxSlabs(10)),
		    SLAB_PARAMS(SlabSize(50), SlabCount(30)),
		    SLAB_PARAMS(SlabSize(25), SlabCount(20)));
		sa.no_malloc = true;
		SLAB_ALLOCATOR(&sa);
		{

			cr_assert_eq(mymalloc(&slab1, 10), 0);
			myfree(&slab1);
			cr_assert_neq(mymalloc(&slab1, 11), 0);
			Slab arr[10];

			for (int i = 0; i < 10; i++) {
				cr_assert_eq(mymalloc(&arr[i], 10), 0);
			}

			cr_assert_neq(mymalloc(&slab1, 10), 0);

			for (int i = 0; i < 10; i++) {
				cr_assert_eq(myfree(&arr[i]), 0);
			}
		}
		UNSET_SLAB_ALLOCATOR();
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_enum_oom) {
	ResourceStats init_stats = get_resource_stats();
	{

		{
			// first use default slab allocator which will succeed
			u64 y1 = 20;
			MyEnum e0 = BUILD_ENUM(MyEnum, VV01, y1);
			cr_assert_neq(e0.slab.data, NULL);
			u64 y1_out = ENUM_VALUE(y1_out, u64, e0);
			cr_assert_eq(y1_out, y1);
		}

		{
			// init slab allocator and define with no slabs and
			// no_malloc
			SlabAllocator sa;
			Slab slab1, slab2, slab3, slab4, slab5, slab6;
			sa = SLABS(false);
			sa.no_malloc = true;
			SLAB_ALLOCATOR(&sa);
			{

				u64 x1 = 10;
				MyEnum e1 = BUILD_ENUM(MyEnum, VV01, x1);
				cr_assert_eq(e1.slab.data, NULL);

				Slab slab;
				mymalloc(&slab, 100);
				MyClass3 x2 = BUILD(MyClass3, slab);
				MyEnum e2 = BUILD_ENUM(MyEnum, VV03, x2);
				cr_assert_eq(e2.slab.data, NULL);
			}

			UNSET_SLAB_ALLOCATOR();
		}
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}

Test(core, test_result) {
	ResourceStats init_stats = get_resource_stats();
	{
		u64 x1 = 1234;
		Result r1 = Ok(x1);
		u64 x1_out = EXPECT(r1, x1_out);
		cr_assert_eq(x1_out, 1234);
		cr_assert_eq(x1, 1234);

		i128 x2 = -123;
		Result r2 = Ok(x2);
		i128 x2_out = EXPECT(r2, x2_out);
		cr_assert_eq(x2_out, -123);
		cr_assert_eq(x2, -123);

		Slab slab;
		mymalloc(&slab, 100);
		MyClass3 x3 = BUILD(MyClass3, slab, 101);
		cr_assert_eq(x3._vv, 101);

		Result r3 = Ok(x3);
		MyClass3 x3_out = EXPECT(r3, x3_out);
		cr_assert_eq(x3_out._vv, 101);
	}
	ResourceStats end_stats = get_resource_stats();
	cr_assert_eq(init_stats.malloc_sum, end_stats.malloc_sum);
}
