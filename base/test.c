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
#include <base/formatter.h>
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
	StringRef s11 = *(StringRef *)unwrap(&r11);
	assert_eq_str(unwrap(&s11), "another");
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

	StringRef x1 = STRINGP("my first string");
	StringRef x2;
	StringRef x3 = STRINGP(" more");
	Result xr0 = deep_copy(&x2, &x1);
	Result ar0 = append(&x1, &x3);

	assert_eq_str(to_str(&x1), "my first string more");
	assert_eq_str(to_str(&x2), "my first string");
	printf("CLASS=%s %s %s\n", CLASS_NAME(&x1), CLASS_NAME(&x2),
	       CLASS_NAME(&x3));

	assert_eq_str(CLASS_NAME(&x1), "StringRef");
	assert_eq_str(CLASS_NAME(&x2), "StringRef");
	assert_eq_str(CLASS_NAME(&x3), "StringRef");
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
	Ident ident = IDENT("abc");
	Token t1 = TOKEN(ident);

	printf("1\n");
	Result r0 = to_string(&t1);
	printf("2\n");
	assert(r0.is_ok());
	StringRef s0 = *(StringRef *)unwrap(&r0);
	printf("class=%s\n", CLASS_NAME(&s0));
	printf("s0=%s\n", to_str(&s0));
	Token t2;
	clone(&t2, &t1);
	printf("4\n");
	assert_eq(*Token_get_ttype(&t1), IdentType);
	assert_eq(*Token_get_ttype(&t2), IdentType);
	IdentPtr *t1_out = *Token_get_ident(&t1);
	printf("5\n");
	assert_eq_str(to_str(t1_out), "abc");
	printf("6\n");
	IdentPtr *t2_out = *Token_get_ident(&t2);
	assert_eq_str(to_str(t2_out), "abc");
	printf("6\n");

	Punct punct = PUNCT('<', '=');
	printf("7\n");

	Token t3 = TOKEN(punct);
	printf("7.4\n");
	Result r13 = to_string(&t3);
	printf("8\n");
	assert(r13.is_ok());
	StringRef s13 = *(StringRef *)unwrap(&r13);
	printf("class=%s\n", CLASS_NAME(&s13));
	printf("s13=%s\n", to_str(&s13));
	Token t4;
	clone(&t4, &t3);
	assert_eq(*Token_get_ttype(&t3), PunctType);
	assert_eq(*Token_get_ttype(&t4), PunctType);
	PunctPtr *t3_out = *Token_get_punct(&t3);
	assert_eq(*Punct_get_ch(t3_out), '<');
	assert_eq(*Punct_get_second_ch(t3_out), '=');
	assert_eq(*Punct_get_third_ch(t3_out), 0);
	PunctPtr *t4_out = *Token_get_punct(&t4);
	assert_eq(*Punct_get_ch(t4_out), '<');
	assert_eq(*Punct_get_second_ch(t4_out), '=');
	assert_eq(*Punct_get_third_ch(t4_out), 0);

	Literal literal = LITERAL(-155023);
	Token t5 = TOKEN(literal);
	Result r10 = to_string(&t5);
	assert(r10.is_ok());
	StringRef s10 = *(StringRef *)unwrap(&r10);
	printf("class=%s\n", CLASS_NAME(&s10));
	printf("s10=%s\n", to_str(&s10));
	Token t6;
	clone(&t6, &t5);
	assert_eq(*Token_get_ttype(&t5), LiteralType);
	assert_eq(*Token_get_ttype(&t6), LiteralType);
	LiteralPtr *t5_out = *Token_get_literal(&t5);
	assert_eq_str(to_str(t5_out), "-155023");
	LiteralPtr *t6_out = *Token_get_literal(&t6);
	assert_eq_str(to_str(t6_out), "-155023");
}

FamTest(base, test_tokenizer) {
	StringRef s1 = STRINGP("this is a test");
	Result r1 = Tokenizer_parse(&s1);
	assert(r1.is_ok());
	Tokenizer t1 = *(Tokenizer *)unwrap(&r1);

	while (true) {
		Result next = Tokenizer_next_token(&t1);
		printf("got next\n");
		assert(next.is_ok());
		Option opt = *(Option *)unwrap(&next);
		if (!opt.is_some())
			break;

		Token token = *(Token *)unwrap(&opt);
		Result nt = to_string(&token);
		StringRef nsr = *(StringRef *)unwrap(&nt);
		printf("next token ret: %s\n", to_str(&nsr));
		cleanup(&next);
	}
}

CLASS(LoopTest, FIELD(char *, my_ptr))
IMPL(LoopTest, TRAIT_COPY)
#define LoopTest DEFINE_CLASS(LoopTest)
void LoopTest_cleanup(LoopTest *ptr) {
	if (ptr->_my_ptr) {
		tlfree(ptr->_my_ptr);
		ptr->_my_ptr = NULL;
	}
}
bool LoopTest_clone(LoopTest *dst, LoopTest *src) { return true; }
usize LoopTest_size(LoopTest *obj) { return sizeof(LoopTest); }

FamTest(base, test_loop) {
	int counter = 0;
	while (counter < 1000) {
		printf("loop %i\n", counter);
		// printf("1\n");
		Ident ident = IDENT("abc");
		Token t = TOKEN(ident);
		// Literal ident = LITERAL("\"def\"");
		// Punct ident = PUNCT('>');
		// printf("2\n");
		// StringRef t = STRINGP("def");
		//  u64 t = counter;

		// char *my_ptr = tlmalloc(sizeof(char) * 100);
		// strcpy(my_ptr, "test");
		// LoopTest t1 = BUILD(LoopTest, my_ptr);
		//  printf("3\n");
		Option o = Some(t);
		// printf("4\n");
		Result r = Ok(o);
		// printf("5\n");
		//	Option o_out = *(Option *)unwrap(&r);
		// Token t_out = *(Token *)unwrap(&o_out);
		//  printf("6\n");
		//	Token t_out = *(Token *)unwrap(&o_out);
		// Result r_out = to_string(&t_out);
		// StringRef sr_out = *(StringRef *)unwrap(&o_out);
		// printf("sr=%s\n", to_str(&sr_out));
		// assert_eq_str(to_str(&sr_out), "def");
		// assert_eq(t_out, counter);
		//     Token t_out = *(Token *)unwrap(&o_out);
		//     Result r_out = to_string(&t_out);

		// cleanup(tcopy);
		// cleanup(icopy);
		// cleanup(tcopy);
		counter += 1;
		// printf("7\n");
	}
}

FamTest(base, test_option_res_etc) {
	Ident ident = IDENT("test");
	Token token = TOKEN(ident);
	Option opt = Some(token);
	Result rx = Ok(opt);
}

FamTest(base, test_formatter) {
	char buf[1024];
	Formatter fmt = BUILD(Formatter, buf, 1024, 0);
	Result r = Formatter_write(&fmt, "%i - %s", 10, "hi there");
	assert(r.is_ok());
	Result r2 = Formatter_to_str_ref(&fmt);
	StringRef ref = *(StringRef *)unwrap(&r2);
	assert_eq_str(to_str(&ref), "10 - hi there");

	Result r3 = Formatter_write(&fmt, " done.");
	Result r4 = Formatter_to_str_ref(&fmt);
	StringRef ref2 = *(StringRef *)unwrap(&r4);
	assert_eq_str(to_str(&ref2), "10 - hi there done.");

	char buf2[10];
	Formatter fmt2 = BUILD(Formatter, buf2, 10, 0);
	Result r5 = Formatter_write(&fmt2, "012345678901234");
	assert(!r5.is_ok());
	Error e5 = unwrap_err(&r5);
	print(&e5);
}

