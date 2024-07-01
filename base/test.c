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

#include <base/ekinds.h>
#include <base/error.h>
#include <base/file.h>
#include <base/formatter.h>
#include <base/option.h>
#include <base/result.h>
#include <base/string.h>
#include <base/test.h>
#include <base/tlmalloc.h>
#include <base/tokenizer.h>
#include <base/tuple.h>
#include <base/unit.h>
#include <limits.h>

FamSuite(base);

FamTest(base, test_tmalloc) { return Ok(UNIT); }

FamTest(base, test_result_all) {
	u64 v1 = 31;
	Result r1 = Ok(v1);
	assert(r1.is_ok());
	printf("y\n");
	U64 v1_out = Try3(U64, r1);
	printf("x\n");
	assert_eq(*(u64 *)unwrap(&v1_out), 31);

	/*
	i8 v2 = 32;
	Result r2 = Ok(v2);
	assert(r2.is_ok());
	i8 v2_out = Try3(v2_out, r2);
	assert_eq(v2_out, 32);

	StringRef v3 = STRINGP("abc");
	Result r3 = Ok(v3);
	assert(r3.is_ok());
	// StringRef v3_out = *(StringRef *)Try3(v3_out, r3);
	StringRef v3_out = Expect(StringRef, r3);
	printf("ok\n");
	// assert_eq_str(to_str(&v3_out), "abc");
	*/

	return Ok(UNIT);
}

FamTest(base, test_result) {
	/*
	StringRef xx1 = STRINGP("test");
	Result xxr1 = Ok(xx1);
	StringRefPtr xx1_out = *(StringRef *)Try3(xx1_out, xxr1);
	assert_eq_str(to_str(&xx1_out), "test");

	Result yr1 = FOPEN("./resources/abc.txt", OpenRead);
	File yf1 = *(File *)Try3(yf1, yr1);

	Result r11 = read_to_string(&yf1);
	StringRef s1 = Expect(StringRef, r11);
	assert_eq_str(to_str(&s1), "this is a test\n");

	// prim
	u64 v1 = 31;
	Result r1 = Ok(v1);
	assert(r1.is_ok());
	u64 v1_out = Try3(v1_out, r1);
	assert_eq(v1_out, 31);
	*/

	// prim again
	i128 v2 = -10;
	Result r2 = Ok(v2);
	assert(r2.is_ok());
	i128 v2_out = Try(i128, r2);
	assert_eq(v2_out, -10);

	// pointers
	U64Ptr *v3 = tlmalloc(sizeof(U64));
	BUILDPTR(v3, U64);
	v3->_value = 101;
	Rc rc3 = RC(v3);
	Result r3 = Ok(rc3);
	U64 v3_out = Try(U64, r3);
	assert_eq(v3_out._value, 101);

	// prim with pre-proc equiv
	u64 v4 = UINT64_MAX;
	Result r4 = Ok(v4);
	assert(r4.is_ok());
	u64 v4_out = Try(u64, r4);
	assert_eq(v4_out, UINT64_MAX);

	// Preprocessed equiv: (prims are auto created)
	/*
	Result<u64> myfun() {
		Ok(UINT64_MAX)
	}
	u64 v5_out = myfun()?;
	assert_eq(v5_out, UINT64_MAX);

	Generated code:
	Result myfun() {
		u64 __generated_v1 = UINT64_MAX;
		return Ok(__generated_v1);
	}
	Result __generated_v2 = myfun();
	u64 v5_out = Try(u64, __generated_v2);
	assert_eq(v5_out, UINT64_MAX);

	*/

	usize x = 1234;
	Result x1 = Ok(x);
	assert(x1.is_ok());
	usize x_out = Try(usize, x1);
	assert_eq(x_out, 1234);

	Option opt1 = Some(x);
	assert(opt1.is_some());
	usize x_out2 = *(usize *)unwrap(&opt1);
	assert_eq(x_out2, 1234);

	return Ok(UNIT);
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
	return Ok(UNIT);
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

	Result r11 = SUBSTRING(s3, 0, 7);
	RcPtr *rc11 = unwrap(&r11);
	StringPtr *s11 = unwrap(rc11);
	assert_eq_str(unwrap(s11), "another");

	cleanup(rc11);

	cleanup(s1);
	tlfree(s1);

	cleanup(s5);
	tlfree(s5);
	return Ok(UNIT);
}

FamTest(base, test_string_ref) {
	StringRef s1 = STRINGP("this is a test");
	Result r2 = STRING("another string");
	StringRef s2 = Try(StringRef, r2);

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
	i64 res = Try(i64, r4);
	assert_eq(res, 14);

	StringRef s5 = STRINGP("i");
	Result r5 = INDEX_OF(&s3, &s5);
	i64 res5 = Try(i64, r5);
	assert_eq(res5, 11);

	Result r6 = LAST_INDEX_OF(&s3, &s5);
	i64 res6 = Try(i64, r6);
	assert_eq(res6, 19);

	Result r7 = LAST_INDEX_OF(&s3, "abababaa");
	i64 res7 = Try(i64, r7);
	assert_eq(res7, -1);

	Result r8 = char_at(&s3, 0);
	signed char ch8 = Try(signed char, r8);
	assert_eq(ch8, 'a');
	Result r9 = char_at(&s3, 2);
	signed char ch9 = Try(signed char, r9);
	assert_eq(ch9, 'o');

	Result r10 = char_at(&s3, 10000);
	assert(!r10.is_ok());

	Result r11 = SUBSTRING(s3, 0, 7);
	StringRef s11 = Try(StringRef, r11);
	assert_eq_str(unwrap(&s11), "another");

	/* from version:
	 StringRef s11 = SUBSTRING(s3, 0, 7)?;
	 assert_eq_str(s11, "another");
	 */

	StringRef abc1 = STRINGP("abc");
	{
		StringRef abc2;
		bool cr = clone(&abc2, &abc1);
		assert(cr);
		assert_eq_str(to_str(&abc1), "abc");
		assert_eq_str(to_str(&abc2), "abc");
	}
	assert_eq_str(to_str(&abc1), "abc");

	StringRef def1;
	{
		StringRef def2 = STRINGP("def");
		bool cr = clone(&def1, &def2);
		assert(cr);
		assert_eq_str(to_str(&def1), "def");
		assert_eq_str(to_str(&def2), "def");
	}

	assert_eq_str(to_str(&def1), "def");

	Option opt1 = Some(def1);
	Option opt2 = Some(opt1);
	Option opt3 = Some(opt2);
	Option opt4 = Some(opt3);
	Option opt3_out = *(Option *)unwrap(&opt4);
	Option opt2_out = *(Option *)unwrap(&opt3_out);
	Option opt1_out = *(Option *)unwrap(&opt2_out);
	StringRef def1_out = *(StringRef *)unwrap(&opt1_out);
	assert_eq_str(to_str(&def1_out), "def");

	StringRef dc;
	Result dcr = deep_copy(&dc, &def1);
	assert(dcr.is_ok());

	Result arr = append(&def1, &abc1);
	assert(arr.is_ok());
	assert_eq_str(to_str(&def1), "defabc");
	assert_eq_str(to_str(&dc), "def");
	return Ok(UNIT);
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
	return Ok(UNIT);
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
	return Ok(UNIT);
}

FamTest(base, test_token) {
	Token t1 = TOKEN(IdentType, "abc");

	Result r0 = to_string(&t1);
	assert(r0.is_ok());
	StringRef s0 = *(StringRef *)unwrap(&r0);
	Token t2;
	clone(&t2, &t1);
	assert_eq(*Token_get_ttype(&t1), IdentType);
	assert_eq(*Token_get_ttype(&t2), IdentType);
	Result r1_out = to_debug(&t1);
	StringRef t1_out = Expect(StringRef, r1_out);
	assert_eq_str(to_str(&t1_out), "abc");
	Result r2_out = to_debug(&t2);
	StringRef t2_out = Expect(StringRef, r2_out);
	assert_eq_str(to_str(&t2_out), "abc");

	Token t3 = TOKEN(PunctType, "<=");
	Result r13 = to_string(&t3);
	assert(r13.is_ok());
	StringRef s13 = *(StringRef *)unwrap(&r13);
	Token t4;
	clone(&t4, &t3);
	assert_eq(*Token_get_ttype(&t3), PunctType);
	assert_eq(*Token_get_ttype(&t4), PunctType);
	Result r3_out = to_debug(&t3);
	StringRef t3_out = Expect(StringRef, r3_out);
	assert_eq_str(to_str(&t3_out), "<=");
	Result r4_out = to_debug(&t4);
	StringRef t4_out = Expect(StringRef, r4_out);
	assert_eq_str(to_str(&t4_out), "<=");

	return Ok(UNIT);
}

Result expect_tokens(char *str, TokenType *type_expects,
		     char **token_str_expects, int expect_count,
		     bool in_comment_end, bool in_comment_start) {
	StringRef s = STRINGP(str);
	Result r = Tokenizer_parse(&s);
	Tokenizer t = Try(Tokenizer, r);
	Tokenizer_set_in_comment(&t, in_comment_start);
	int count = 0;

	while (true) {
		Result next = NEXT_TOKEN(t);
		Option opt = Try(Option, next);
		if (!opt.is_some())
			break;
		Token token = *(Token *)unwrap(&opt);
		Result token_str_r = TOKEN_STR(token);
		StringRef token_str = Expect(StringRef, token_str_r);
		Result nt = to_string(&token);
		StringRef nsr = Expect(StringRef, nt);
		printf("Token[%i]: %s\n", count, to_str(&nsr));
		assert_eq(TOKEN_TYPE(token), type_expects[count]);
		assert_eq_str(to_str(&token_str), token_str_expects[count]);

		count += 1;
	}
	assert_eq(*Tokenizer_get_in_comment(&t), in_comment_end);
	assert_eq(count, expect_count);

	return Ok(UNIT);
}

FamTest(base, test_tokenizer) {
	printf("t1:\n");
	TokenType exp_tt1[] = {IdentType, IdentType, IdentType, IdentType};
	char *exp_tstr1[] = {"this", "is", "a", "test"};
	Result r1 = expect_tokens("this is a test", exp_tt1, exp_tstr1, 4,
				  false, false);

	printf("t2:\n");
	TokenType exp_tt2[] = {IdentType, IdentType, PunctType, LiteralType,
			       PunctType};
	char *exp_tstr2[] = {"u64", "x", "=", "0", ";"};
	Result r2 =
	    expect_tokens("u64 x = 0;", exp_tt2, exp_tstr2, 5, false, false);

	printf("t3:\n");
	TokenType exp_tt3[] = {IdentType, PunctType,   IdentType,
			       PunctType, LiteralType, PunctType};
	char *exp_tstr3[] = {"char", "*", "x9_bac", "=", "\"abc123\"", ";"};
	Result r3 = expect_tokens("char *x9_bac = \"abc123\";", exp_tt3,
				  exp_tstr3, 6, false, false);

	printf("t4:\n");
	TokenType exp_tt4[] = {IdentType,   PunctType, IdentType,   PunctType,
			       LiteralType, PunctType, PunctType,   IdentType,
			       LiteralType, PunctType, LiteralType, PunctType};
	char *exp_tstr4[] = {"if", "(",	     "abc",	 "!=", "7",	")",
			     "{",  "return", "\"test\"", "+",  "\'x\'", "}"};
	Result r4 = expect_tokens("if(abc!= 7) { return \"test\" + \'x\'}",
				  exp_tt4, exp_tstr4, 12, false, false);

	printf("t5\n");
	TokenType exp_tt5[] = {IdentType,   PunctType, IdentType, PunctType,
			       LiteralType, PunctType, IdentType, PunctType};
	char *exp_tstr5[] = {"if", "(", "x", "=", "745", ")", "return", ";"};
	Result r5 = expect_tokens("if\n\n\t(x=745) return;\t\n", exp_tt5,
				  exp_tstr5, 8, false, false);

	printf("t6\n");
	TokenType exp_tt6[] = {IdentType,   PunctType, IdentType, PunctType,
			       LiteralType, PunctType, IdentType, PunctType};
	char *exp_tstr6[] = {"if", "(", "x", "=", "745", ")", "return", ";"};
	Result r6 = expect_tokens("if\n\n\t(x=745) return; ", exp_tt6,
				  exp_tstr6, 8, false, false);

	printf("t7\n");
	TokenType exp_tt7[] = {IdentType, PunctType, IdentType};
	char *exp_tstr7[] = {"abc", "=", "def"};
	Result r7 =
	    expect_tokens("abc = def", exp_tt7, exp_tstr7, 3, false, false);

	printf("t8\n");
	TokenType exp_tt8[] = {IdentType, PunctType, IdentType};
	char *exp_tstr8[] = {"abc", "=", "def"};
	Result r8 =
	    expect_tokens("abc = def\n ", exp_tt8, exp_tstr8, 3, false, false);

	printf("t9\n");
	TokenType exp_tt9[] = {PunctType, PunctType, PunctType};
	char *exp_tstr9[] = {"<=", ">=", "..."};
	Result r9 =
	    expect_tokens("<=>=...\n ", exp_tt9, exp_tstr9, 3, false, false);

	printf("t10\n");
	TokenType exp_tt10[] = {IdentType, PunctType, PunctType, IdentType,
				PunctType};
	char *exp_tstr10[] = {"x", ">=", "...", "hi", ";"};
	Result r10 = expect_tokens("x>=... // ok this is a test  \t\n  hi ; ",
				   exp_tt10, exp_tstr10, 5, false, false);

	printf("t11\n");
	TokenType exp_tt11[] = {IdentType, PunctType, PunctType, IdentType,
				PunctType};
	char *exp_tstr11[] = {"x", ">=", "...", "hi", ";"};
	Result r11 = expect_tokens("x>=... /* ok this is a test  \t\n  */ hi; ",
				   exp_tt11, exp_tstr11, 5, false, false);

	printf("t12\n");
	TokenType exp_tt12[] = {IdentType, PunctType, PunctType, IdentType,
				PunctType};
	char *exp_tstr12[] = {"x", ">=", "...", "hi", ";"};
	Result r12 = expect_tokens("x>=... /**/ hi ; ", exp_tt12, exp_tstr12, 5,
				   false, false);

	printf("t13\n");
	// use same exp as last time
	Result r13 = expect_tokens("x>=... /* */ hi ; ", exp_tt12, exp_tstr12,
				   5, false, false);

	// end on a comment
	printf("t14\n");
	// use same exp as last time
	Result r14 = expect_tokens("x>=...  hi ;  /*      ", exp_tt12,
				   exp_tstr12, 5, true, false);

	// end on a comment
	printf("t15\n");
	// use same exp as last time
	Result r15 = expect_tokens("x>=...  hi ;  // testing     ", exp_tt12,
				   exp_tstr12, 5, false, false);

	printf("t16\n");
	TokenType exp_tt16[] = {IdentType, PunctType, PunctType,
				DocType,   IdentType, PunctType};
	char *exp_tstr16[] = {"x", ">=", "...", "ok this is a test", "hi", ";"};
	Result r16 = expect_tokens("x>=... /// ok this is a test\n   hi ; ",
				   exp_tt16, exp_tstr16, 6, false, false);

	printf("t17\n");
	TokenType exp_tt17[] = {IdentType, PunctType, PunctType,
				DocType,   IdentType, PunctType};
	char *exp_tstr17[] = {"x", ">=", "...", "", "hi", ";"};
	Result r17 = expect_tokens("x>=... ///\n   hi ; ", exp_tt17, exp_tstr17,
				   6, false, false);

	printf("t18\n");
	// use 17 exp again
	Result r18 = expect_tokens("x>=... /// \n   hi ; ", exp_tt17,
				   exp_tstr17, 6, false, false);

	printf("t19\n");
	// use 17 exp again
	Result r19 = expect_tokens("x>=... ///\t\n   hi ; ", exp_tt17,
				   exp_tstr17, 6, false, false);

	printf("t20\n");
	TokenType exp_tt20[] = {IdentType, PunctType, PunctType,
				DocType,   IdentType, PunctType};
	char *exp_tstr20[] = {"x", ">=", "...", "x", "hi", ";"};
	Result r20 = expect_tokens("x>=... ///x\n   hi ; ", exp_tt20,
				   exp_tstr20, 6, false, false);

	printf("t21\n");
	TokenType exp_tt21[] = {IdentType, PunctType};
	char *exp_tstr21[] = {"hi", ";"};
	Result r21 =
	    expect_tokens(" hi ; /*", exp_tt21, exp_tstr21, 2, true, false);

	printf("t22\n");
	TokenType exp_tt22[] = {IdentType, PunctType};
	char *exp_tstr22[] = {"hi", ";"};
	Result r22 = expect_tokens(" a b c 10 \"abc\" */ hi ; ", exp_tt22,
				   exp_tstr22, 2, false, true);

	Expect(Unit, r1);
	Expect(Unit, r2);
	Expect(Unit, r3);
	Expect(Unit, r4);
	Expect(Unit, r5);

	Expect(Unit, r6);
	Expect(Unit, r7);
	Expect(Unit, r8);
	Expect(Unit, r9);
	Expect(Unit, r10);

	Expect(Unit, r11);
	Expect(Unit, r12);
	Expect(Unit, r13);
	Expect(Unit, r14);
	Expect(Unit, r15);

	Expect(Unit, r16);
	Expect(Unit, r17);
	Expect(Unit, r18);
	Expect(Unit, r19);
	Expect(Unit, r20);

	Expect(Unit, r21);
	Expect(Unit, r22);

	return Ok(UNIT);
}

FamTest(base, test_option_res_etc) {
	Token token = TOKEN(DocType, "doctest");
	Option opt = Some(token);
	Result rx = Ok(opt);
	return Ok(UNIT);
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
	return Ok(UNIT);
}

FamTest(base, test_unwrap_as) {
	StringRef test1 = STRINGP("test unwrap_as");
	Result r1 = Ok(test1);
	StringRef test1_out = Expect(StringRef, r1);
	assert_eq_str(to_str(&test1_out), "test unwrap_as");

	Result r = to_debug(&UNIT);
	StringRef ref = Expect(StringRef, r);
	char *tmp = to_str(&ref);
	assert_eq_str(tmp, "\"()\"");

	Option opt = Some(UNIT);
	Result r2 = to_debug(&opt);
	StringRef ref2 = Expect(StringRef, r2);
	tmp = to_str(&ref2);
	assert_eq_str(tmp, "Option[\"()\"]");

	return Ok(UNIT);
}

#define UINT128_MAX ((unsigned __int128)(-1))
#define INT128_MAX ((signed __int128)(UINT128_MAX >> 1))
#define INT128_MIN ((signed __int128)(-INT128_MAX - 1))

FamTest(base, test_tuple) {
	i128 i = 1;
	i32 x = 2;
	u64 uu = 3;
	Unit u = UNIT;
	StringRef vv = STRINGP("this is a test");
	i32 x2 = 10;
	char *okx = "okokok";
	Tuple t = TUPLE(u, vv, okx, x2, i, uu);
	assert_eq(len(&t), 6);
	i32 x2_out;
	ELEMENT_AT(&t, 3, &x2_out);
	assert_eq(x2_out, 10);
	char ok_out[100];
	ELEMENT_AT(&t, 2, ok_out);
	assert_eq_str(ok_out, "okokok");
	char vv_out[100];
	ELEMENT_AT(&t, 1, vv_out);
	assert_eq_str(vv_out, "this is a test");
	Error e = ERROR(ALLOC_ERROR, "test errors");
	Tuple t2 = TUPLE(e);
	Error e_out;
	ELEMENT_AT(&t2, 0, &e_out);
	assert(equal(KIND(e_out), &ALLOC_ERROR));
	i8 i8_1v = 1;
	i16 i16_1v = 2;
	i32 i32_1v = 1;
	i64 i64_1v = 2;
	i128 i128_1v = 2;
	u8 u8_1v = 1;
	u16 u16_1v = 2;
	u32 u32_1v = 1;
	u64 u64_1v = 2;
	u128 u128_1v = 2;
	bool bool_1v = false;
	char *str_1v = "12345";
	Tuple t3 = TUPLE(i8_1v, i16_1v, i32_1v, i64_1v, i128_1v, u8_1v, u16_1v,
			 u32_1v, u64_1v, u128_1v, bool_1v, str_1v);

	i8 i8_1v_out = 0;
	i16 i16_1v_out = 0;
	i32 i32_1v_out = 0;
	i64 i64_1v_out = 0;
	i128 i128_1v_out = 0;
	u8 u8_1v_out = 0;
	u16 u16_1v_out = 0;
	u32 u32_1v_out = 0;
	u64 u64_1v_out = 0;
	u128 u128_1v_out = 0;
	bool bool_1v_out = true;
	char str_1v_out[100];

	ELEMENT_AT(&t3, 0, &i8_1v_out);
	assert_eq(i8_1v_out, i8_1v);

	ELEMENT_AT(&t3, 1, &i16_1v_out);
	assert_eq(i16_1v_out, i16_1v);

	ELEMENT_AT(&t3, 2, &i32_1v_out);
	assert_eq(i32_1v_out, i32_1v);

	ELEMENT_AT(&t3, 3, &i64_1v_out);
	assert_eq(i64_1v_out, i64_1v);

	ELEMENT_AT(&t3, 4, &i128_1v_out);
	assert_eq(i128_1v_out, i128_1v);

	ELEMENT_AT(&t3, 5, &u8_1v_out);
	assert_eq(u8_1v_out, u8_1v);

	ELEMENT_AT(&t3, 6, &u16_1v_out);
	assert_eq(u16_1v_out, u16_1v);

	ELEMENT_AT(&t3, 7, &u32_1v_out);
	assert_eq(u32_1v_out, u32_1v);

	ELEMENT_AT(&t3, 8, &u64_1v_out);
	assert_eq(u64_1v_out, u64_1v);

	ELEMENT_AT(&t3, 9, &u128_1v_out);
	assert_eq(u128_1v_out, u128_1v);

	ELEMENT_AT(&t3, 10, &bool_1v_out);
	assert_eq(bool_1v_out, bool_1v);

	ELEMENT_AT(&t3, 11, str_1v_out);
	assert_eq_str(str_1v_out, "12345");

	i128 c1_v1 = -100;
	char *c1_v2 = "this is a test2";
	u32 c1_v3 = 37;
	Tuple c1 = TUPLE(c1_v1, c1_v2, c1_v3);
	Tuple c2;
	copy(&c2, &c1);
	u32 c1_v3_out = 0;
	ELEMENT_AT(&c2, 2, &c1_v3_out);
	assert_eq(c1_v3_out, 37);

	char *str = "test";
	char *str2 = "ok";
	char *str3 = "another";
	u64 v64 = 101;
	u32 v32 = 102;
	i32 vi32 = -100;
	bool vbool1 = false;
	bool vbool2 = true;

	Tuple cc = TUPLE(str, str2, str3, v64, v32, vi32, vbool1, vbool2);
	char str_out[100];
	ELEMENT_AT(&cc, 0, &str_out);
	assert_eq_str(str_out, "test");

	Result debug = to_debug(&cc);
	StringRef dbg = Expect(StringRef, debug);
	printf("debug=%s\n", to_str(&dbg));

	u8 display_u8_max = UCHAR_MAX;
	i8 display_i8_min = SCHAR_MIN;
	i8 display_i8_max = SCHAR_MAX;
	u16 display_u16_max = USHRT_MAX;
	i16 display_i16_min = SHRT_MIN;
	i16 display_i16_max = SHRT_MAX;
	u32 display_u32_max = UINT_MAX;
	i32 display_i32_min = INT_MIN;
	i32 display_i32_max = INT_MAX;
	u64 display_u64_max = ULLONG_MAX;
	i64 display_i64_min = LLONG_MIN;
	i64 display_i64_max = LLONG_MAX;
	u128 display_u128_max = UINT128_MAX;
	i128 display_i128_min = INT128_MIN;
	i128 display_i128_max = INT128_MAX;
	bool display_bool_false = false;
	bool display_bool_true = true;

	Tuple test_display = TUPLE(
	    display_u8_max, display_i8_min, display_i8_max, display_u16_max,
	    display_i16_min, display_i16_max, display_u32_max, display_i32_min,
	    display_i32_max, display_u64_max, display_i64_min, display_i64_max);
	Result debug2 = to_debug(&test_display);
	StringRef dbg2 = Expect(StringRef, debug2);
	printf("debug2=%s\n", to_str(&dbg2));
	assert_eq_str("Tuple(255, -128, 127, 65535, -32768, 32767, 4294967295, "
		      "-2147483648, 2147483647, 18446744073709551615, "
		      "-9223372036854775808, 9223372036854775807)",
		      to_str(&dbg2))

	    Tuple test_display2 =
		TUPLE(display_u128_max, display_i128_min, display_i128_max,
		      display_bool_false, display_bool_true);
	Result debug3 = to_debug(&test_display2);
	StringRef dbg3 = Expect(StringRef, debug3);
	printf("debug3=%s\n", to_str(&dbg3));

	u128 vvux = UINT128_MAX;
	i128 vvixo = INT128_MAX;
	i128 vvix = 1;
	i128 vvim = 0;
	u128 vvsome = 1234;
	i128 vvimo = INT128_MIN;
	Tuple test = TUPLE(vvux, vvix, vvim, vvsome, vvixo, vvimo);
	Result debugx = to_debug(&test);
	StringRef dbgx = Expect(StringRef, debugx);
	printf("debug=%s\n", to_str(&dbgx));
	// TODO: implement printing of large i128/u128
	assert_eq_str(to_str(&dbgx), "Tuple(U128, 1, 0, 1234, I128, I128)");

	return Ok(UNIT);
}

FamTest(base, test_file) {
	Result r1 = FOPEN("./resources/abc.txt", OpenRead);
	File f1 = Expect(File, r1);

	Result r11 = read_to_string(&f1);
	StringRef s1 = Expect(StringRef, r11);
	assert_eq_str(to_str(&s1), "this is a test\n");

	Result r2 = FOPEN("./resources/abc.txt", OpenRead);
	File f2 = Expect(File, r2);
	char buf[3];
	Result r22 = myread(&f2, buf, 2);
	u64 bytes = *(u64 *)unwrap(&r22);
	assert_eq(bytes, 2);
	buf[2] = 0;
	assert_eq_str(buf, "th");

	Result r23 = myread(&f2, buf, 2);
	bytes = *(u64 *)unwrap(&r23);
	assert_eq(bytes, 2);
	buf[2] = 0;
	assert_eq_str(buf, "is");

	Result r24 = read_exact(&f2, buf, 2);
	assert(r24.is_ok());
	buf[2] = 0;
	assert_eq_str(buf, " i");

	Result r26 = myseek(&f2, 0);
	Expect(Unit, r26);

	Result r27 = myread(&f2, buf, 2);
	bytes = *(u64 *)unwrap(&r27);
	assert_eq(bytes, 2);
	buf[2] = 0;
	assert_eq_str(buf, "th");

	char buf2[1000];
	Result r25 = read_exact(&f2, buf2, 1000);
	assert(!r25.is_ok());

	Result r40 = myseek(&f2, 0);
	Expect(Unit, r40);

	Result r41 = myread(&f2, buf, 2);
	bytes = *(u64 *)unwrap(&r41);
	assert_eq(bytes, 2);
	buf[2] = 0;
	assert_eq_str(buf, "th");

	Result r3 = FOPEN("./resources/not_there.txt", OpenRead);
	assert(!r3.is_ok());

	return Ok(UNIT);
}

FamTest(base, test_file_write) {
	Result r1 = FOPEN("./resources/tmpfile.txt", OpenWrite);
	assert(r1.is_ok());
	File f1 = Expect(File, r1);
	return Ok(UNIT);
}

Result test_try2() {
	Result r1 = STRING("this is a test");
	StringRef s1 = Try2(StringRef, r1);
	assert_eq_str(to_str(&s1), "this is a test");

	return Ok(UNIT);
}

FamTest(base, test_try2) {
	Result r1 = test_try2();
	assert(r1.is_ok());

	return Ok(UNIT);
}
