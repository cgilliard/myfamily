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

#include <core/class.h>
#include <core/enum.h>
#include <core/error.h>
#include <core/format.h>
#include <core/formatter.h>
#include <core/option.h>
#include <core/rc.h>
#include <core/result.h>
#include <core/string.h>
#include <core/test.h>
#include <core/tuple.h>
#include <core/unit.h>

MySuite(core);

CLASS(MyTestClass, FIELD(u32, testf));
#define MyTestClass DEFINE_CLASS(MyTestClass)
void MyTestClass_cleanup(MyTestClassPtr *ptr) {}
GETTER(MyTestClass, testf);
SETTER(MyTestClass, testf);

MyTest(core, test_class) {
	MyTestClass c1 = BUILD(MyTestClass, 8);
	u32 out = GET(MyTestClass, &c1, testf);
	assert_eq(out, 8);
	SET(MyTestClass, &c1, testf, 9);
	out = GET(MyTestClass, &c1, testf);
	assert_eq(out, 9);
	return Ok(_());
}

MyTest(core, test_tuple) {
	u8 vu8 = 10;
	u16 vu16 = 20;
	MyTestClass c1 = BUILD(MyTestClass, 8);
	Rc c1_rc = HEAPIFY(c1);
	Tuple tuple = TUPLE(vu8, vu16, c1_rc);
	u8 vu8_out;
	u16 vu16_out;
	ELEMENT_AT(&tuple, 0, &vu8_out);
	assert_eq(vu8_out, 10);
	ELEMENT_AT(&tuple, 1, &vu16_out);
	assert_eq(vu16_out, 20);
	MyTestClass c1_out;
	ELEMENT_AT(&tuple, 2, &c1_out);
	u32 out = GET(MyTestClass, &c1_out, testf);
	assert_eq(out, 8);
	return Ok(_());
}

ENUM(MyEnum, VARIANTS(TYPE_U32, TYPE_U64), TYPES("u32", "u64"))
#define MyEnum DEFINE_ENUM(MyEnum)

ENUM(MyEnum2, VARIANTS(TYPE_U322, TYPE_U642, TYPE_BOOL),
     TYPES("u32", "u64", "bool"))
#define MyEnum2 DEFINE_ENUM(MyEnum2)

MyTest(core, test_enum) {
	u64 value1 = 49;
	MyEnum x = BUILD_ENUM(MyEnum, TYPE_U64, value1);
	u64 value1_out = ENUM_VALUE(value1_out, u64, x);
	assert_eq(value1_out, 49);

	u64 ret;
	MATCH(x, VARIANT(TYPE_U64, {
		      u64 val = ENUM_VALUE(val, u64, x);
		      ret = val + 100;
	      }) VARIANT(TYPE_U32, {
		      u32 val = ENUM_VALUE(val, u32, x);
		      ret = val + 200;
	      }));

	assert_eq(ret, 149);

	bool value2 = false;
	MyEnum2 y = BUILD_ENUM(MyEnum2, TYPE_BOOL, value2);
	bool value2_out = true;
	MATCH(y, VARIANT(TYPE_BOOL,
			 { value2_out = ENUM_VALUE(value2_out, bool, y); }));
	assert(!value2_out);

	bool value3 = true;
	MyEnum2 z = BUILD_ENUM(MyEnum2, TYPE_BOOL, value3);
	bool value3_out = false;
	MATCH(z, VARIANT(TYPE_BOOL,
			 { value3_out = ENUM_VALUE(value2_out, bool, z); }));
	assert(value3_out);

	u64 value4 = 101;
	MyEnum my_enum = TRY_BUILD_ENUM(MyEnum, TYPE_U64, value4);
	u64 value4_out = 0;
	MATCH(my_enum, VARIANT(TYPE_U64, {
		      value4_out = TRY_ENUM_VALUE(value4_out, u64, my_enum);
	      }));
	assert_eq(value4_out, 101);

	return Ok(_());
}

Result test_fn(u64 x) {
	if (x < 100) {
		Error err = ERR(ILLEGAL_ARGUMENT, "test");
		return Err(err);
	} else {
		u64 y = 1234;
		return Ok(y);
	}
}

MyTest(core, test_result) {
	I64 v = BUILD(I64, 10);
	Result r = Ok(v);
	bool is_ok = false;
	MATCH(r, VARIANT(Ok, { is_ok = true; }));
	assert(is_ok);

	Error err = ERR(ILLEGAL_ARGUMENT, "test");
	Result r2 = Err(err);
	MATCH(r2, VARIANT(Err, {
		      Error e2 = UNWRAP_ERR(r2);
		      assert(equal(KIND(e2), &ILLEGAL_ARGUMENT));
		      is_ok = false;
	      }));
	assert(!is_ok);

	I32 v2 = BUILD(I32, 1);
	Result r3 = Ok(v2);
	MATCH(r3, VARIANT(Ok, { is_ok = true; }));
	assert(is_ok);

	u16 v4 = 101;
	Result r4 = Ok(v4);

	bool confirm = false;

	MATCH(r4, VARIANT(Ok, {
		      u16 v4_out = UNWRAP_PRIM(r4, v4_out);
		      assert_eq(v4_out, 101);
		      confirm = true;
	      }) VARIANT(Err, { confirm = false; }));

	assert(confirm);

	U128 v128 = BUILD(U128, 1234);
	Result r5 = Ok(v128);
	U128 v128_out = UNWRAP_PRIM(r5, v128_out);
	u128 v128_out_unwrap = *(u128 *)unwrap(&v128_out);
	assert_eq(v128_out_unwrap, 1234);

	Result r6 = test_fn(200);
	u64 v6 = UNWRAP_PRIM(r6, v6);
	assert_eq(v6, 1234);

	Result r7 = test_fn(1);
	bool is_err = false;
	MATCH(r7, VARIANT(Err, { is_err = true; }));
	assert(is_err);
	Error err7 = UNWRAP_ERR(r7);
	assert(equal(KIND(err7), &ILLEGAL_ARGUMENT));
	return Ok(_());
}

Result test_try2(u64 x) {
	if (x < 50) {
		Error e = ERR(ILLEGAL_STATE, "test_try2");
		return Err(e);
	}
	return Ok(x);
}

Result test_try(u64 x) {
	Result r = test_try2(x);
	u64 y = TRY(r, y);
	if (x < 100) {
		Error e = ERR(ILLEGAL_ARGUMENT, "test_try");
		return Err(e);
	}
	return Ok(y);
}

MyTest(core, test_try_expect) {
	Result r1 = test_try(200);
	assert(IS_OK(r1));

	Result r2 = test_try(90);
	assert(IS_ERR(r2));
	Error e2 = UNWRAP_ERR(r2);
	assert(equal(KIND(e2), &ILLEGAL_ARGUMENT));

	Result r3 = test_try(9);
	assert(IS_ERR(r3));
	Error e3 = UNWRAP_ERR(r3);
	assert(equal(KIND(e3), &ILLEGAL_STATE));

	Result r4 = test_try(200);
	u64 v4 = EXPECT(r4, v4);
	assert_eq(v4, 200);

	u16 v5 = 10;
	Result r5 = Ok(v5);
	u16 v5_out = UNWRAP_PRIM(r5, v5_out);

	U64Ptr u = BUILD(U64, 1001);
	Result r6 = Ok(u);
	U64 u_out = UNWRAP(r6, U64);
	u64 u_out_unwrap = *(u64 *)unwrap(&u_out);
	assert_eq(u_out_unwrap, 1001);
	return Ok(_());
}

CLASS(TestCleanup, FIELD(void *, ptr))
#define TestCleanup DEFINE_CLASS(TestCleanup)
GETTER(TestCleanup, ptr)
SETTER(TestCleanup, ptr)

void TestCleanup_cleanup(TestCleanup *tc) {
	void *p = GET(TestCleanup, tc, ptr);
	if (p != NULL) {
		myfree(p);
		SET(TestCleanup, tc, ptr, NULL);
	}
}

Result ret_test_cleanup() {
	void *ptr = mymalloc(1);
	TestCleanupPtr tc = BUILD(TestCleanup, ptr);

	return Ok(tc);
}

MyTest(core, test_cleanup) {
	void *ptr = mymalloc(1);
	TestCleanupPtr tc = BUILD(TestCleanup, ptr);
	Result r1 = Ok(tc);
	TestCleanup tc_out = UNWRAP(r1, TestCleanup);

	Result r2 = ret_test_cleanup();

	Result r3 = ret_test_cleanup();
	TestCleanup tc3_out = UNWRAP(r3, TestCleanup);

	return Ok(_());
}

MyTest(core, test_option) {
	u64 v1 = 10;
	Option opt1 = Some(v1);
	assert(IS_SOME(opt1));
	bool is_some = false;

	MATCH(opt1, VARIANT(SOME, { is_some = true; })
			VARIANT(NONE, { is_some = false; }));
	assert(is_some);

	u64 v1_out = UNWRAP_PRIM(opt1, v1_out);
	assert_eq(v1_out, 10);

	Option opt2 = None;
	assert(IS_NONE(opt2));

	return Ok(_());
}

MyTest(core, test_complex) {
	u32 x1 = 123;
	ResultPtr x2 = Ok(x1);
	ResultPtr x3 = Ok(x2);
	ResultPtr x4 = Ok(x3);
	OptionPtr x5 = Some(x4);
	OptionPtr x6 = Some(x5);
	Result x7 = Ok(x6);

	Option x6_out = UNWRAP(x7, Option);
	Option x5_out = UNWRAP(x6_out, Option);
	Result x4_out = UNWRAP(x5_out, Result);
	Result x3_out = UNWRAP(x4_out, Result);
	Result x2_out = UNWRAP(x3_out, Result);
	u32 x1_out = UNWRAP_PRIM(x2_out, x1_out);
	assert_eq(x1_out, 123);

	return Ok(_());
}

MyTest(core, test_string) {
	Result r1 = String_build("this is a test");
	String s1 = TRY(r1, s1);
	assert_eq_str(unwrap(&s1), "this is a test");
	assert_eq(len(&s1), strlen("this is a test"));

	String s2;
	bool ret = clone(&s2, &s1);
	assert(ret);
	assert_eq_str(unwrap(&s2), "this is a test");
	assert_eq(len(&s2), strlen("this is a test"));

	assert(equal(&s1, &s2));

	Result r3 = String_build("this is a test2");
	String s3 = TRY(r3, s3);

	assert(!equal(&s1, &s3));

	Result r4 = append(&s3, &s2);
	assert(IS_OK(r4));

	// s3 is the string which has been appended to
	assert_eq_str(unwrap(&s3), "this is a test2this is a test");
	assert_eq(len(&s3), strlen("this is a test2this is a test"));

	// s2 is unchanged
	assert_eq_str(unwrap(&s2), "this is a test");
	assert_eq(len(&s2), strlen("this is a test"));

	return Ok(_());
}

MyTest(core, test_string_core) {
	String s1 = STRING("abcdefghijklmnopqrstuvwxyz");

	Result r2 = String_index_of_s(&s1, "def", 0);
	i64 v2 = UNWRAP_PRIM(r2, v2);
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
	i64 v3 = UNWRAP_PRIM(r3, v3);
	assert_eq(v3, 26);

	return Ok(_());
}

CLASS(FmtClass, FIELD(u64, value1) FIELD(i32, value2))
IMPL(FmtClass, TRAIT_DISPLAY)
IMPL(FmtClass, TRAIT_DEBUG)
#define FmtClass DEFINE_CLASS(FmtClass)
GETTER(FmtClass, value1)
GETTER(FmtClass, value2)

void FmtClass_cleanup(FmtClass *ptr) {}

Result FmtClass_fmt(FmtClass *ptr, Formatter *fmt) {
	u64 v1 = GET(FmtClass, ptr, value1);
	i32 v2 = GET(FmtClass, ptr, value2);
	return WRITE(fmt, "value1: %llu, value2: %i", v1, v2);
}

Result FmtClass_dbg(FmtClass *ptr, Formatter *fmt) {
	u64 v1 = GET(FmtClass, ptr, value1);
	i32 v2 = GET(FmtClass, ptr, value2);
	return WRITE(fmt, "[value1: %llu, value2: %i]", v1, v2);
}

MyTest(core, test_formatter) {
	FmtClass fmtclass = BUILD(FmtClass, 123, -1);
	Result r = to_string(&fmtclass);
	String s = TRY(r, s);
	assert_eq_str(unwrap(&s), "value1: 123, value2: -1");

	Result r2 = to_debug(&fmtclass);
	String s2 = TRY(r2, s2);
	assert_eq_str(unwrap(&s2), "[value1: 123, value2: -1]");

	String s3 = TO_STRING(&fmtclass);
	assert_eq_str(unwrap(&s3), "value1: 123, value2: -1");

	String s4 = TO_DEBUG(&fmtclass);
	assert_eq_str(unwrap(&s4), "[value1: 123, value2: -1]");

	String s5 = STRING("testing abc");
	assert_eq_str(unwrap(&s5), "testing abc");

	Formatter fmt = FORMATTER(1000);
	ResultPtr (*do_fmt)(Object *obj, Formatter *formatter) =
	    find_fn((Object *)&fmtclass, "fmt");
	Result r6 = do_fmt((Object *)&fmtclass, &fmt);
	TRYU(r6);
	Result r7 = Formatter_to_string(&fmt);
	String s7 = UNWRAP(r7, String);
	assert_eq_str(unwrap(&s7), "value1: 123, value2: -1");

	return Ok(_());
}

ENUM(MyEnum3, VARIANTS(TYPE_USIZE, TYPE_U643, TYPE_BOOL3),
     TYPES("u64", "u64", "bool"))
#define MyEnum3 DEFINE_ENUM(MyEnum3)

MyTest(core, test_alloc_error_result) {
	Result rx2 = STATIC_ALLOC_RESULT;
	Error err3 = UNWRAP_ERR(rx2);
	assert(equal(KIND(err3), KIND(STATIC_ALLOC_ERROR)));

	u64 x1 = 505;
	MyEnum3 my_enum = TRY_BUILD_ENUM(MyEnum3, TYPE_USIZE, x1);

	return Ok(_());
}

MyTest(core, test_format) {
	Formatter fmt = FORMATTER(10000);
	u32 x = 123;
	Result r1 = FORMAT(&fmt, "this is a test {}", x);
	Result rr1 = Formatter_to_string(&fmt);
	String s1 = TRY(rr1, s1);
	char *s_out = unwrap(&s1);
	printf("s=%s\n", unwrap(&s1));
	assert_eq_str(s_out, "this is a test 123");
	return Ok(_());
}
