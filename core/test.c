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

MyTest(core, test_result) {
	I64 v = BUILD(I64, 10);
	Result r = Ok(v);
	bool is_ok = false;
	MATCH(r, VARIANT(Ok, { is_ok = true; }));
	assert(is_ok);

	ErrorKind ILLEGAL_ARGUMENT = EKIND("IllegalArgument");
	Error err = ERROR(ILLEGAL_ARGUMENT, "test");
	Result r2 = Err(err);
	MATCH(r2, VARIANT(Err, {
		      Error e2 = ENUM_VALUE(e2, Error, r2);
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
		      u16 v4_out = UNWRAP(r4, v4_out);
		      assert_eq(v4_out, 101);
		      confirm = true;
	      }) VARIANT(Err, { confirm = false; }));

	assert(confirm);

	U128 v128 = BUILD(U128, 1234);
	Result r5 = Ok(v128);
	U128 v128_out = UNWRAP(r5, v128_out);
	u128 v128_out_unwrap = *(u128 *)unwrap(&v128_out);
	assert_eq(v128_out_unwrap, 1234);
}
