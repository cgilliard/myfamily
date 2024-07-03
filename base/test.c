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

#include <base/enum.h>
#include <base/test.h>

FamSuite(base);

FamTest(base, test_basic) { return Ok(UNIT); }

ENUM(MyEnum, VARIANTS(TYPE_INT, TYPE_FLOAT, TYPE_STRING),
     TYPES("u64", "f32", "StringRef"))
#define MyEnum DEFINE_ENUM(MyEnum)

CLASS(MyClass, FIELD(u64, value))
#define MyClass DEFINE_CLASS(MyClass)

void MyClass_cleanup(MyClass *ptr) {
	printf("--------myclass cleanup %i\n", ptr);
}

ENUM(MyEnum2, VARIANTS(TYPE_MY_CLASS, TYPE_U64), TYPES("MyClass", "u64"))
#define MyEnum2 DEFINE_ENUM(MyEnum2)

FamTest(base, test_enum) {
	u64 value1 = 49;
	MyEnum x = BUILD_ENUM(MyEnum, TYPE_INT, value1);
	u64 value1_out = ENUM_VALUE(value1_out, u64, x);
	assert_eq(value1_out, 49);
	printf("x.type=%i %i\n", x.type, x.value);

	u64 ret;
	MATCH(x, VARIANT(TYPE_INT, {
		      u64 val = ENUM_VALUE(val, u64, x);
		      printf("type_int: %llu\n", val);
		      ret = 100;
	      }) VARIANT(TYPE_STRING, {
		      StringRef val = ENUM_VALUE(val, StringRef, x);
		      printf("type_string: %s\n", to_str(&val));
		      ret = 200;
	      }) VARIANT(TYPE_FLOAT, { printf("type_float\n"); }));

	assert_eq(ret, 100);

	StringRef value2 = STRINGP("this is a test");
	MyEnum x2 = BUILD_ENUM(MyEnum, TYPE_STRING, value2);
	StringRef value2_out = ENUM_VALUE(value2_out, StringRef, x2);
	printf("1: %s\n", CLASS_NAME(&value2_out));
	assert_eq_str(to_str(&value2_out), "this is a test");
	printf("2\n");

	i64 ret2 = 0;
	MATCH(x2, VARIANT(TYPE_INT, { printf("type_int\n"); })
		      VARIANT(TYPE_STRING, {
			      StringRef val2 = ENUM_VALUE(val2, StringRef, x2);
			      printf("type_string: %s\n", to_str(&val2));
			      ret2 = -1;
		      }) VARIANT(TYPE_FLOAT, {
			      printf("type_float\n");
			      ret2 = -2;
		      }) VARIANT(TYPE_FLOAT, {
			      printf("type_float\n");
			      ret2 = -3;
		      }));
	assert_eq(ret2, -1);

	MyClassPtr *mc = tlmalloc(sizeof(MyClass));
	mc->_value = 1234;
	BUILDPTR(mc, MyClass);
	Rc rc = RC(mc);

	MyEnum2 x3 = BUILD_ENUM(MyEnum2, TYPE_MY_CLASS, rc);

	printf("pre match\n");
	u64 ret4 = 0;
	MATCH(x3, VARIANT(TYPE_MY_CLASS, {
		      MyClass mc2 = ENUM_VALUE(mc2, MyClass, x3);
		      printf("mc.value=%llu\n", mc2._value);
		      ret4 = mc2._value;
	      }));
	assert_eq(ret4, 1234);
	printf("post match\n");

	return Ok(UNIT);
}

ENUM(TestVariants, VARIANTS(VU8, VU16, VU32), TYPES("u8", "u16", "u32"))
#define TestVariants DEFINE_ENUM(TestVariants)

u64 do_match(TestVariants tv) {
	u64 check = 0;
	MATCH(tv, VARIANT(VU8, { check = 1; }) VARIANT(VU16, { check = 2; })
		      VARIANT(VU32, { check = 3; }));
	return check;
}

FamTest(base, test_enum2) {
	u64 check = 0;
	u8 vu8 = 1;
	u16 vu16 = 2;
	u32 vu32 = 3;

	TestVariants mu8 = BUILD_ENUM(TestVariants, VU8, vu8);
	check = do_match(mu8);
	assert_eq(check, 1);

	TestVariants mu16 = BUILD_ENUM(TestVariants, VU16, vu16);
	check = do_match(mu16);
	assert_eq(check, 2);

	TestVariants mu32 = BUILD_ENUM(TestVariants, VU32, vu32);
	check = do_match(mu32);
	assert_eq(check, 3);

	return Ok(UNIT);
}
