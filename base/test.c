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
#include <base/test.h>
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

	fprintf((FILE *)ptr, "%s%s%s%s%s%s%s%s%s", DIMMED, MAGENTA, BRIGHT_RED, RED, BLUE, CYAN, YELLOW,
			GREEN, RESET);

	myfclose(ptr);

	MYFILE *ptr2 = myfopen(&file, "r");
	char buf[1024];
	fgets(buf, 1024, (FILE *)ptr2);
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
	fprintf((FILE *)ptr3, "%s%s%s%s%s%s%s%s%s", DIMMED, MAGENTA, BRIGHT_RED, RED, BLUE, CYAN,
			YELLOW, GREEN, RESET);

	myfclose(ptr3);

	MYFILE *ptr4 = myfopen(&file2, "r");
	char buf2[1024];
	fgets(buf2, 1024, (FILE *)ptr4);
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
