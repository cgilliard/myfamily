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

#include <base/misc.h>
#include <base/panic.h>
#include <base/print_util.h>
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
	cr_assert(!mystrcmp(tmp, "test123 5"));
	sprintln(tmp, 100, "testabc {} {}", -4, "ok");
	cr_assert(!mystrcmp(tmp, "testabc -4 ok\n"));
	sprint(tmp, 100, "abc {}", 100000000000ULL);
	cr_assert(!mystrcmp(tmp, "abc 100000000000"));
}
