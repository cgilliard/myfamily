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
#include <core/test.h>
#include <pthread.h>
#include <unistd.h>

MySuite(core);

MyTest(core, slab_data) {
	SlabData sd;
	SlabDataParams p;
	Slab slab;
	slab.len = 10;
	p.slab_size = 10;
	p.initial_chunks = 1;
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
	return Ok(UNIT);
}

MyTest(core, slab_allocator) {
	SlabAllocator sa;
	SlabDataParams sdp1, sdp2;
	sdp1.slab_size = 10;
	sdp1.initial_chunks = 1;
	sdp1.max_slabs = 30;
	sdp1.slabs_per_resize = 5;
	sdp1.free_list_head = 0;

	sdp2.slab_size = 20;
	sdp2.initial_chunks = 1;
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
	return Ok(UNIT);
}

MyTest(core, slab_allocator_resize) {
	SlabAllocator sa;
	SlabDataParams sdp1, sdp2;
	sdp1.slab_size = 10;
	sdp1.initial_chunks = 2;
	sdp1.max_slabs = 35;
	sdp1.slabs_per_resize = 5;
	sdp1.free_list_head = 0;

	sdp2.slab_size = 20;
	sdp2.initial_chunks = 1;
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
	return Ok(UNIT);
}

MyTest(core, slab_allocator_params) {
	SlabAllocator sa;
	slab_allocator_build(&sa, false, 2,
			     SLAB_PARAMS(SlabSize(10), InitialChunks(2)),
			     SLAB_PARAMS(SlabSize(50), InitialChunks(3)));

	SlabAllocator sa2 =
	    SLABS(true, SLAB_PARAMS(SlabSize(50), InitialChunks(2)),
		  SLAB_PARAMS(SlabSize(10), InitialChunks(3)));
	cr_assert_eq(sa2.slab_data_arr_size, 2);
	cr_assert_eq(sa2.slab_data_arr[0].sdp.slab_size, 10);
	cr_assert_eq(sa2.slab_data_arr[1].sdp.slab_size, 50);

	// check defaults
	SlabDataParams sdp1 = SLAB_PARAMS();
	cr_assert_eq(sdp1.slab_size, 512);
	cr_assert_eq(sdp1.max_slabs, 100);
	cr_assert_eq(sdp1.slabs_per_resize, 10);
	cr_assert_eq(sdp1.initial_chunks, 1);
	cr_assert_eq(sdp1.free_list_head, 0);

	SlabDataParams sdp2 = SLAB_PARAMS(SlabSize(256));
	cr_assert_eq(sdp2.slab_size, 256);
	cr_assert_eq(sdp2.max_slabs, 100);
	cr_assert_eq(sdp2.slabs_per_resize, 10);
	cr_assert_eq(sdp2.initial_chunks, 1);
	cr_assert_eq(sdp2.free_list_head, 0);

	SlabDataParams sdp3 = SLAB_PARAMS(InitialChunks(3), MaxSlabs(400));
	cr_assert_eq(sdp3.slab_size, 512);
	cr_assert_eq(sdp3.max_slabs, 400);
	cr_assert_eq(sdp3.slabs_per_resize, 10);
	cr_assert_eq(sdp3.initial_chunks, 3);
	cr_assert_eq(sdp3.free_list_head, 0);

	SlabData sd;
	slab_data_build(&sd, sdp3);
	return Ok(UNIT);
}

MyTest(core, test_mymalloc) {
	{
		// init slab allocator and define some slabs
		SlabAllocator sa;
		Slab slab1, slab2, slab3, slab4, slab5, slab6;
		sa = SLABS(false, SLAB_PARAMS(SlabSize(10), InitialChunks(2)),
			   SLAB_PARAMS(SlabSize(50), InitialChunks(3)),
			   SLAB_PARAMS(SlabSize(25), InitialChunks(2)));
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
	return Ok(UNIT);
}

MyTest(core, test_realloc) {
	{
		Slab slab1;
		cr_assert_eq(mymalloc(&slab1, 10), 0);
		// ensure it's the global slab allocator and not the one we
		// define below
		cr_assert_neq(slab1.id, 0);
		cr_assert_eq(myfree(&slab1), 0);

		SlabAllocator sa =
		    SLABS(false, SLAB_PARAMS(SlabSize(10), InitialChunks(2)),
			  SLAB_PARAMS(SlabSize(50), InitialChunks(3)),
			  SLAB_PARAMS(SlabSize(25), InitialChunks(2)));
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
	return Ok(UNIT);
}

MyTest(core, test_0size_initial_sa) {
	{
		SlabAllocator sa = SLABS(
		    false, SLAB_PARAMS(SlabSize(1), InitialChunks(0),
				       MaxSlabs(100), SlabsPerResize(10)));
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
	return Ok(UNIT);
}

MyTest(core, test_nested_slabs) {

	{
		SlabAllocator sa1 = SLABS(
		    false, SLAB_PARAMS(SlabSize(10), InitialChunks(1),
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
							     InitialChunks(1),
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
	return Ok(UNIT);
}

CLASS(MyClass, FIELD(u64, val))
#define MyClass DEFINE_CLASS(MyClass)

void MyClass_cleanup(MyClass *ptr) {}

GETTER(MyClass, val)

MyTest(core, test_class) {
	ResourceStats rs_start = get_resource_stats();
	MyClass x = BUILD(MyClass, 1);
	u64 val = GET(MyClass, &x, val);
	cr_assert_eq(val, 1);
	ResourceStats rs = get_resource_stats();
	cr_assert_eq(rs.slab_misses, rs_start.slab_misses);
	return Ok(UNIT);
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

MyTest(core, test_threads) {
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
	return Ok(UNIT);
}

MyTest(core, test_next_greater_slab_size) {
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
	return Ok(UNIT);
}

MyTest(core, test_rc) {
	Slab slab1;
	mymalloc(&slab1, sizeof(MyClass));
	MyClassPtr *ptr = slab1.data;
	BUILDPTR(ptr, MyClass);
	Rc rc1 = RC(slab1);
	return Ok(UNIT);
}

MyTest(core, test_rc_advanced) {
	Slab slab1;
	mymalloc(&slab1, sizeof(MyClass));
	MyClassPtr *ptr = slab1.data;
	BUILDPTR(ptr, MyClass);
	Rc rc1 = RC(slab1);
	return Ok(UNIT);
}

MyTest(core, test_prim) {
	u64 xu = 1234;
	U64 xv = BUILD(U64, xu);
	u64 xu_out = *(u64 *)unwrap(&xv);
	cr_assert_eq(xu_out, xu);
	Unit x;
	copy(&x, &UNIT);
	cr_assert(equal(&x, &UNIT));
	return Ok(UNIT);
}

MyTest(core, test_backtrace_entry) {
	BacktraceEntry e1 =
	    BUILD(BacktraceEntry, "test1", "test2", "test3", "test4");

	cr_assert(!strcmp(e1._name, "test1"));

	Backtrace bt = INIT_BACKTRACE;
	BACKTRACE(&bt);
	print(&bt);
	return Ok(UNIT);
}

CLASS(MyClass2, FIELD(Slab, slab))
#define MyClass2 DEFINE_CLASS(MyClass2)

void MyClass2_cleanup(MyClass2 *ptr) {}

MyTest(core, test_error) {
	Error err = ERR(ILLEGAL_ARGUMENT, "illegal arg found");
	Result r1 = Err(err);
	print(&err);
	return Ok(UNIT);
}

Result gen_result() {
	u64 x = 1234;
	return Ok(x);
}

CLASS(TestClassHeapify, FIELD(u64, v) FIELD(Slab, s))
#define TestClassHeapify DEFINE_CLASS(TestClassHeapify)
GETTER(TestClassHeapify, s)

static int cleanup_count = 0;

void TestClassHeapify_cleanup(TestClassHeapify *ptr) {
	cleanup_count++;
	Slab s = GET(TestClassHeapify, ptr, s);
	myfree(&s);
}

MyTest(core, test_heapify) {
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
	return Ok(UNIT);
}

MyTest(core, test_rc_clone) {
	cleanup_count = 0;
	Slab slab;
	mymalloc(&slab, 100);
	TestClassHeapify x3 = BUILD(TestClassHeapify, 555, slab);
	Rc rc3 = HEAPIFY(x3);
	Rc rc4;
	myclone(&rc4, &rc3);
	TestClassHeapify x3_out = *(TestClassHeapify *)unwrap(&rc3);
	cr_assert_eq(x3_out._v, 555);
	return Ok(UNIT);
}

CLASS(MyClass3, FIELD(Slab, s) FIELD(u64, vv));
#define MyClass3 DEFINE_CLASS(MyClass3)

void MyClass3_cleanup(MyClass3 *ptr) { myfree(&ptr->_s); }

ENUM(MyEnum, VARIANTS(VV01, VV02, VV03), TYPES("u64", "u32", "MyClass3"))
#define MyEnum DEFINE_ENUM(MyEnum)

ENUM(MyEnum2, VARIANTS(V201, V202, V203, V204, V205, V206, V207),
     TYPES("i8", "i16", "i32", "i64", "i128", "bool", "f64"))
#define MyEnum2 DEFINE_ENUM(MyEnum2)

MyTest(core, test_enum) {
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
	return Ok(UNIT);
}

MyTest(core, test_local_slab_allocator) {
	u64 x = 1;
	loop {
		Slab slab1;
		cr_assert_eq(mymalloc(&slab1, x), 0);
		myfree(&slab1);
		x += 10;
		if (x > 20000)
			break;
	}
	return Ok(UNIT);
}

MyTest(core, test_slab_no_malloc) {

	{

		// init slab allocator and define some slabs
		SlabAllocator sa;
		Slab slab1, slab2, slab3, slab4, slab5, slab6;
		sa = SLABS(
		    false,
		    SLAB_PARAMS(SlabSize(10), InitialChunks(1), MaxSlabs(10)),
		    SLAB_PARAMS(SlabSize(50), InitialChunks(3)),
		    SLAB_PARAMS(SlabSize(25), InitialChunks(2)));
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
	return Ok(UNIT);
}

Result res_fun(int x);

MyTest(core, test_enum_oom) {
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

				Result r = Ok(x1);
				cr_assert(IS_ERR(r));
				Error e = UNWRAP_ERR(r);
				print(&e);

				// this is still an error because we're oom
				Result r2 = res_fun(1);
				cr_assert(IS_ERR(r2));

				Result r3 = res_fun(-1);
				cr_assert(IS_ERR(r3));
				Error e3 = UNWRAP_ERR(r3);
				print(&e3);
			}

			UNSET_SLAB_ALLOCATOR();
		}
	}
	return Ok(UNIT);
}

MyTest(core, test_result) {
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

	Slab _slab___;
	mymalloc(&_slab___, 100);
	MyClass3 x3 = BUILD(MyClass3, _slab___, 101);
	cr_assert_eq(x3._vv, 101);

	Result r3 = Ok(x3);
	MyClass3 x3_out = EXPECT(r3, x3_out);
	cr_assert_eq(x3_out._vv, 101);

	i64 x6 = -888;
	Result r6 = Ok(x6);
	i64 x6_out = EXPECT(r6, x6_out);
	cr_assert_eq(x6_out, -888);
	cr_assert_eq(x6, -888);

	u128 x5 = 222;
	Result r5 = Ok(x5);
	u128 x5_out = EXPECT(r5, x5_out);
	cr_assert_eq(x5_out, 222);
	cr_assert_eq(x5, 222);
	return Ok(UNIT);
}

MyTest(core, test_res_fun) {
	Result r1 = res_fun(1);
	cr_assert(!IS_ERR(r1));
	cr_assert(IS_OK(r1));

	Result r2 = res_fun(-1);
	cr_assert(IS_ERR(r2));
	cr_assert(!IS_OK(r2));
	return Ok(UNIT);
}

Result ret_err() {
	Error e =
	    ERR(ILLEGAL_ARGUMENT, "x was %i. It must be a positive number");
	return Err(e);
}

Result res_fun(int x) {
	if (x < 0) {
		ErrorPtr e = ERR(ILLEGAL_ARGUMENT,
				 "x was %i. It must be a positive number", x);
		return Err(e);
	} else {
		int y = x + 10;
		return Ok(y);
	}
}

Result res_fun2(int x) {
	Result r = res_fun(x);
	int y = TRY(r, y);
	return Ok(y);
}

Result res_fun3(int x) {
	Slab slab;
	mymalloc(&slab, 20000);
	MyClass3Ptr v = BUILD(MyClass3, slab, 107);
	ResultPtr ret = Ok(v);
	return ret;
}

Result res_fun4(int x) {
	Result r = res_fun3(0);
	MyClass3 ret9 = TRY(r, ret9);
	ResultPtr rr1 = Ok(ret9);
	return rr1;
}

MyTest(core, test_try) {
	Result r = res_fun4(0);
	MyClass3 mc3 = EXPECT(r, mc3);
	cr_assert_eq(mc3._vv, 107);

	Result r1 = res_fun2(7);
	cr_assert(IS_OK(r1));
	int x1 = EXPECT(r1, x1);
	cr_assert_eq(x1, 17);

	Result r2 = res_fun2(-10);
	cr_assert(IS_ERR(r2));
	Error e3 = UNWRAP_ERR(r2);
	print(&e3);

	Result r3 = res_fun2(1);
	bool v3 = false;
	MATCH(r3, VARIANT(Ok, { v3 = true; }) VARIANT(Err, { v3 = false; }));
	cr_assert(v3);

	Result r4 = res_fun2(-10);
	bool v4 = false;
	MATCH(r4, VARIANT(Ok, { v4 = true; }) VARIANT(Err, { v4 = false; }));
	cr_assert(!v4);

	Result res1 = STATIC_ALLOC_RESULT;
	Error err1 = UNWRAP_ERR(res1);
	print(&err1);
	return Ok(UNIT);
}

Rc rc_ret1() {
	u64 x1 = 111;
	RcPtr r1 = HEAPIFY(x1);
	return r1;
}

Rc rc_ret2() {
	Rc r1 = rc_ret1();
	RcPtr r2;
	myclone(&r2, &r1);
	return r2;
}

Rc rc_ret3() {
	Slab slab;
	mymalloc(&slab, 300);
	MyClass3 mc3 = BUILD(MyClass3, slab, 11);
	RcPtr rc = HEAPIFY(mc3);
	return rc;
}

MyTest(core, test_rc_2) {
	Rc r1 = rc_ret1();
	Rc r2 = rc_ret1();
	u64 x1_out = *(u64 *)unwrap(&r2);
	cr_assert_eq(x1_out, 111);

	Rc r3 = rc_ret2();
	u64 x3_out = *(u64 *)unwrap(&r3);
	cr_assert_eq(x3_out, 111);

	u16 x4 = 72;
	Rc r4 = HEAPIFY(x4);

	u16 x5 = 77;
	Rc r5 = HEAPIFY(x5);
	u16 x5_out = *(u16 *)unwrap(&r5);
	cr_assert_eq(x5_out, 77);

	Rc r6 = rc_ret3();
	Rc r7 = rc_ret3();
	Rc r8;
	myclone(&r8, &r7);
	MyClass3 r7_out = *(MyClass3 *)unwrap(&r7);
	cr_assert_eq(r7_out._vv, 11);
	return Ok(UNIT);
}

MyTest(core, test_enum2) {
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
	return Ok(UNIT);
}

Result res_fun_simple(i64 v) {
	if (v > 0)
		return Ok(v);
	else {
		Error e = ERR(ILLEGAL_ARGUMENT,
			      "v was %i. It must be a positive number", v);
		return Err(e);
	}
}

MyTest(core, test_result_overhead) {
	Result res = res_fun_simple(77);
	i64 v_out = EXPECT(res, v_out);
	cr_assert_eq(v_out, 77);
	return Ok(UNIT);
}

Result test_option(u32 y) {
	Option x = Some(y);
	cr_assert(IS_SOME(x));
	cr_assert(!IS_NONE(x));
	u32 v1 = EXPECT(x, v1);
	cr_assert_eq(v1, 9);

	Option x2 = None;
	cr_assert(IS_NONE(x2));
	cr_assert(!IS_SOME(x2));

	i64 v3 = -100;
	Option x3 = Some(v3);
	i64 v3_out = TRY(x3, v3_out);
	cr_assert_eq(v3_out, -100);

	return Ok(UNIT);
}

MyTest(core, test_option) {
	Result r = test_option(9);
	cr_assert(IS_OK(r));
	return Ok(UNIT);
}

Result test_complex5(int x) {
	int xy = x + 1;
	Result x1 = Ok(xy);
	Result x2 = Ok(x1);
	Result x3 = Ok(x2);
	Result x4 = Ok(x3);
	Option x5 = Some(x4);
	Option x6 = Some(x5);
	Result x7 = Ok(x6);
	Option x6_out = TRY(x7, x6_out);
	Option x5_out = TRY(x6_out, x5_out);
	Result x4_out = TRY(x5_out, x4_out);
	Result x3_out = TRY(x4_out, x3_out);
	Result x2_out = TRY(x3_out, x2_out);
	Result x1_out = TRY(x2_out, x1_out);
	int v_out = TRY(x1_out, v_out);
	cr_assert_eq(v_out, 1);
	int y = x + 5;
	return Ok(y);
}

Option test_complex4(int x) {
	Result r5 = test_complex5(x);
	int v5 = EXPECT(r5, v5);
	int v = v5 + 5;
	return Some(v);
}

Result test_complex3(int x) {
	Option v4 = test_complex4(x);
	int v = TRY(v4, v);
	return Ok(v);
}

Result test_complex2(int x) {
	Option y = Some(x);
	int z = TRY(y, z);
	cr_assert_eq(z, x);
	return test_complex3(z);
}

Result test_complex1(int x) { return test_complex2(x); }

MyTest(core, test_complex) {
	Result r = test_complex1(0);
	cr_assert(IS_OK(r));
	int r_val = EXPECT(r, r_val);
	cr_assert_eq(r_val, 10);
	return Ok(UNIT);
}

MyTest(core, test_slab_resize) {
	int xy = 1;
	Result x1 = Ok(xy);
	Result x2 = Ok(x1);
	Result x3 = Ok(x2);
	Result x4 = Ok(x3);
	Option x5 = Some(x4);
	Result x6 = Ok(x5);
	return Ok(UNIT);
}

MyTest(core, test_slab_data2) {
	SlabData sd;
	SlabDataParams p;
	Slab slab;
	slab.len = 10;
	p.slab_size = 10;
	p.initial_chunks = 1;
	p.max_slabs = 35;
	p.slabs_per_resize = 10;
	p.free_list_head = 0;
	cr_assert_eq(slab_data_build(&sd, p), 0);

	slab_data_access(&sd, &slab, 0);
	for (int i = 0; i < 10; i++)
		((char *)slab.data)[i] = i;
	slab_data_access(&sd, &slab, 20);
	for (int i = 0; i < 10; i++) {
		((char *)slab.data)[i] = i + 10;
	}

	cr_assert_eq(slab_data_access(&sd, &slab, 0), 0);
	for (int i = 0; i < 10; i++) {
		cr_assert_eq(((char *)slab.data)[i], i);
	}

	cr_assert_eq(slab_data_access(&sd, &slab, 20), 0);
	for (int i = 0; i < 10; i++)
		cr_assert_eq(((char *)slab.data)[i], i + 10);

	cr_assert_eq(slab_data_access(&sd, &slab, 180), -1);

	cr_assert_eq(slab_data_resize(&sd), 0);

	cr_assert_eq(slab_data_access(&sd, &slab, 180), 0);
	for (int i = 0; i < 10; i++) {
		((char *)slab.data)[i] = i + 20;
	}

	cr_assert_eq(slab_data_access(&sd, &slab, 0), 0);
	for (int i = 0; i < 10; i++) {
		cr_assert_eq(((char *)slab.data)[i], i);
	}

	cr_assert_eq(slab_data_access(&sd, &slab, 20), 0);
	for (int i = 0; i < 10; i++)
		cr_assert_eq(((char *)slab.data)[i], i + 10);

	cr_assert_eq(slab_data_access(&sd, &slab, 180), 0);
	for (int i = 0; i < 10; i++)
		cr_assert_eq(((char *)slab.data)[i], i + 20);

	cr_assert_eq(slab_data_access(&sd, &slab, 360), -1);

	cr_assert_eq(slab_data_access(&sd, &slab, 359), 0);
	return Ok(UNIT);
}

MyTest(core, test_string) {
	Result r1 = String_build("this is a test");
	String s1 = TRY(r1, s1);
	assert_eq_str(unwrap(&s1), "this is a test");
	assert_eq(len(&s1), strlen("this is a test"));

	String s2;
	bool ret = myclone(&s2, &s1);
	assert(ret);
	assert_eq_str(unwrap(&s2), "this is a test");
	assert_eq(len(&s2), strlen("this is a test"));

	assert(equal(&s1, &s2));

	Result r3 = String_build("this is a test2");
	String s3 = TRY(r3, s3);

	assert(!equal(&s1, &s3));

	Result r4 = append(&s3, "this is a test");
	assert(IS_OK(r4));

	// s3 is the string which has been appended to
	assert_eq_str(unwrap(&s3), "this is a test2this is a test");
	assert_eq(len(&s3), strlen("this is a test2this is a test"));

	// s2 is unchanged
	assert_eq_str(unwrap(&s2), "this is a test");
	assert_eq(len(&s2), strlen("this is a test"));

	return Ok(UNIT);
}

MyTest(core, test_string_core) {
	String s1 = STRING("abcdefghijklmnopqrstuvwxyz");

	Result r2 = String_index_of_s(&s1, "def", 0);
	i64 v2 = UNWRAP_VALUE(r2, v2);
	assert_eq(v2, 3);

	String s2 = STRING("abcdefghijklmnopqrstuvwxyzabc");
	assert_eq(CHAR_AT(&s2, 3), 'd');

	assert_eq(INDEX_OF(&s2, "ghi"), 6);
	String sub1 = STRING("ghi");
	assert_eq(INDEX_OF(&s2, &sub1), 6);

	assert_eq(INDEX_OF(&s2, "ghi"), 6);
	String sub2 = STRING("ghi");
	assert_eq(INDEX_OF(&s2, &sub2), 6);

	assert_eq(INDEX_OF(&s2, "abc"), 0);
	String sub3 = STRING("abc");
	assert_eq(INDEX_OF(&s2, &sub3), 0);

	assert_eq(LAST_INDEX_OF(&s2, "abc"), 26);
	String sub4 = STRING("abc");
	assert_eq(LAST_INDEX_OF(&s2, &sub4), 26);

	String sub = SUBSTRING(&s2, 3, 6);
	assert_eq_str(unwrap(&sub), "def");

	Result r3 = String_index_of_s(&s2, "abc", 1);
	i64 v3 = UNWRAP_VALUE(r3, v3);
	assert_eq(v3, 26);

	return Ok(UNIT);
}

MyTest(core, test_string_append_s) {
	String s1 = STRING("test1");
	Result r1 = append(&s1, "test2");

	assert_eq_str(unwrap(&s1), "test1test2");

	String s2 = STRING("test222");
	Result r2 = append_s(&s1, (Object *)&s2);
	assert_eq_str(unwrap(&s1), "test1test2test222");
	Result r3 = append(&s1, unwrap(&s2));
	assert_eq_str(unwrap(&s1), "test1test2test222test222");
	String s4 = STRING("abc");
	String s5 = STRING("def");

	Result r4 = append(&s4, &s5);
	assert_eq_str(unwrap(&s4), "abcdef");

	Result r5 = StringBuilder_build("testing");
	StringBuilder sb1 = TRY(r5, sb1);

	Result r6 = append(&sb1, "test");
	assert_eq(1000, GET(StringBuilder, &sb1, capacity));
	Result r7 = StringBuilder_to_string(&sb1);
	String s_out7 = TRY(r7, s_out7);
	assert_eq_str(unwrap(&s_out7), "testingtest");
	assert_eq(1000, GET(StringBuilder, &sb1, capacity));
	u64 len7 = len(&sb1);

	String cont = STRING("continue");
	Result r8 = append(&sb1, &cont);

	Result r9 = StringBuilder_to_string(&sb1);
	String s_out9 = TRY(r9, s_out9);
	assert_eq_str(unwrap(&s_out9), "testingtestcontinue");
	assert_eq(1000, GET(StringBuilder, &sb1, capacity));
	assert_eq(len(&sb1), strlen("testingtestcontinue"));

	StringBuilder sb2 = STRING_BUILDER("", sb2);
	for (u64 i = 0; i < 100; i++) {
		Result r = append(&sb2, "abcdefghijklmnopqrstuvwxyz");
	}
	Result r10 = StringBuilder_to_string(&sb2);
	String s10 = TRY(r10, s10);
	for (u64 i = 0; i < 100 * 26; i++) {
		Result rr = String_char_at(&s10, i);
		i8 b = TRY(rr, b);
		char ch = (char)b;
		assert_eq(ch, 'a' + i % 26);
	}

	u64 cap = GET(StringBuilder, &sb2, capacity);
	u64 sblen = len(&sb2);
	assert_eq(cap, 3000);
	assert_eq(sblen, 2600);

	return Ok(UNIT);
}
