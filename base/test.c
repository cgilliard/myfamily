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

#include <base/error.h>
#include <base/option.h>
#include <base/result.h>
#include <base/string.h>
#include <base/test.h>
#include <base/unit.h>

FamSuite(base);

FamTest(base, test_tmalloc) {}

FamTest(base, test_result) {
	u64 v1 = 3;
	Result r1 = Ok(v1);
	assert(r1.is_ok());
	assert_eq(*(u64 *)unwrap(&r1), 3);

	i128 v2 = -10;
	Result r2 = Ok(v2);
	assert(r2.is_ok());
	assert_eq(*(i128 *)unwrap(&r2), -10);

	U64Ptr *v3 = tlmalloc(sizeof(U64));
	BUILDPTR(v3, U64);
	v3->_value = 101;
	Rc rc3 = RC(v3);
	Result r3 = Ok(rc3);
	U64 v3_out = *(U64 *)unwrap(&r3);
	assert_eq(v3_out._value, 101);
}

FamTest(base, test_option) {
	u64 v1 = 3;
	Option o1 = Some(v1);
	assert(o1.is_some());
	assert_eq(*(u64 *)unwrap(&o1), 3);

	i128 v2 = -10;
	Option o2 = Some(v2);
	assert(o2.is_some());
	assert_eq(*(i128 *)unwrap(&o2), -10);

	U64Ptr *v3 = tlmalloc(sizeof(U64));
	BUILDPTR(v3, U64);
	v3->_value = 101;
	Rc rc3 = RC(v3);
	Option o3 = Some(rc3);
	U64 v3_out = *(U64 *)unwrap(&o3);
	assert_eq(v3_out._value, 101);
}

FamTest(base, test_string) {
	StringPtr *s1 = STRINGPTRP("this is a test");
	Result r2 = STRINGPTR("another string");
	Rc rc = *(Rc *)unwrap(&r2);
	StringPtr s2 = *(String *)unwrap(&rc);

	assert_eq_str(to_str(&s2), "another string");
	assert_eq_str(to_str(s1), "this is a test");
	assert_eq(len(s1), strlen("this is a test"));
	assert_eq(len(&s2), strlen("another string"));

	String s3;
	clone(&s3, &s2);

	assert(equal(&s2, &s3));
	assert(!equal(&s1, &s3));

	Result ra = append(&s3, s1);
	assert_eq_str(to_str(&s3), "another stringthis is a test");
	assert_eq(len(&s3), strlen("another stringthis is a test"));

	assert_eq_str(unwrap(&s3), "another stringthis is a test");

	Result r4 = INDEX_OF(&s3, "this is");
	i64 res = *(i64 *)unwrap(&r4);
	assert_eq(res, 14);

	StringPtr *s5 = STRINGPTRP("i");
	Result r5 = INDEX_OF(&s3, s5);
	i64 res5 = *(i64 *)unwrap(&r5);
	assert_eq(res5, 11);

	Result r6 = LAST_INDEX_OF(&s3, s5);
	i64 res6 = *(i64 *)unwrap(&r6);
	assert_eq(res6, 19);

	Result r7 = LAST_INDEX_OF(&s3, "abababaa");
	i64 res7 = *(i64 *)unwrap(&r7);
	assert_eq(res7, -1);

	Result r8 = char_at(&s3, 0);
	signed char ch8 = *(signed char *)unwrap(&r8);
	assert_eq(ch8, 'a');
	Result r9 = char_at(&s3, 2);
	signed char ch9 = *(signed char *)unwrap(&r9);
	assert_eq(ch9, 'o');

	Result r10 = char_at(&s3, 10000);
	assert(!r10.is_ok());

	Result r11 = SUBSTRING(&s3, 0, 7);
	RcPtr *rc11 = unwrap(&r11);
	StringPtr *s11 = unwrap(rc11);
	assert_eq_str(unwrap(s11), "another");

	cleanup(s1);
	tlfree(s1);

	cleanup(s5);
	tlfree(s5);
}

FamTest(base, test_string_ref) {
	StringRef s1 = STRINGP("this is a test");
	Result r2 = STRING("another string");
	StringRef s2 = *(StringRef *)unwrap(&r2);

	assert_eq_str(to_str(&s2), "another string");
	assert_eq_str(to_str(&s1), "this is a test");
	assert_eq(len(&s1), strlen("this is a test"));
	assert_eq(len(&s2), strlen("another string"));

	StringRef s3;
	clone(&s3, &s2);

	assert(equal(&s2, &s3));
	assert(!equal(&s1, &s3));

	Result ra = append(&s3, &s1);
	assert_eq_str(to_str(&s3), "another stringthis is a test");
	assert_eq(len(&s3), strlen("another stringthis is a test"));
	assert_eq_str(unwrap(&s3), "another stringthis is a test");

	Result r4 = INDEX_OF(&s3, "this is");
	i64 res = *(i64 *)unwrap(&r4);
	assert_eq(res, 14);

	StringRef s5 = STRINGP("i");
	Result r5 = INDEX_OF(&s3, &s5);
	i64 res5 = *(i64 *)unwrap(&r5);
	assert_eq(res5, 11);

	Result r6 = LAST_INDEX_OF(&s3, &s5);
	i64 res6 = *(i64 *)unwrap(&r6);
	assert_eq(res6, 19);

	Result r7 = LAST_INDEX_OF(&s3, "abababaa");
	i64 res7 = *(i64 *)unwrap(&r7);
	assert_eq(res7, -1);

	Result r8 = char_at(&s3, 0);
	signed char ch8 = *(signed char *)unwrap(&r8);
	assert_eq(ch8, 'a');
	Result r9 = char_at(&s3, 2);
	signed char ch9 = *(signed char *)unwrap(&r9);
	assert_eq(ch9, 'o');

	Result r10 = char_at(&s3, 10000);
	assert(!r10.is_ok());

	Result r11 = SUBSTRING(&s3, 0, 7);
	RcPtr *rc11 = unwrap(&r11);
	StringPtr *s11 = unwrap(rc11);
	assert_eq_str(unwrap(s11), "another");
}

FamTest(base, test_error_clone) {
	ErrorKind kind1 = EKIND("TestKind");
	Error e1 = ERROR(kind1, "this is a test");
	Error e2;
	copy(&e2, &e1);
	ErrorKindPtr *k2 = Error_get_kind(&e2);
	assert(equal(&kind1, k2));
	char *msg = (char *)Error_get_message(&e2);
	assert_eq_str(msg, "this is a test");
	print(&e1);
	print(&e2);
}
