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
#include <core/rc.h>
#include <core/result.h>
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
	u32 out = GET(MyTestClass, c1, testf);
	assert_eq(out, 8);
	SET(MyTestClass, c1, testf, 9);
	out = GET(MyTestClass, c1, testf);
	assert_eq(out, 9);
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
	u32 out = GET(MyTestClass, c1_out, testf);
	assert_eq(out, 8);
}

ENUM(MyEnum, VARIANTS(TYPE_U32, TYPE_U64), TYPES("u32", "u64"))
#define MyEnum DEFINE_ENUM(MyEnum)

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
}

static ErrorKind ILLEGAL_ARGUMENT = EKIND("IllegalArgument");
static ErrorKind ILLEGAL_STATE = EKIND("IllegalState");

Result test_fn(u64 x) {
	if (x < 100) {
		Error err = ERROR(ILLEGAL_ARGUMENT, "test");
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

	Error err = ERROR(ILLEGAL_ARGUMENT, "test");
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
		      u16 v4_out = UNWRAP_IMPL(r4, v4_out);
		      assert_eq(v4_out, 101);
		      confirm = true;
	      }) VARIANT(Err, { confirm = false; }));

	assert(confirm);

	U128 v128 = BUILD(U128, 1234);
	Result r5 = Ok(v128);
	U128 v128_out = UNWRAP_IMPL(r5, v128_out);
	u128 v128_out_unwrap = *(u128 *)unwrap(&v128_out);
	assert_eq(v128_out_unwrap, 1234);

	Result r6 = test_fn(200);
	u64 v6 = UNWRAP_IMPL(r6, v6);
	assert_eq(v6, 1234);

	Result r7 = test_fn(1);
	bool is_err = false;
	MATCH(r7, VARIANT(Err, { is_err = true; }));
	assert(is_err);
	Error err7 = UNWRAP_ERR(r7);
	assert(equal(KIND(err7), &ILLEGAL_ARGUMENT));
}

Result test_try2(u64 x) {
	if (x < 50) {
		Error e = ERROR(ILLEGAL_STATE, "test_try2");
		return Err(e);
	}
	return Ok(x);
}

Result test_try(u64 x) {
	Result r = test_try2(x);
	u64 y = TRY(r, y);
	if (x < 100) {
		Error e = ERROR(ILLEGAL_ARGUMENT, "test_try");
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
	u16 v5_out = UNWRAP_IMPL(r5, v5_out);

	U64Ptr u = BUILD(U64, 1001);
	Result r6 = Ok(u);
	U64 u_out = UNWRAP(r6, U64);
	u64 u_out_unwrap = *(u64 *)unwrap(&u_out);
	assert_eq(u_out_unwrap, 1001);
}

CLASS(TestCleanup, FIELD(void *, ptr))
#define TestCleanup DEFINE_CLASS(TestCleanup)
GETTER(TestCleanup, ptr)
SETTER(TestCleanup, ptr)

void TestCleanup_cleanup(TestCleanup *tc) {
	void *p = GET(TestCleanup, *tc, ptr);
	if (p != NULL) {
		myfree(p);
		SET(TestCleanup, *tc, ptr, NULL);
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
}
