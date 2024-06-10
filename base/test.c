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

#include <base/tlmalloc.h>
#include <base/types.h>
#include <criterion/criterion.h>
#include <stdio.h>

const ErrorKind ILLEGAL_STATE = {"IllegalState"};

Test(base, ErrorKindTest) {
	ErrorKind ekind1 = {"ArrayIndexOutOfBounds"};
	ErrorKind ekind2 = {"IllegalState"};
	ErrorKind ekind3 = {"ArrayIndexOutOfBounds"};
	ErrorKind ekind4 = {"IllegalState"};

	cr_assert_eq(errorkind_equal(&ekind1, &ekind2), false);
	cr_assert_eq(errorkind_equal(&ekind1, &ekind3), true);
	cr_assert_eq(errorkind_equal(&ekind1, &ekind4), false);
	cr_assert_eq(errorkind_equal(&ekind2, &ekind4), true);
}

Test(base, ErrorTest) {
	Error err1 = {{"ArrayIndexOutOfBounds"},
		      "array index (10) was greater than array.len (8)",
		      NULL};

	Error err2 = {{"ArrayIndexOutOfBounds"},
		      "array index (30) was greater than array.len (18)",
		      NULL};

	Error err3 = {{"IllegalState"}, "invalid state", NULL};

	cr_assert_eq(error_equal(&err1, &err2), true);
	cr_assert_eq(error_equal(&err1, &err3), false);

	char buf[MAX_ERROR_MESSAGE_LEN];
	error_to_string(buf, &err1);
	cr_assert_eq(strcmp(buf, "ArrayIndexOutOfBounds: array index (10) was "
				 "greater than array.len (8)"),
		     0);
}

typedef struct MyStructPtr {
	int x;
	int y;
	char *data;
} MyStructPtr;

void my_struct_free(MyStructPtr *ptr) {
	if (ptr->data != NULL) {
		tlfree(ptr->data);
		ptr->data = NULL;
	}
}

#define MyStruct MyStructPtr CLEANUP(my_struct_free)

MyStructPtr my_struct_build(int x, int y, char *data) {
	char *data_copy = tlmalloc(sizeof(char) * (strlen(data) + 1));
	strcpy(data_copy, data);
	MyStructPtr ret = {x, y, data_copy};
	return ret;
}

void copy_my_struct(void *dest, void *src) {
	MyStructPtr *dest_ms = dest;
	MyStructPtr *src_ms = src;
	dest_ms->x = src_ms->x;
	dest_ms->y = src_ms->y;
	if (src_ms->data != NULL) {
		int len = strlen(src_ms->data);
		dest_ms->data = tlmalloc(sizeof(char) * (len + 1));
		strcpy(dest_ms->data, src_ms->data);
	}
}

Test(base, user_defined) {
	MyStruct m = my_struct_build(2, 3, "test1");
	Result r = OkC(&r, m, copy_my_struct);

	cr_assert(r.is_ok());

	MyStruct unwrapped = UNWRAP(&unwrapped, r);
	cr_assert_eq(unwrapped.x, 2);
	cr_assert_eq(unwrapped.y, 3);
	cr_assert(!strcmp(unwrapped.data, "test1"));

	printf("pre\n");
	String s = STRINGP(&s, "hello");
	printf("post\n");
	// printf("post %s\n", s.to_str());
	cr_assert(!strcmp(s.ptr, "hello"));
	// cr_assert(!strcmp(s.to_str(), "this is a test999"));
}

Result test_call(Result *res) {

	Result r;
	StringPtr *s = CALL(res, STRING(&r, "abc123"));

	printf("svalue=%s\n", s->ptr);
	cr_assert(!strcmp(s->ptr, "abc123"));

	return Ok(res, UNIT);
}

Test(base, StringTest) {
	void *ptr = NULL;
	{
		String x = STRINGP(&x, "this is a test44");
		cr_assert(!strcmp(TO_STR(x), "this is a test44"));
		String s;
		s.ptr = malloc(sizeof(char) * 10);
		ptr = &s;
		strcpy(s.ptr, "test");
		cr_assert_neq(((String *)ptr)->ptr, NULL);
	}

	cr_assert_eq(((String *)ptr)->ptr, NULL);

	Result r = STRING(&r, "this is a test1");
	String s = UNWRAP(&s, r);
	cr_assert(!strcmp(TO_STR(s), "this is a test1"));

	String s5 = STRINGP(&s5, "abc123");
	cr_assert(!strcmp(TO_STR(s5), "abc123"));

	StringPtr *s2 = Unwrap1(STRING(&r, "this is a test2"));
	cr_assert(!strcmp(s2->ptr, "this is a test2"));

	Result r2 = test_call(&r2);
	cr_assert(r2.is_ok());
}

Result test_error(Result *res, int x) {
	if (x > 100) {
		int y = x + 100;
		return Ok(res, y);
	} else {
		Error err = ERROR(&err, ILLEGAL_STATE, "test error %d", 1);
		return Err(res, err);
	}
}

Result test_string(Result *res, int x) {
	if (x > 100) {
		String s = STRINGP(&s, "test str");
		return Ok(res, s);
	} else {
		Error err = ERROR(&err, ILLEGAL_STATE, "test error %d", 3);
		return Err(res, err);
	}
}

Test(base, result) {
	void *ptr = NULL;
	u64 x = 3;
	Result r = Ok(&r, x);
	cr_assert_eq(r.is_ok(), true);

	u64 *ret = Unwrap1(r);
	cr_assert_eq(*ret, 3);
	printf("0\n");
	Result r1 = test_error(&r1, 200);
	cr_assert_eq(r1.is_ok(), true);
	u64 *v = Unwrap1(r1);
	cr_assert_eq(*v, 300);

	printf("1\n");
	Result r2 = test_error(&r2, 20);
	cr_assert_eq(r2.is_ok(), false);

	Error e = Unwrap_err(&e, r2);

	Error compare = ERROR(&compare, ILLEGAL_STATE, "test error2 %d", 2);

	printf("Printing e:\n");
	error_print(&e, 0);
	printf("Printing compare:\n");
	error_print(&compare, 0);

	cr_assert(error_equal(&e, &compare));

	char buf[100];
	cr_assert(
	    !strcmp(error_to_string(buf, &e), "IllegalState: test error 1"));

	Result rr = test_error(&rr, 20);
	u64 vv = rr.is_ok() ? *((u64 *)Unwrap1(rr)) : 0;
	printf("vv=%i\n", vv);

	Result rr2 = test_error(&rr2, 2030);
	u64 vv2 = Unwrap_or(rr2, (u64)2);
	printf("vv2=%i\n", vv2);

	Result rr3 = test_string(&rr3, 200);
	StringPtr *sx3 = Unwrap1(rr3);
	printf("s='%s'\n", sx3->ptr);
	cr_assert(!strcmp(sx3->ptr, "test str"));
	printf("end\n");

	Result rr4 = test_string(&rr4, 1);
	cr_assert(!rr4.is_ok());

	printf("end2\n");
}

Test(base, backtrace) {}

Test(base, TestCopy) {
	u64 x = 3;
	Copy test = COPY(test, x);
	u64 dst = 1;
	test.copy(&dst, &x);
	cr_assert_eq(dst, 3);

	void *confirm1;
	void *confirm2;
	{
		String s = STRINGP(&s, "test");
		String s2;

		Copy stest = COPY(stest, s);
		stest.copy(&s2, &s);
		confirm1 = &s;
		confirm2 = &s2;
		cr_assert(!strcmp(s2.ptr, "test"));
	}

	cr_assert_eq(((String *)confirm1)->ptr, NULL);
	cr_assert_eq(((String *)confirm2)->ptr, NULL);
}

Result alloc_test_fun(Result *res) {
	MyStruct m = my_struct_build(5, 7, "test9");
	Result rz = OkC(&rz, m, copy_my_struct);

	cr_assert(rz.is_ok());

	MyStruct unwrapped = UNWRAP(&unwrapped, rz);
	cr_assert_eq(unwrapped.x, 5);
	cr_assert_eq(unwrapped.y, 7);
	cr_assert(!strcmp(unwrapped.data, "test9"));

	Result r;
	r = Ok(&r, UNIT);
	Result r2;
	int x = 8;
	r2 = Ok(&r2, x);
	Error e = ERROR(&e, ILLEGAL_STATE, "ill state");
	Result r3;
	r3 = Err(&r3, e);

	Error zz = Unwrap_err(&zz, r3);
	return Ok(res, UNIT);
}

Test(base, alloc) {
	u64 initial_alloc = alloc_count();
	u64 initial_bytes_alloc = cur_bytes_alloc();
	u64 initial_free_count = free_count();

	{

		printf("initial = %llu %llu %llu\n", initial_alloc,
		       initial_bytes_alloc, initial_free_count);

		String s = STRINGP(&s, "testing 1234");

		cr_assert(!strcmp(TO_STR(s), "testing 1234"));

		Result r;
		String s2 = UNWRAP(&s2, STRING(&r, "abcdef"));
		cr_assert(!strcmp(TO_STR(s2), "abcdef"));

		Result r2 = alloc_test_fun(&r2);
		cr_assert(r2.is_ok());

		Error e = ERROR(&e, ILLEGAL_STATE, "ill state");

		Backtrace bt = EMPTY_BACKTRACE;
		backtrace_generate(&bt, 100);

		Error e2 = ERROR(&e2, ILLEGAL_STATE, "ill state");
		printf("err ok\n");
		Result rx = Err(&rx, e2);
		printf("res ok\n");
	}

	u64 final_alloc = alloc_count();
	u64 final_bytes_alloc = cur_bytes_alloc();
	u64 final_free_count = free_count();
	printf("final = %llu %llu %llu\n", final_alloc, final_bytes_alloc,
	       final_free_count);

	u64 diff_free = final_free_count - initial_free_count;
	u64 diff_alloc = final_alloc - initial_alloc;

	cr_assert_eq(diff_free, diff_alloc);
}
