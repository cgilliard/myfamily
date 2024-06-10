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

#define CleanupTest CleanupTestPtr CLEANUP(cleanup_test_free)

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
	ErrorKind kind1 = EKIND("ILLEGAL_STATE");
	ErrorKind kind2 = EKIND("ILLEGAL_ARGUMENT");

	cr_assert(errorkind_equal(&kind1, &kind1));
	cr_assert(!errorkind_equal(&kind1, &kind2));
}

Test(base, test_error) {
	ErrorKind ILLEGAL_STATE = EKIND("ILLEGAL_STATE");
	ErrorKind ILLEGAL_ARGUMENT = EKIND("ILLEGAL_ARGUMENT");
	Error err1 = ERROR(ILLEGAL_STATE, "Illegal state number %i", 5);
	cr_assert(!strcmp(err1.msg, "Illegal state number 5"));
	cr_assert(equal(&ILLEGAL_STATE, &ILLEGAL_STATE));
	cr_assert(equal(&(err1.kind), &ILLEGAL_STATE));
}

typedef struct {
	Vtable *table;
	int x;
	int y;
} MyStruct1Ptr;
void cleanup_my_struct1(MyStruct1Ptr *obj) { printf("cleanup fn called\n"); }
#define MyStruct1 MyStruct1Ptr CLEANUP(cleanup_my_struct1)

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

static Vtable MyStruct1Vtable = {3,
				 {{"display", display_my_struct1},
				  {"ord", ord_my_struct1},
				  {"equal", equal_my_struct1}}};

MyStruct1 my_struct1_build(int x, int y) {
	MyStruct1Ptr ret = {&MyStruct1Vtable, x, y};
	return ret;
}

typedef struct {
	Vtable *table;
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
#define MyStruct2 MyStruct2Ptr CLEANUP(cleanup_my_struct2)

void display_my_struct2(MyStruct2 *mstr) { printf("x='%s'\n", mstr->x); }

static Vtable MyStruct2Vtable = {1, {{"display", display_my_struct2}}};

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

static Vtable AnotherVtable = {0, NULL};

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
