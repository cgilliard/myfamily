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
#include <base/tokenizer.h>
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

FamTest(base, test_deep_copy) {
	StringRef sr1 = STRINGP("test");
	StringRef sr2;
	StringRef sr3 = STRINGP("2");

	copy(&sr2, &sr1);
	assert_eq_str(unwrap(&sr1), unwrap(&sr2));
	Result r1 = append(&sr1, &sr3);
	assert(r1.is_ok());

	assert_eq_str(unwrap(&sr1), "test2");
	assert_eq_str(unwrap(&sr3), "2");
	// not a deep copy just a ref copy so this will be the same memory
	// location as sr1
	assert_eq_str(unwrap(&sr2), "test2");

	StringRef sdr1 = STRINGP("test");
	StringRef sdr2 = BUILD(StringRef);
	StringRef sdr3 = STRINGP("2");

	Result r0 = deep_copy(&sdr2, &sdr1);
	assert(r0.is_ok());
	assert_eq_str(unwrap(&sdr1), unwrap(&sdr2));
	Result r11 = append(&sdr1, &sdr3);
	assert(r11.is_ok());

	assert_eq_str(unwrap(&sdr1), "test2");
	assert_eq_str(unwrap(&sdr3), "2");
	// deep copy occurred so it's separate from sdr1
	assert_eq_str(unwrap(&sdr2), "test");
}

FamTest(base, test_ident) {
	Ident ident = IDENT("abc");
	assert_eq_str(to_str(&ident), "abc");

	Ident ident2;
	bool cr = clone(&ident2, &ident);
	assert(cr);

	assert_eq_str(to_str(&ident2), "abc");
}

FamTest(base, test_literal) {
	Literal literal = LITERAL("\"def\"");
	assert_eq_str(to_str(&literal), "\"def\"");

	Literal literal2;
	bool cr = clone(&literal2, &literal);
	assert(cr);
	assert_eq_str(to_str(&literal2), "\"def\"");

	Literal literal3 = LITERAL(100);
	assert_eq_str(to_str(&literal3), "100");

	Literal literal4;
	cr = clone(&literal4, &literal3);
	assert(cr);
	assert_eq_str(to_str(&literal4), "100");
}

FamTest(base, test_punct) {
	Punct p1 = PUNCT('.', '.', '.');
	assert_eq(*Punct_get_ch(&p1), '.');
	assert_eq(*Punct_get_second_ch(&p1), '.');
	assert_eq(*Punct_get_third_ch(&p1), '.');

	Punct p2 = PUNCT('<');
	assert_eq(*Punct_get_ch(&p2), '<');
	assert_eq(*Punct_get_second_ch(&p2), 0);
	assert_eq(*Punct_get_third_ch(&p2), 0);
}

FamTest(base, test_token) {
	IdentPtr ident = IDENT("abc");
	Token t1 = TOKEN(ident);
	Token t2;
	clone(&t2, &t1);
	assert_eq(*Token_get_ttype(&t1), IdentType);
	assert_eq(*Token_get_ttype(&t2), IdentType);
	IdentPtr t1_out = *Token_get_ident(&t1);
	assert_eq_str(to_str(&t1_out), "abc");
	IdentPtr t2_out = *Token_get_ident(&t2);
	assert_eq_str(to_str(&t2_out), "abc");

	PunctPtr punct = PUNCT('<', '=');
	Token t3 = TOKEN(punct);
	Token t4;
	clone(&t4, &t3);
	assert_eq(*Token_get_ttype(&t3), PunctType);
	assert_eq(*Token_get_ttype(&t4), PunctType);
	PunctPtr t3_out = *Token_get_punct(&t3);
	assert_eq(*Punct_get_ch(&t3_out), '<');
	assert_eq(*Punct_get_second_ch(&t3_out), '=');
	assert_eq(*Punct_get_third_ch(&t3_out), 0);
	PunctPtr t4_out = *Token_get_punct(&t4);
	assert_eq(*Punct_get_ch(&t4_out), '<');
	assert_eq(*Punct_get_second_ch(&t4_out), '=');
	assert_eq(*Punct_get_third_ch(&t4_out), 0);

	LiteralPtr literal = LITERAL(-155023);
	Token t5 = TOKEN(literal);
	Token t6;
	clone(&t6, &t5);
	assert_eq(*Token_get_ttype(&t5), LiteralType);
	assert_eq(*Token_get_ttype(&t6), LiteralType);
	LiteralPtr t5_out = *Token_get_literal(&t5);
	assert_eq_str(to_str(&t5_out), "-155023");
	LiteralPtr t6_out = *Token_get_literal(&t6);
	assert_eq_str(to_str(&t6_out), "-155023");
}
