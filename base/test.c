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

ENUM(TestVariants,
     VARIANTS(VU8, VU16, VU32, VU64, VU128, VI8, VI16, VI32, VI64, VI128, VBOOL,
	      VUSIZE, VF32, VF64),
     TYPES("u8", "u16", "u32", "u64", "u128", "i8", "i16", "i32", "i64", "i128",
	   "bool", "usize", "f32", "f64"))
#define TestVariants DEFINE_ENUM(TestVariants)

u64 do_match(TestVariants tv) {
	u64 check = 0;
	// clang-format off
	MATCH(
		tv,
		VARIANT(VU8, { check = 1; })
		VARIANT(VU16, { check = 2; })
		VARIANT(VU32, { check = 3; })
		VARIANT(VU64, { check = 4; })
		VARIANT(VU128, { check = 5; })
		VARIANT(VI8, { check = 6; })
		VARIANT(VI16, { check = 7; })
		VARIANT(VI32, { check = 8; })
		VARIANT(VI64, { check = 9; })
		VARIANT(VI128, { check = 10; })
		VARIANT(VF32, { check = 11; })
		VARIANT(VF64, { check = 12; })
		VARIANT(VBOOL, { check = 13; })
		VARIANT(VUSIZE, { check = 14; })
	);
	// clang-format on
	return check;
}

i128 do_match_value(TestVariants tv) {
	i128 check = 0;
	// clang-format off
        MATCH(
                tv,
                VARIANT(VU8, { u8 checkr = ENUM_VALUE(checkr, u8, tv); check = checkr; })
                VARIANT(VU16, { u16 checkr = ENUM_VALUE(checkr, u16, tv); check = checkr;  })
                VARIANT(VU32, { u32 checkr = ENUM_VALUE(checkr, u32, tv); check = checkr;  })
                VARIANT(VU64, { u64 checkr = ENUM_VALUE(checkr, u64, tv); check = checkr; })
                VARIANT(VU128, { u128 checkr = ENUM_VALUE(checkr, u128, tv); check = checkr; })
                VARIANT(VI8, { i8 checkr = ENUM_VALUE(checkr, i8, tv); check = checkr; })
                VARIANT(VI16, { i16 checkr = ENUM_VALUE(checkr, i16, tv); check = checkr; })
                VARIANT(VI32, { i32 checkr = ENUM_VALUE(checkr, i32, tv); check = checkr; })
                VARIANT(VI64, { i64 checkr = ENUM_VALUE(checkr, i64, tv); check = checkr; })
                VARIANT(VI128, { i128 checkr = ENUM_VALUE(checkr, i128, tv); check = checkr; })
                VARIANT(VF32, { f32 checkr = ENUM_VALUE(checkr, f32, tv); check = checkr; })
                VARIANT(VF64, { f64 checkr = ENUM_VALUE(checkr, f64, tv); check = checkr; })
                VARIANT(VBOOL, { bool checkr = ENUM_VALUE(checkr, bool, tv); check = checkr; })
                VARIANT(VUSIZE, { usize checkr = ENUM_VALUE(checkr, usize, tv); check = checkr; })
        );
	// clang-format on
	return check;
}

FamTest(base, test_enum2) {
	u64 check = 0;
	u8 vu8 = 10;
	u16 vu16 = 20;
	u32 vu32 = 30;
	u64 vu64 = 40;
	u128 vu128 = 50;
	i8 vi8 = 60;
	i16 vi16 = 70;
	i32 vi32 = 80;
	i64 vi64 = 90;
	i128 vi128 = 100;
	f32 vf32 = 110.0;
	f64 vf64 = 120.0;
	bool vbool = false;
	usize vusize = 140;

	TestVariants mu8 = BUILD_ENUM(TestVariants, VU8, vu8);
	check = do_match(mu8);
	assert_eq(check, 1);
	check = do_match_value(mu8);
	assert_eq(check, 10);

	TestVariants mu16 = BUILD_ENUM(TestVariants, VU16, vu16);
	check = do_match(mu16);
	assert_eq(check, 2);
	check = do_match_value(mu16);
	assert_eq(check, 20);

	TestVariants mu32 = BUILD_ENUM(TestVariants, VU32, vu32);
	check = do_match(mu32);
	assert_eq(check, 3);
	check = do_match_value(mu32);
	assert_eq(check, 30);

	TestVariants mu64 = BUILD_ENUM(TestVariants, VU64, vu64);
	check = do_match(mu64);
	assert_eq(check, 4);
	check = do_match_value(mu64);
	assert_eq(check, 40);

	TestVariants mu128 = BUILD_ENUM(TestVariants, VU128, vu128);
	check = do_match(mu128);
	assert_eq(check, 5);
	check = do_match_value(mu128);
	assert_eq(check, 50);

	TestVariants mi8 = BUILD_ENUM(TestVariants, VI8, vi8);
	check = do_match(mi8);
	assert_eq(check, 6);
	check = do_match_value(mi8);
	assert_eq(check, 60);

	TestVariants mi16 = BUILD_ENUM(TestVariants, VI16, vi16);
	check = do_match(mi16);
	assert_eq(check, 7);
	check = do_match_value(mi16);
	assert_eq(check, 70);

	TestVariants mi32 = BUILD_ENUM(TestVariants, VI32, vi32);
	check = do_match(mi32);
	assert_eq(check, 8);
	check = do_match_value(mi32);
	assert_eq(check, 80);

	TestVariants mi64 = BUILD_ENUM(TestVariants, VI64, vi64);
	check = do_match(mi64);
	assert_eq(check, 9);
	check = do_match_value(mi64);
	assert_eq(check, 90);

	TestVariants mi128 = BUILD_ENUM(TestVariants, VI128, vi128);
	check = do_match(mi128);
	assert_eq(check, 10);
	check = do_match_value(mi128);
	assert_eq(check, 100);

	TestVariants mf32 = BUILD_ENUM(TestVariants, VF32, vf32);
	check = do_match(mf32);
	assert_eq(check, 11);
	check = do_match_value(mf32);
	assert_eq(check, 110);

	TestVariants mf64 = BUILD_ENUM(TestVariants, VF64, vf64);
	check = do_match(mf64);
	assert_eq(check, 12);
	check = do_match_value(mf64);
	assert_eq(check, 120);

	TestVariants mbool = BUILD_ENUM(TestVariants, VBOOL, vbool);
	check = do_match(mbool);
	assert_eq(check, 13);
	check = do_match_value(mbool);
	assert_eq(check, false);

	TestVariants musize = BUILD_ENUM(TestVariants, VUSIZE, vusize);
	check = do_match(musize);
	assert_eq(check, 14);
	check = do_match_value(musize);
	assert_eq(check, 140);

	return Ok(UNIT);
}
