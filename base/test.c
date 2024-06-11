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

#include <base/backtrace.h>
#include <base/cleanup.h>
#include <base/error.h>
#include <base/panic.h>
#include <base/result.h>
#include <base/tlmalloc.h>
#include <base/types.h>
#include <base/vtable.h>
#include <criterion/criterion.h>
#include <stdio.h>

Test(base, test_tlmalloc) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_realloc_count = realloc_count();
	u64 initial_free_count = free_count();
	u128 initial_bytes_alloc = cur_bytes_alloc();

	void *ptr1 = tlmalloc(10);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 10);
	cr_assert_eq(alloc_count(), initial_alloc_count + 1);
	cr_assert_eq(realloc_count(), initial_realloc_count);
	cr_assert_eq(free_count(), initial_free_count);

	void *ptr2 = tlmalloc(20);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 30);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count);
	cr_assert_eq(free_count(), initial_free_count);

	ptr2 = tlrealloc(ptr2, 30);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 60);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count + 1);
	cr_assert_eq(free_count(), initial_free_count);

	tlfree(ptr1);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 60);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count + 1);
	cr_assert_eq(free_count(), initial_free_count + 1);

	tlfree(ptr2);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 60);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count + 1);
	cr_assert_eq(free_count(), initial_free_count + 2);
}

typedef struct CleanupTestPtr {
	char *s;
	int x;
} CleanupTestPtr;

void cleanup_test_free(CleanupTestPtr *ptr) {
	if (ptr->s) {
		tlfree(ptr->s);
		ptr->s = NULL;
	}
}

#define CleanupTest CleanupTestPtr Cleanup(cleanup_test_free)

CleanupTest build_cleanup_test(char *s, int x) {

	CleanupTestPtr ret;
	ret.x = x;
	ret.s = tlmalloc((strlen(s) + 1) * sizeof(char));
	strcpy(ret.s, s);

	return ret;
}

Test(base, test_cleanup) {
	CleanupTestPtr *tmp;

	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;

	{
		CleanupTest ct = build_cleanup_test("this is a test", 5);
		cr_assert_eq(ct.x, 5);
		tmp = &ct;
		cr_assert(!strcmp("this is a test", ct.s));
		// tmp->s is not null
		cr_assert_neq(tmp->s, NULL);
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	cr_assert_eq(final_diff, initial_diff);
	// cleanup is called and tmp->s is now null
	cr_assert_eq(tmp->s, NULL);

	// it's ok to call cleanup twice
	cleanup_test_free(tmp);
}

Test(base, test_bt) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;
	{
		Backtrace b = backtrace_generate(100);
		backtrace_print(&b, 0);
		cr_assert(b.count > 0);
		cr_assert(b.count <= 100);
	}
	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	cr_assert_eq(final_diff, initial_diff);
}

Test(base, test_error_kind) {
	ErrorKind kind1 = EKind("ILLEGAL_STATE");
	ErrorKind kind2 = EKind("ILLEGAL_ARGUMENT");

	cr_assert(errorkind_equal(&kind1, &kind1));
	cr_assert(!errorkind_equal(&kind1, &kind2));
}

Test(base, test_error) {
	ErrorKind ILLEGAL_STATE = EKind("ILLEGAL_STATE");
	ErrorKind ILLEGAL_ARGUMENT = EKind("ILLEGAL_ARGUMENT");
	Error err1 = ERROR(ILLEGAL_STATE, "Illegal state number %i", 5);
	cr_assert(!strcmp(err1.msg, "Illegal state number 5"));
	cr_assert(equal(&ILLEGAL_STATE, &ILLEGAL_STATE));
	cr_assert(equal(&(err1.kind), &ILLEGAL_STATE));
}

typedef struct {
	Vtable *vtable;
	int x;
	int y;
} MyStruct1Ptr;
void cleanup_my_struct1(MyStruct1Ptr *obj) { printf("cleanup fn called\n"); }
#define MyStruct1 MyStruct1Ptr Cleanup(cleanup_my_struct1)

void display_my_struct1(MyStruct1 *mstr) {
	printf("x=%i,y=%i\n", mstr->x, mstr->y);
}

int ord_my_struct1(MyStruct1 *obj, MyStruct1 *other) {
	if (obj->x == other->x)
		return 0;
	else if (obj->x > other->x)
		return 1;
	else
		return -1;
}

bool equal_my_struct1(MyStruct1 *obj1, MyStruct1 *obj2) {
	return obj1->x == obj2->x && obj1->y == obj2->y;
}

static VtableEntry MyStruct1VtableEntries[] = {{"display", display_my_struct1},
					       {"ord", ord_my_struct1},
					       {"equal", equal_my_struct1}};
DEFINE_VTABLE(MyStruct1Vtable, MyStruct1VtableEntries)

MyStruct1 my_struct1_build(int x, int y) {
	MyStruct1Ptr ret = {&MyStruct1Vtable, x, y};
	return ret;
}

typedef struct {
	Vtable *vtable;
	char *x;
} MyStruct2Ptr;
void cleanup_my_struct2(MyStruct2Ptr *obj) {
	printf("cleanup fn called my_struct2\n");
	if (obj->x) {
		printf("tlfree %i\n", obj->x);
		tlfree(obj->x);
		obj->x = NULL;
	}
}
#define MyStruct2 MyStruct2Ptr Cleanup(cleanup_my_struct2)

void display_my_struct2(MyStruct2 *mstr) { printf("x='%s'\n", mstr->x); }

static VtableEntry MyStruct2VtableEntries[] = {{"display", display_my_struct2}};
DEFINE_VTABLE(MyStruct2Vtable, MyStruct2VtableEntries)

MyStruct2 my_struct2_build(char *x) {
	char *x_copy = tlmalloc(sizeof(char) * (1 + strlen(x)));
	strcpy(x_copy, x);
	MyStruct2Ptr ret = {&MyStruct2Vtable, x_copy};
	return ret;
}

void test_display(void *obj) {
	void (*display)(Object *obj) = find_fn((Object *)obj, "display");
	if (display)
		display(obj);
}

int test_ord(void *obj, void *other) {
	if (((Object *)obj)->vtable != ((Object *)other)->vtable) {
		panic("attempt to compare values of different types");
	}
	int (*ord)(Object *obj, Object *other) = find_fn((Object *)obj, "ord");
	if (ord == NULL)
		panic("ord not implemented");
	return ord(obj, other);
}

static Vtable AnotherVtable = {0, UNIQUE_ID, NULL};

Test(base, test_vtable) {
	MyStruct1 test1 = my_struct1_build(10, 20);
	test_display(&test1);

	MyStruct1 test2 = my_struct1_build(100, 200);
	test_display(&test2);

	MyStruct1 test3 = my_struct1_build(1000, 2000);
	test_display(&test3);

	printf("o1=%i\n", test_ord(&test1, &test2));
	printf("o2=%i\n", test_ord(&test2, &test1));
	printf("oe=%i\n", test_ord(&test1, &test1));

	MyStruct2 test4 = my_struct2_build("hello!");
	test_display(&test4);

	cr_assert(equal(&test1, &test1));
	cr_assert(!equal(&test1, &test2));
	cr_assert(!equal(&test1, &test4));
}

typedef struct {
	Vtable *vtable;
	int x;
	int y;
	char *z;
} MyStructResPtr;

void cleanup_my_struct_res(MyStructResPtr *msr) {
	if (msr->z) {
		tlfree(msr->z);
		msr->z = NULL;
	}
}

#define MyStructRes MyStructResPtr Cleanup(cleanup_my_struct_res)

void my_struct_res_copy(MyStructRes *dst, MyStructRes *src) {
	dst->vtable = src->vtable;
	dst->x = src->x;
	dst->y = src->y;
	dst->z = tlmalloc(sizeof(char) * (strlen(src->z) + 1));
	strcpy(dst->z, src->z);
}

size_t my_struct_res_size(MyStructRes *obj) { return sizeof(MyStructRes); }

static VtableEntry MyStructResVtableEntries[] = {
    {"copy", my_struct_res_copy},
    {"size", my_struct_res_size},
    {"cleanup", cleanup_my_struct_res}};
DEFINE_VTABLE(MyStructResVtable, MyStructResVtableEntries)

MyStructRes my_struct_res_build(int x, int y, char *z) {
	MyStructResPtr ret;

	ret.z = tlmalloc(sizeof(char) * (strlen(z) + 1));
	strcpy(ret.z, z);
	ret.x = x;
	ret.y = y;
	ret.vtable = &MyStructResVtable;

	return ret;
}

static ErrorKind test_err3 = EKind("TEST_ERR");
static ErrorKind test_err2 = EKind("OTHER");
static ErrorKind test_err = EKind("TEST_ERR");

Test(base, test_result) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();
	u64 initial_diff = initial_alloc_count - initial_free_count;
	{
		MyStructRes msr = my_struct_res_build(1, 2, "this is a test");

		Result r1 = result_build_ok(&msr);

		cr_assert(r1.is_ok());

		MyStructResPtr *res = unwrap(&r1);

		cr_assert_eq(res->x, 1);
		cr_assert_eq(res->y, 2);

		Error e = ERROR(test_err3, "this is a test");

		Result r2 = result_build_err(e);

		cr_assert(!r2.is_ok());
		ErrorPtr *err = unwrap_err(&r2);
		cr_assert(equal(&err->kind, &test_err));
		cr_assert(!equal(&err->kind, &test_err2));
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("initialdiff=%llu,final_diff=%llu\n", initial_diff, final_diff);
	cr_assert_eq(initial_diff, final_diff);
}

static ErrorKind ILLEGAL_STATE = EKind("IllegalState");
static ErrorKind ILLEGAL_ARGUMENT = EKind("IllegalArgument");

Result my_test_fun(int x, int y) {
	if (x > 100) {
		Error e = ERROR(ILLEGAL_ARGUMENT, "arg not valid: %i", x);
		return Err(e);
	} else if (y > 100) {
		Error e = ERROR(ILLEGAL_STATE, "state not valid: %i", y);
		return Err(e);
	} else {
		MyStructRes msr = my_struct_res_build(x, y, "a val");
		return Ok(msr);
	}
}

Test(base, test_sample) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();
	u64 initial_diff = initial_alloc_count - initial_free_count;
	{
		Result r1 = my_test_fun(10, 2);
		cr_assert(r1.is_ok());
		MyStructResPtr *msr = unwrap(&r1);
		cr_assert_eq(msr->x, 10);
		cr_assert_eq(msr->y, 2);

		Result r2 = my_test_fun(300, 2);
		cr_assert(!r2.is_ok());
		ErrorPtr *e2 = unwrap_err(&r2);
		cr_assert(equal(&e2->kind, &ILLEGAL_ARGUMENT));

		Result r3 = my_test_fun(30, 200);
		cr_assert(!r3.is_ok());
		ErrorPtr *e3 = unwrap_err(&r3);
		cr_assert(equal(&e3->kind, &ILLEGAL_STATE));
	}
	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("initialdiff=%llu,final_diff=%llu\n", initial_diff, final_diff);
	cr_assert_eq(initial_diff, final_diff);
}

Result my_test_fun2() {
	f64 x = 10.9;
	return Ok(x);
}

Test(base, test_primitives) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();
	u64 initial_diff = initial_alloc_count - initial_free_count;
	{
		u64 v1 = 10;
		u32 v2 = 1;

		Result r1 = Ok(v1);
		cr_assert(r1.is_ok());
		u64 *res = unwrap(&r1);
		cr_assert_eq(*res, v1);

		MyStructRes msr = my_struct_res_build(10, 20, "a val");
		Result r2 = Ok(msr);

		Result r3 = Ok(v2);
		cr_assert(r3.is_ok());
		u32 *res3 = unwrap(&r3);
		cr_assert_eq(*res3, v2);

		f32 f1 = 9.0;
		Result r4 = Ok(f1);
		cr_assert(r4.is_ok());
		f32 *res4 = unwrap(&r4);
		cr_assert_eq(*res4, f1);
		printf("r4id=%i, test_err3.id=%i\n", r4.vtable->id,
		       test_err3.vtable->id);

		Result tr = my_test_fun2();
		cr_assert(tr.is_ok());
		f64 *tres = unwrap(&tr);
		cr_assert_eq(*tres, 10.9);
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("initialdiff=%llu,final_diff=%llu\n", initial_diff, final_diff);
	cr_assert_eq(initial_diff, final_diff);
}

void test_vtable_fn(void *obj) {}
void test_vtable_fn2(void *obj) {}

typedef struct {
	Vtable *vtable;
	int x;
	int y;
} MyTestVtableStruct;

static VtableEntry MyTestVtableStructVtableEntries[] = {
    {"test", test_vtable_fn}, {"test2", test_vtable_fn2}};
DEFINE_VTABLE(MyTestVtableStructTable, MyTestVtableStructVtableEntries)

Test(base, test_vtable2) {
	printf("in test\n");
	MyTestVtableStruct mtvs = {&MyTestVtableStructTable, 0, 1};
	cr_assert(find_fn((Object *)&mtvs, "test") != NULL);
	cr_assert(find_fn((Object *)&mtvs, "test2") != NULL);
	cr_assert(find_fn((Object *)&mtvs, "test3") == NULL);
	cr_assert_eq(MyTestVtableStructTable.len, 2);
}

Result fn_test_1(u64 x) {
	if (x > 100) {
		Error err = ERROR(ILLEGAL_STATE, "illegal state");
		return Err(err);
	}

	return Ok(x);
}

Result fn_test_2(u64 x) {
	if (x > 200) {
		Error err = ERROR(ILLEGAL_ARGUMENT, "illegal state");
		return Err(err);
	}

	Result rr = fn_test_1(x);
	u64 *res = Try(rr);
	return Ok(*res);
}

Test(base, test_unwrap) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();
	u64 initial_diff = initial_alloc_count - initial_free_count;
	{
		Result r1 = fn_test_2(10);
		cr_assert(r1.is_ok());
		u64 *res = unwrap(&r1);
		cr_assert_eq(*res, 10);

		Result r2 = fn_test_2(101);
		cr_assert(!r2.is_ok());
		ErrorPtr *e2 = unwrap_err(&r2);
		cr_assert(equal(&e2->kind, &ILLEGAL_STATE));

		Result r3 = fn_test_2(201);
		cr_assert(!r3.is_ok());
		ErrorPtr *e3 = unwrap_err(&r3);
		cr_assert(equal(&e3->kind, &ILLEGAL_ARGUMENT));

		Result r4 = fn_test_2(20);
		cr_assert(r4.is_ok());
		u64 *res4 = Expect(r4);
		cr_assert_eq(*res4, 20);
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("initialdiff=%llu,final_diff=%llu\n", initial_diff, final_diff);
	cr_assert_eq(initial_diff, final_diff);
}
