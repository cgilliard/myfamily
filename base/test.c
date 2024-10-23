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

#include <base/fam_err.h>
#include <base/print_util.h>
#include <base/resources.h>
#include <base/string.h>
#include <criterion/criterion.h>
#include <string.h>

Test(base, test_strcmp) {
	const u8 *test1 = "test";
	const u8 *test2 = "test2";
	const u8 *test3 = "test3";
	const u8 *abc = "abc2";
	const u8 *def = "def1";
	cr_assert(!mystrcmp(test1, test1));
	cr_assert(mystrcmp(test1, test2) < 0 && strcmp(test1, test2) < 0);
	cr_assert(mystrcmp(test2, test1) > 0 && strcmp(test2, test1) > 0);
	cr_assert(mystrcmp(test3, test2) == strcmp(test3, test2));
	cr_assert(mystrcmp(test2, test3) == strcmp(test2, test3));
	cr_assert(mystrcmp(abc, def) < 0 && strcmp(abc, def) < 0);
	cr_assert(mystrcmp(def, abc) > 0 && strcmp(def, abc) > 0);
}

Test(base, test_strlen) {
	u8 *test1 = "alsdfkjl";
	cr_assert_eq(strlen(test1), mystrlen(test1));
	u8 *test2 = "";
	cr_assert_eq(strlen(test2), mystrlen(test2));
}

Test(base, test_strstr) {
	const u8 *s1 = "abc123a";
	const u8 *s2 = "123";
	cr_assert_eq(mystrstr(s1, s2), s1 + 3);
	cr_assert(!mystrcmp(mystrstr(s1, s2), s1 + 3));
	cr_assert(!strcmp(strstr(s1, s2), s1 + 3));

	const u8 *t1 = "abcdef123";
	const u8 *t2 = "123";
	cr_assert(!mystrcmp(mystrstr(t1, t2), t1 + 6));
	cr_assert(!strcmp(strstr(t1, t2), t1 + 6));

	cr_assert(!mystrstr(t1, "xyz"));
}

Test(base, test_strcpy) {
	const u8 *test1 = "cpytext";
	u8 buf1[10];
	for (u8 i = 0; i < 10; i++)
		buf1[i] = 'a';
	buf1[9] = 0;
	mystrcpy(buf1, test1, 9);
	cr_assert(!mystrcmp(buf1, "cpytext"));

	mystrcpy(buf1, "", 9);
	cr_assert(!mystrcmp(buf1, ""));
}

Test(base, test_print_utils) {
	i16 vi16 = 32000;
	println("no args!");
	print("start: u64value={},i16value={},i32value={},", 999ULL, vi16, -111);
	println("str='{}'", "this is a test");
	u8 tmp[101];
	for (int i = 0; i < 101; i++)
		tmp[i] = 0;
	sprint(tmp, 100, "test123 {}", 5);
	println("tmp={}", tmp);
	cr_assert(!mystrcmp(tmp, "test123 5"));
	sprintln(tmp, 100, "testabc {} {}", -4, "ok");
	cr_assert(!mystrcmp(tmp, "testabc -4 ok\n"));
	sprint(tmp, 100, "abc {}", 100000000000ULL);
	cr_assert(!mystrcmp(tmp, "abc 100000000000"));
}

Test(base, test_string) {
	fam_err = NoErrors;
	cr_assert(mymemcmp("abc", NULL, 1));
	cr_assert_eq(fam_err, IllegalArgument);
	cr_assert(!mymemcmp("abc", "def", 0));
	cr_assert(mymemcmp("abc", "def", 3) < 0);
	cr_assert(mymemcmp("def", "abc", 3) > 0);

	cr_assert(!mymemcmp("aaab", "aaa", 3));

	fam_err = NoErrors;
	cr_assert(mystrcmp("abc", NULL));
	cr_assert_eq(fam_err, IllegalArgument);

	fam_err = NoErrors;
	cr_assert(mystrncmp("abc", NULL, 1));
	cr_assert_eq(fam_err, IllegalArgument);
	cr_assert(!mystrncmp("abc", "def", 0));

	cr_assert(!mystrncmp("abc", "aef", 1));

	cr_assert(mystrncmp("abc", "aef", 3) < 0);
	cr_assert(mystrncmp("aef", "abc", 3) > 0);

	cr_assert(mystrncmp("abc", "abcdef", 6) < 0);
	cr_assert(mystrncmp("abcdef", "abc", 6) > 0);
	cr_assert(!mystrncmp("abc", "abcdef", 3));
	fam_err = NoErrors;
	cr_assert(mystrlen(NULL) < 0);
	cr_assert_eq(fam_err, IllegalArgument);

	fam_err = NoErrors;
	__is_debug_mystrlen_overflow = true;
	cr_assert(mystrlen("abc") < 0);
	__is_debug_mystrlen_overflow = false;
	cr_assert_eq(fam_err, Overflow);

	fam_err = NoErrors;
	cr_assert_eq(mystrstr(NULL, NULL), NULL);
	cr_assert_eq(fam_err, IllegalArgument);

	u8 buf[100];
	mystrcpy(buf, "abc", 4);
	println("buf={}", buf);
	cr_assert(!mystrcmp(buf, "abc"));
	mystrcat(buf, "def", 4);
	println("buf={}", buf);
	cr_assert(!mystrcmp(buf, "abcdef"));

	fam_err = NoErrors;
	cr_assert_eq(mystrcat(NULL, NULL, 10), NULL);
	cr_assert_eq(fam_err, IllegalArgument);
	memzero(buf, 100);
	mystrcpy(buf, "ok", 4);
	cr_assert_eq(mystrcat(buf, "abc", 0), buf);
	cr_assert(!mystrcmp(buf, "ok"));
	fam_err = NoErrors;
	cr_assert_eq(mystrcpy(NULL, NULL, 0), NULL);
	cr_assert_eq(fam_err, IllegalArgument);

	cr_assert_eq(mystrcpy(buf, "abc", 0), buf);
	cr_assert(!mystrcmp(buf, "ok"));

	fam_err = NoErrors;
	cr_assert_eq(mymemcpy(NULL, NULL, 10), NULL);
	cr_assert_eq(fam_err, IllegalArgument);

	cr_assert_eq(mymemcpy(buf, "abc", 0), buf);
	cr_assert(!mystrcmp(buf, "ok"));

	cr_assert_eq(mymemcpy(buf, "test", 4), buf);
	buf[4] = 0;
	cr_assert(!mystrcmp(buf, "test"));

	citoau64(0, buf, 10);
	cr_assert(!mystrcmp(buf, "0"));

	citoai64(0, buf, 10);
	cr_assert(!mystrcmp(buf, "0"));
	u8 *teststr = "testabcdeftest";
	cr_assert_eq(rstrstr(teststr, "abc") - teststr, 4);
	cr_assert_eq(rstrstr(teststr, "test") - teststr, mystrlen("testabcdef"));
	cr_assert_eq(mystrstr(teststr, "test") - teststr, 0);

	cr_assert_eq(rstrstr("abc", "abcdef"), NULL);
	cr_assert_eq(rstrstr("abc", "x"), NULL);
	fam_err = NoErrors;
	memzero(NULL, 10);
	cr_assert_eq(fam_err, IllegalArgument);
}
