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
#include <base/test.h>
// #include <criterion/criterion.h>
#include <limits.h>
#include <string.h>

MySuite(base);

MyTest(base, test_strcmp) {
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

MyTest(base, test_strlen) {
	u8 *test1 = "alsdfkjl";
	cr_assert_eq(strlen(test1), mystrlen(test1));
	u8 *test2 = "";
	cr_assert_eq(strlen(test2), mystrlen(test2));
}

MyTest(base, test_strstr) {
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

MyTest(base, test_strcpy) {
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

MyTest(base, test_string) {
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
	cr_assert(!mystrcmp(buf, "abc"));
	mystrcat(buf, "def", 4);
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

MyTest(base, test_print_util) {
	u8 tmp[101];
	i16 vi16 = 32000;
	println("no args!");
	print("start: u64value={},i16value={},i32value={},", 999ULL, vi16, -111);
	println("str='{}'", "this is a test");
	for (int i = 0; i < 101; i++)
		tmp[i] = 0;
	sprint(tmp, 100, "test123 {}", 5);
	cr_assert(!mystrcmp(tmp, "test123 5"));
	sprintln(tmp, 100, "testabc {} {}", -4, "ok");
	cr_assert(!mystrcmp(tmp, "testabc -4 ok\n"));
	i32 ret = sprint(tmp, 100, "abc {}", 100000000000ULL);
	cr_assert(!mystrcmp(tmp, "abc 100000000000"));
	cr_assert_eq(ret, mystrlen("abc 100000000000"));
	SetErr(NoErrors);
	cr_assert_eq(sprint(tmp, -1, "abc"), -1);
	cr_assert_eq(fam_err, IllegalArgument);
	int ret2 = slen("abc {} {} {}", 1000ULL, -80, "test");
	cr_assert_eq(ret2, mystrlen("abc 1000 -80 test"));

	mystrcpy(tmp, "", 1);
	cr_assert_eq(mystrlen(tmp), 0);
	print_impl(out_strm, tmp, 100, false, false, 0, "prefix", "test");
	cr_assert(!mystrcmp(tmp, "prefixtest"));

	int ret3 = sprint(tmp, 100, "abc {} {}", "ok");
	cr_assert(!mystrcmp(tmp, "abc ok {}"));
	cr_assert_eq(ret3, mystrlen("abc ok {}"));
}

MyTest(base, test_path) {
	char buf[PATH_MAX];
	Path home_test;
	path_for(&home_test, "~");
	path_canonicalize(&home_test);

	cr_assert(path_for(NULL, NULL));

	Path len0;
	cr_assert(path_for(&len0, ""));

	cr_assert(path_for(&len0, NULL));

	Path file;
	__is_debug_malloc = true;
	cr_assert(path_for(&file, "test.txt"));
	__is_debug_malloc = false;

	Path home_realloc_err;
	cr_assert(!path_for(&home_realloc_err, "~"));
	__is_debug_realloc = true;
	cr_assert(path_push(&home_realloc_err, "test"));
	cr_assert(path_canonicalize(&home_realloc_err));
	__is_debug_realloc = false;

	cr_assert(path_push(NULL, NULL));
	Path test, test2;
	path_for(&test, "~");
	path_for(&test2, "~");
	cr_assert(path_push(&test, NULL));
	cr_assert(path_push(&test2, ""));
	Path pop_test1;
	path_copy(&pop_test1, test_dir);
	path_push(&pop_test1, "test1");
	strcpy(buf, path_to_string(&pop_test1));
	path_push(&pop_test1, "test2");
	cr_assert(!path_is_dir(&pop_test1));
	path_pop(&pop_test1);
	cr_assert(!strcmp(buf, path_to_string(&pop_test1)));
	path_pop(&pop_test1);
	path_pop(&pop_test1);
	cr_assert(!path_file_name(&pop_test1));

	// all these fail due to invalid input
	Path test_name = {.ptr = NULL, .len = 0};
	cr_assert(!path_exists(&test_name));
	cr_assert(!path_is_dir(&test_name));
	cr_assert(!path_mkdir(&test_name, 0700, false));
	path_copy(&test_name, test_dir);
	// succeed because it already exists
	cr_assert(path_mkdir(&test_name, 0700, false));
	path_push(&test_name, "test.txt");
	cr_assert(!strcmp("test.txt", path_file_name(&test_name)));

	// try to make a directory
	Path mkdir_test1;
	path_copy(&mkdir_test1, test_dir);
	path_push(&mkdir_test1, "ndir1");
	cr_assert(path_mkdir(&mkdir_test1, 0700, false));

	// this will fail because parent is false
	Path mkdir_test2;
	path_copy(&mkdir_test2, test_dir);
	path_push(&mkdir_test2, "ndir2");
	path_push(&mkdir_test2, "2");
	cr_assert(!path_mkdir(&mkdir_test2, 0700, false));

	Path mkdir_test3;
	path_copy(&mkdir_test3, test_dir);
	path_push(&mkdir_test3, "ndir3");
	path_push(&mkdir_test3, "3");
	path_push(&mkdir_test3, "4");
	path_push(&mkdir_test3, "5");
	cr_assert(!path_exists(&mkdir_test3));
	cr_assert(!path_is_dir(&mkdir_test3));
	cr_assert(path_mkdir(&mkdir_test3, 0700, true));
	cr_assert(path_exists(&mkdir_test3));
	cr_assert(path_is_dir(&mkdir_test3));

	Path mkdir_test4;
	path_copy(&mkdir_test4, test_dir);
	path_push(&mkdir_test4, "ndir4");
	path_push(&mkdir_test4, "3");
	path_push(&mkdir_test4, "4");
	path_push(&mkdir_test4, "5");
	cr_assert(!path_exists(&mkdir_test4));
	cr_assert(!path_is_dir(&mkdir_test4));

	__is_debug_malloc = true;
	cr_assert(!path_mkdir(&mkdir_test4, 0700, true));
	__is_debug_malloc = false;

	// make 20+ depth which fails
	Path mkdir_test5;
	path_copy(&mkdir_test5, test_dir);
	for (int i = 0; i < 25; i++)
		path_push(&mkdir_test5, "a1");
	cr_assert(!path_mkdir(&mkdir_test5, 0700, true));

	Path file_stem_test1;
	path_copy(&file_stem_test1, test_dir);
	path_push(&file_stem_test1, "file.txt");
	path_file_stem(&file_stem_test1, buf, PATH_MAX);
	cr_assert(!strcmp(buf, "file"));

	sprint(buf, PATH_MAX, "Path={}", &file_stem_test1);
	cr_assert(!strcmp(buf, "Path=.base_test_path.fam/file.txt"));
}

MyTest(base, test_colors) {
	Path file;
	path_copy(&file, test_dir);
	path_push(&file, "file.txt");

	Path file2;
	path_copy(&file2, test_dir);
	path_push(&file2, "file2.txt");

	Stream ptr = myfopen(&file, O_WRONLY | O_CREAT, 0700);

	fprint(&ptr, "{}{}{}{}{}{}{}{}{}", DIMMED, MAGENTA, BRIGHT_RED, RED, BLUE, CYAN, YELLOW, GREEN,
		   RESET);

	myfclose(&ptr);

	/*
		Stream ptr2 = myfopen(&file, O_RDONLY, 0700);

		char buf[1024];
		u64 v = read_all(buf, 1, 1024, ptr2);
		if (v < 1024) {
			buf[v] = '\0';
		}
		int brackets = 0;
		for (int i = 0; i < strlen(buf); i++) {
			if (buf[i] == '[')
				brackets++;
		}
		cr_assert_eq(brackets, 9);
		myfclose(ptr2);

		// set no color
		setenv("NO_COLOR", "true", 1);
		MYFILE *ptr3 = myfopen(&file2, "w");
		myfprintf(ptr3, "%s%s%s%s%s%s%s%s%s", DIMMED, MAGENTA, BRIGHT_RED, RED, BLUE, CYAN, YELLOW,
				  GREEN, RESET);

		myfclose(ptr3);

		MYFILE *ptr4 = myfopen(&file2, "r");
		char buf2[1024];
		strcpy(buf2, "");
		v = read_all(buf2, 1, 1024, ptr4);
		brackets = 0;
		for (int i = 0; i < strlen(buf2); i++) {
			if (buf2[i] == '[')
				brackets++;
		}
		cr_assert_eq(brackets, 0);
		myfclose(ptr4);
		*/
}
