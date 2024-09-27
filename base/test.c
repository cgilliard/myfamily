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

#include <base/colors.h>
#include <base/misc.h>
#include <base/test.h>
#include <errno.h>
#include <limits.h>
#ifdef __linux__
#include <linux/limits.h>
#endif // __linux__

MySuite(base);

MyTest(base, test_colors) {
	Path file;
	path_copy(&file, test_dir);
	path_push(&file, "file.txt");

	Path file2;
	path_copy(&file2, test_dir);
	path_push(&file2, "file2.txt");

	MYFILE *ptr = myfopen(&file, "w");

	myfprintf(ptr, "%s%s%s%s%s%s%s%s%s", DIMMED, MAGENTA, BRIGHT_RED, RED, BLUE, CYAN, YELLOW,
			  GREEN, RESET);
	cr_assert(!myferror(ptr));

	myfclose(ptr);

	MYFILE *ptr2 = myfopen(&file, "r");
	char buf[1024];
	u64 v = read_all(buf, 1, 1024, ptr2);
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
}

MyTest(base, test_resources) {
	u64 initial_my_realloc_sum = myrealloc_sum();
	u64 initial_my_malloc_sum = mymalloc_sum();
	u64 initial_my_free_sum = myfree_sum();
	void *tmp = mymalloc(10);
	cr_assert_neq(tmp, NULL);
	void *tmp2 = myrealloc(tmp, 100);
	cr_assert_neq(tmp2, NULL);
	myfree(tmp2);
	cr_assert_eq(myrealloc_sum(), initial_my_realloc_sum + 1);
	cr_assert_eq(myfree_sum(), initial_my_free_sum + 1);
	cr_assert_eq(mymalloc_sum(), initial_my_malloc_sum + 1);

	Path file;
	path_copy(&file, test_dir);
	path_push(&file, "file.txt");
	MYFILE *ptr = myfopen(&file, "w");
	myfclose(ptr);

	cr_assert(path_exists(&file));
	cr_assert(!path_is_dir(&file));
	cr_assert(!path_mkdir(&file, 0700, false));
}

MyTest(base, test_path) {
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
	char buf[PATH_MAX];
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
}

MyTest(base, test_misc) {
	cr_assert(!rstrstr("test", "test1"));

	Path file;
	path_copy(&file, test_dir);
	path_push(&file, "file.txt");

	Path file2;
	path_copy(&file2, test_dir);
	path_push(&file2, "file2.txt");

	MYFILE *ptr = myfopen(&file, "w");

	char *testline = "testline";
	myfprintf(ptr, "%s", testline);

	myfclose(ptr);

	cr_assert(!path_exists(&file2));
	copy_file(&file2, &file);
	cr_assert(path_exists(&file2));

	MYFILE *ptr2 = myfopen(&file, "r");
	char buf[1024];
	strcpy(buf, "");
	u64 v = read_all(buf, 1, 1024, ptr2);
	buf[v] = 0;
	cr_assert_eq(v, strlen(testline));
	cr_assert(!strcmp(testline, buf));
	myfclose(ptr2);

	Path file3;
	path_copy(&file3, test_dir);
	path_push(&file3, "abc");
	Path file4;
	path_copy(&file4, test_dir);
	path_push(&file4, "def");
	cr_assert(copy_file(&file4, &file3));

	cr_assert(copy_file(NULL, &file));

	Path file5;
	path_copy(&file5, test_dir);
	path_push(&file5, "a");
	path_push(&file5, "a");
	path_push(&file5, "a");
	path_push(&file5, "a");

	cr_assert(copy_file(&file5, &file));
}

MyTest(base, test_remove_dir) {
	Path file;
	path_copy(&file, test_dir);
	path_push(&file, "mydir");
	path_push(&file, "sub1");

	path_mkdir(&file, 0700, true);

	Path text;
	path_copy(&text, &file);
	path_push(&text, "file1.txt");

	MYFILE *ptr = myfopen(&text, "w");
	myfprintf(ptr, "test1");
	myfclose(ptr);

	cr_assert(path_exists(&text));
	Path text2;
	path_copy(&text2, test_dir);
	path_push(&text2, "mydir");
	path_push(&text2, "file2.txt");
	MYFILE *ptr2 = myfopen(&text2, "w");
	myfprintf(ptr2, "test2");
	myfclose(ptr2);

	Path rem;
	path_copy(&rem, test_dir);
	path_push(&rem, "mydir");
	cr_assert(path_exists(&rem));
	remove_directory(&rem, true);

	cr_assert(path_exists(&rem));
	remove_directory(&rem, false);

	cr_assert(!path_exists(&rem));
}

MyTest(base, test_other_misc_situations) {
	char buf[1];
	Path test;
	path_copy(&test, resources_dir);
	path_push(&test, "simple.txt");

	MYFILE *stream = myfopen(&test, "r");
	__is_debug_misc_ferror = true;
	errno = 0;
	cr_assert_eq(errno, 0);
	read_all(buf, 1, 1, stream);
	cr_assert_eq(errno, EIO);
	__is_debug_misc_ferror = false;
	myfclose(stream);

	Path dst;
	path_copy(&dst, test_dir);
	path_push(&dst, "tmp.txt");
	__is_debug_misc_fwrite = true;
	// write error simulated
	cr_assert(copy_file(&dst, &test));
	__is_debug_misc_fwrite = false;

	__is_debug_misc_stat = true;
	cr_assert(remove_directory(test_dir, true));
	__is_debug_misc_stat = false;

	Path dirtest1;
	path_copy(&dirtest1, test_dir);
	path_push(&dirtest1, "testsub1");
	path_push(&dirtest1, "testsub2");
	path_push(&dirtest1, "testsub3");
	path_mkdir(&dirtest1, 0700, true);
	// pop back to testsub1
	path_pop(&dirtest1);
	path_pop(&dirtest1);

	__is_debug_misc_remove_dir = true;
	cr_assert(remove_directory(&dirtest1, false));
	__is_debug_misc_remove_dir = false;

	path_push(&dirtest1, "f.txt");
	copy_file(&dirtest1, &test);
	path_pop(&dirtest1);

	__is_debug_misc_unlink = true;
	cr_assert(remove_directory(&dirtest1, false));
	__is_debug_misc_unlink = false;

	__is_debug_misc_no_exit = true;
	// will not return because of debug flag
	exit_error("test");
	__is_debug_misc_no_exit = false;

	__is_debug_misc_no_exit = false;
}
