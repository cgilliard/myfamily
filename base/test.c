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

#include <base/class.h>
#include <base/error.h>
#include <base/rc.h>
#include <base/traits.h>
#include <base/unit.h>
#include <criterion/criterion.h>
#include <stdio.h>

int default_favorite_number2(void *obj) { return 2; }

int favorite_number3(void *obj) {
	int (*do_favorite_number3)(Object *) =
	    find_fn((Object *)obj, "favorite_number3");

	if (do_favorite_number3 == NULL)
		panic("do_favorite_number3 not implemented for this type");

	return do_favorite_number3(obj);
}

int favorite_number2(void *obj) {
	int (*do_favorite_number2)(Object *) =
	    find_fn((Object *)obj, "favorite_number2");

	if (do_favorite_number2 == NULL)
		panic("do_favorite_number2 not implemented for this type");

	return do_favorite_number2(obj);
}

int favorite_number(void *obj) {
	int (*do_favorite_number)(Object *) =
	    find_fn((Object *)obj, "favorite_number");

	if (do_favorite_number == NULL)
		panic("do_favorite_number not implemented for this type");
	return do_favorite_number(obj);
}

#define TRAIT_FAVORITE_NUMBER3(T)                                              \
	TRAIT_REQUIRED(T, int, favorite_number3, T##Ptr *obj)

#define TRAIT_FAVORITE_NUMBER(T)                                               \
	TRAIT_REQUIRED(T, int, favorite_number, T##Ptr *obj)                   \
	TRAIT_IMPL(T, favorite_number2, default_favorite_number2)              \
	TRAIT_SUPER(T, TRAIT_FAVORITE_NUMBER3)

CLASS(testself, FIELD(u32, x) FIELD(u64, y))
IMPL(testself, TRAIT_SIZE)
IMPL(testself, TRAIT_FAVORITE_NUMBER);
#define testself DEFINE_CLASS(testself)
GETTER(testself, y)
SETTER(testself, y)
GETTER(testself, x)
SETTER(testself, x)

int testself_favorite_number(testself *self) { return 3; }

int testself_favorite_number3(testself *self) { return 300; }

void testself_cleanup(testselfPtr *ptr) {
	printf("cleanup.x=%i,x.y=%i\n", *testself_get_x(ptr),
	       *testself_get_y(ptr));
}
size_t testself_size(testselfPtr *ptr) { return sizeof(testselfPtr); }

CLASS(testself2, FIELD(u32, x) FIELD(u64, y) FIELD(u64, z))
IMPL(testself2, TRAIT_SIZE)
IMPL(testself2, TRAIT_FAVORITE_NUMBER);
#define testself2 DEFINE_CLASS(testself2)
int testself2_favorite_number2(testself2 *self);
OVERRIDE(testself2, favorite_number2, testself2_favorite_number2);
GETTER(testself2, y)
SETTER(testself2, y)
GETTER(testself2, x)
SETTER(testself2, x)

int testself2_favorite_number3(testself2 *self) { return 400; }

int testself2_favorite_number2(testself2 *self) { return 100; }

int testself2_favorite_number(testself2 *self) { return 4; }

void testself2_cleanup(testself2Ptr *ptr) {
	printf("cleanup2.x=%i,x.y=%i\n", *testself2_get_x(ptr),
	       *testself2_get_y(ptr));
}
size_t testself2_size(testself2Ptr *ptr) { return sizeof(testself2Ptr); }

Test(base, test_class) {
	testself ts = BUILD(testself, 10, 20);
	printf("ts.x=%u,ts.y=%llu\n", *testself_get_x(&ts),
	       *testself_get_y(&ts));
	cr_assert_eq(*testself_get_y(&ts), 20);
	cr_assert_eq(*testself_get_x(&ts), 10);

	testself_set_y(&ts, 200);
	testself_set_x(&ts, 100);

	cr_assert_eq(*testself_get_y(&ts), 200);
	cr_assert_eq(*testself_get_x(&ts), 100);

	cr_assert_eq(size(&ts), sizeof(testself));

	cr_assert_eq(favorite_number2(&ts), 2);
	cr_assert_eq(favorite_number(&ts), 3);
	cr_assert_eq(favorite_number3(&ts), 300);

	testself2 ts2 = BUILD(testself2, 11, 22);
	cr_assert_eq(*testself2_get_y(&ts2), 22);
	cr_assert_eq(*testself2_get_x(&ts2), 11);

	cr_assert_eq(size(&ts2), sizeof(testself2));
	cr_assert_neq(size(&ts), size(&ts2));

	cr_assert_eq(favorite_number(&ts2), 4);
	cr_assert_eq(favorite_number2(&ts2), 100);
	cr_assert_eq(favorite_number3(&ts2), 400);
}

Test(base, test_backtrace) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;

	{
		BacktraceEntry bt = EMPTY_BACKTRACE_ENTRY;
		bool ret = BacktraceEntry_set_backtrace_entry_values(
		    &bt, "name", "bin_name", "address", "file_path");
		cr_assert(ret);

		cr_assert(!strcmp(*BacktraceEntry_get_name(&bt), "name"));
		cr_assert(
		    !strcmp(*BacktraceEntry_get_bin_name(&bt), "bin_name"));
		cr_assert(!strcmp(*BacktraceEntry_get_address(&bt), "address"));
		cr_assert(
		    !strcmp(*BacktraceEntry_get_file_path(&bt), "file_path"));

		BacktraceEntry btcopy = EMPTY_BACKTRACE_ENTRY;
		ret = copy(&btcopy, &bt);
		cr_assert(ret);

		cr_assert(!strcmp(*BacktraceEntry_get_name(&btcopy), "name"));
		cr_assert(
		    !strcmp(*BacktraceEntry_get_bin_name(&btcopy), "bin_name"));
		cr_assert(
		    !strcmp(*BacktraceEntry_get_address(&btcopy), "address"));
		cr_assert(!strcmp(*BacktraceEntry_get_file_path(&btcopy),
				  "file_path"));

		Backtrace btp = BUILD(Backtrace, NULL, 0);
		Backtrace_generate(&btp, 100);
		print(&btp, 0);
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("init=%i,final=%i\n", initial_diff, final_diff);
	cr_assert_eq(final_diff, initial_diff);
}

static ErrorKind ILLEGAL_STATE = BUILD(ErrorKind, "IllegalState");
static ErrorKind ILLEGAL_STATE2 = BUILD(ErrorKind, "IllegalState");
static ErrorKind ILLEGAL_ARGUMENT = BUILD(ErrorKind, "IllegalArgument");

Test(base, test_errorkind) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;

	{
		ErrorKind x = BUILD(ErrorKind, "IllegalState");
		cr_assert(
		    !strcmp((char *)ErrorKind_get_kind(&x), "IllegalState"));

		ErrorKind test = BUILD(ErrorKind, "IllegalState");
		cr_assert(equal(&ILLEGAL_STATE, &test));
		cr_assert(!equal(&ILLEGAL_STATE, &ILLEGAL_ARGUMENT));
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("init=%i,final=%i\n", initial_diff, final_diff);
	cr_assert_eq(final_diff, initial_diff);
}

Test(base, test_error) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;

	{
		BacktracePtr bt = BUILD(Backtrace, NULL, 0);

		Backtrace_generate(&bt, 100);

		Error x = BUILD(Error, ILLEGAL_STATE, "some message", bt);

		BacktracePtr bt2 = BUILD(Backtrace, NULL, 0);
		Backtrace_generate(&bt2, 100);

		Error y =
		    BUILD(Error, ILLEGAL_ARGUMENT, "another message", bt2);

		cr_assert(!equal(&x, &y));

		BacktracePtr bt3 = BUILD(Backtrace, NULL, 0);
		Backtrace_generate(&bt3, 100);
		Error z =
		    BUILD(Error, ILLEGAL_ARGUMENT, "yet another message", bt3);

		cr_assert(!equal(&x, &z));

		cr_assert(equal(&y, &z));
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("init=%i,final=%i\n", initial_diff, final_diff);
	cr_assert_eq(final_diff, initial_diff);
}

Test(base, test_error_build) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;

	{
		Error err = ERROR(ILLEGAL_STATE, "hi there");
		Error err2 = ERROR(ILLEGAL_STATE2, "ok");
		Error err3 = ERROR(ILLEGAL_ARGUMENT, "test");
		cr_assert(equal(&err, &err2));
		cr_assert(!equal(&err, &err3));
		cr_assert(!strcmp((char *)Error_get_message(&err), "hi there"));
		cr_assert(!strcmp((char *)Error_get_message(&err2), "ok"));
		cr_assert(!strcmp((char *)Error_get_message(&err3), "test"));

		BacktracePtr bt = BUILD(Backtrace, NULL, 0);
		Error err4 = {{&ErrorVtable}, {}, "abc123", bt};
		copy(&err4, &err3);
		printf("4 = %s, 3 = %s\n", (char *)Error_get_message(&err4),
		       (char *)Error_get_message(&err3));
		cr_assert(!strcmp((char *)Error_get_message(&err4), "test"));

		print(&err, 0);
		print(&err3, 0);
		print(&err4, 0);
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("init=%i,final=%i\n", initial_diff, final_diff);
	cr_assert_eq(final_diff, initial_diff);
}

Test(base, test_rc) {
	int *x = tlmalloc(sizeof(int));
	*x = 100;
	Rc r = BUILD(Rc, x);
	printf("x=%i\n", *(int *)(*Rc_get_p(&r)));
	cr_assert_eq(*(int *)(*Rc_get_p(&r)), 100);
}

Test(base, copy_bt) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;

	{
		Backtrace bt1 = BUILD(Backtrace, NULL, 0);
		Backtrace_generate(&bt1, 100);

		Backtrace bt2 = BUILD(Backtrace, NULL, 0);
		copy(&bt2, &bt1);
		cr_assert_eq(*Backtrace_get_count(&bt1),
			     *Backtrace_get_count(&bt2));
		BacktraceEntryPtr *ptr1 = *Backtrace_get_rows(&bt1);
		BacktraceEntryPtr *ptr2 = *Backtrace_get_rows(&bt2);

		printf("ptr1.name=%s\n", *BacktraceEntry_get_name(&ptr1[0]));
		printf("ptr2.name=%s\n", *BacktraceEntry_get_name(&ptr2[0]));
		printf("ptr1.name=%s\n", *BacktraceEntry_get_name(&ptr1[1]));
		printf("ptr2.name=%s\n", *BacktraceEntry_get_name(&ptr2[1]));

		for (u64 i = 0; i < *Backtrace_get_count(&bt1); i++) {
			cr_assert(!strcmp(*BacktraceEntry_get_name(&ptr1[i]),
					  *BacktraceEntry_get_name(&ptr2[i])));
		}

		BacktracePtr bt3 = BUILD(Backtrace, NULL, 0);
		Backtrace_generate(&bt3, 100);
		dispose(&bt3);
		u64 x = 10;
		u32 y = 20;
		dispose(&x);
		dispose(&y);
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	printf("init=%i,final=%i\n", initial_diff, final_diff);
	cr_assert_eq(final_diff, initial_diff);
}

Test(base, test_copy) {
	u64 x1 = 10, x2 = 20;
	copy(&x1, &x2);
	cr_assert_eq(x1, x2);
	size_t size_u64 = size(&x1);
	u32 x3 = 3;
	u64 xu32 = size(&x3);
	cr_assert_eq(xu32, 4);
	u64 xu64 = size(&x1);
	cr_assert_eq(xu64, 8);
}
