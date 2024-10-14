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

#include <base/backtrace.h>
#include <base/bitflags.h>
#include <base/colors.h>
#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/misc.h>
#include <base/panic.h>
#include <base/slabs.h>
#include <base/test.h>
#include <limits.h>
#include <pthread.h>
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
	fam_err = NoErrors;
	read_all(buf, 1, 1, stream);
	cr_assert_eq(fam_err, IO);
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

	__is_debug_misc_preserve = true;
	cr_assert(remove_directory(&dirtest1, false));
	__is_debug_misc_preserve = false;

	__is_debug_misc_no_exit = true;
	// will not return because of debug flag
	exit_error("test");
	__is_debug_misc_no_exit = false;
}

MyTest(base, test_path_other_situations) {
	Path test1;
	path_copy(&test1, test_dir);
	path_push(&test1, "other");
	path_push(&test1, "stemless");
	char buf[10];
	path_file_stem(&test1, buf, 10);
	cr_assert(!strcmp(buf, "stemless"));
	fam_err = NoErrors;
	cr_assert(!path_file_size(&test1));
	cr_assert_eq(fam_err, IO);

	Path test2;
	path_for(&test2, ".");
	path_canonicalize(&test2);

	Path test3;
	path_for(&test3, ".");
	__is_debug_realloc = true;
	path_canonicalize(&test3);
	__is_debug_realloc = false;

	// succeeded canonicalized path of '.' would be longer.
	cr_assert(strlen(path_to_string(&test2)) > strlen("."));
	// this failed so we have the original path '.'.
	cr_assert(strlen(path_to_string(&test3)) == strlen("."));

	Path test4;
	path_for(&test4, "~");
	__is_debug_path_homedir_null = true;
	cr_assert(path_canonicalize(&test4));
	__is_debug_path_homedir_null = false;

	Path test5;
	char pmax[PATH_MAX + 1];
	for (int i = 0; i < PATH_MAX; i++)
		pmax[i] = 'z';
	pmax[PATH_MAX] = 0;
	path_for(&test5, "~");
	path_push(&test5, pmax);
	cr_assert(path_canonicalize(&test5));

	Path test6;
	path_for(&test6, "~");
	path_push(&test6, "test/");
	cr_assert(path_canonicalize(&test6));
}

typedef struct TestData {
	u64 v1;
	u32 v2;
	i32 v3;
	char buf[10];
} TestData;

MyTest(base, test_fat_ptr) {
	cr_assert_eq($size(null), 0);
	cr_assert_eq($(null), NULL);
	FatPtr ptr = null;
	cr_assert_eq($(ptr), NULL);
	cr_assert_eq($size(ptr), 0);
	cr_assert(nil(ptr));

	for (u64 i = 0; i < 100; i++) {
		u32 id = i * 2;
		FatPtr ptr2;
		// create a 32 bit test obj
		fat_ptr_test_obj32(&ptr2, id, sizeof(TestData), false, true, false);
		// do assertions and use allocated data
		cr_assert_eq($size(ptr2), sizeof(TestData));
		cr_assert_eq(fat_ptr_id(&ptr2), id);
		cr_assert(!$global(ptr2));
		cr_assert($pin(ptr2));
		cr_assert(!$copy(ptr2));
		TestData *td2 = $(ptr2);
		td2->v1 = 1234;
		td2->v2 = 5678;
		td2->v3 = 9999;
		strcpy(td2->buf, "hi");
		cr_assert_eq(td2->v1, 1234);
		cr_assert_eq(td2->v2, 5678);
		cr_assert_eq(td2->v3, 9999);
		cr_assert(!strcmp(td2->buf, "hi"));
		fat_ptr_free_test_obj32(&ptr2);
	}

	for (u32 i = 16777216 - 1000; i < 16777216; i++) {
		FatPtr ptr2;
		u32 id = i;
		fat_ptr_test_obj32(&ptr2, id, i, false, false, true);
		cr_assert_eq($size(ptr2), i);
		cr_assert(!$global(ptr2));
		cr_assert(!$pin(ptr2));
		cr_assert($copy(ptr2));
		fat_ptr_free_test_obj32(&ptr2);
	}
}

MyTest(base, test_slab_config) {
	SlabAllocatorConfig sc1;
	slab_allocator_config_build(&sc1, true, false);
	cr_assert_eq(sc1.zeroed, true);

	slab_allocator_config_build(&sc1, true, false);
	cr_assert_eq(sc1.zeroed, true);

	slab_allocator_config_build(&sc1, true, false);
	cr_assert_eq(sc1.zeroed, true);

	slab_allocator_config_build(&sc1, false, false);
	cr_assert_eq(sc1.zeroed, false);

	SlabType t1 = {1, 2, 3, 4};
	slab_allocator_config_add_type(&sc1, &t1);

	SlabType t2 = {5, 6, 7, 8};
	slab_allocator_config_add_type(&sc1, &t2);

	cr_assert_eq(sc1.slab_types[0].slab_size, 1);
	cr_assert_eq(sc1.slab_types[1].slab_size, 5);
	cr_assert_eq(sc1.slab_types_count, 2);
}

MyTest(base, test_slab_allocator32) {
	SlabAllocatorConfig sc;
	slab_allocator_config_build(&sc, false, false);
	SlabType t1 = {32, 10, 1, 10};
	slab_allocator_config_add_type(&sc, &t1);
	SlabAllocator sa;
	cr_assert(!slab_allocator_build(&sa, &sc));

	for (u64 i = 0; i < 10; i++) {
		FatPtr fptr;
		int ret = slab_allocator_allocate(&sa, 13, &fptr);
		cr_assert_eq(ret, 0);
		// it's rounded up to our nearest slab size

		cr_assert_eq($size(fptr), 32);
		slab_allocator_free(&sa, &fptr);
	}

	FatPtr fptr;
	int ret = slab_allocator_allocate(&sa, 13, &fptr);
	cr_assert_eq(ret, 0);
	u64 x = 101;
	u64 *ptr = fat_ptr_data(&fptr);
	memcpy(ptr, &x, sizeof(u64));
	cr_assert_eq(*ptr, 101);
	slab_allocator_free(&sa, &fptr);

	FatPtr arr[10];
	for (u64 i = 0; i < 10; i++) {
		int ret = slab_allocator_allocate(&sa, 10, &arr[i]);
		cr_assert_eq(ret, 0);
		cr_assert_eq($size(arr[i]), 32);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), 10);

	// no more slabs
	ret = slab_allocator_allocate(&sa, 10, &fptr);
	// returns error neq(0)
	cr_assert_neq(ret, 0);

	for (u64 i = 0; i < 10; i++) {
		slab_allocator_free(&sa, &arr[i]);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), 0);
}

MyTest(base, test_big32) {
	SlabAllocatorConfig sc;
	slab_allocator_config_build(&sc, false, false);
	u64 size = 24;
	// u64 count = MAX_SLABS;
	u64 count = 1000000;
	SlabType t1 = {size, count, 1, count};
	slab_allocator_config_add_type(&sc, &t1);
	SlabAllocator sa;
	cr_assert(!slab_allocator_build(&sa, &sc));
	FatPtr *arr;
	arr = malloc(sizeof(FatPtr) * count);

	for (u64 i = 0; i < count; i++) {
		int ret = slab_allocator_allocate(&sa, 8, &arr[i]);
		cr_assert_eq(ret, 0);
		cr_assert_eq($size(arr[i]), size);
	}
	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), count);

	for (u64 i = 0; i < count; i++) {
		slab_allocator_free(&sa, &arr[i]);
	}
	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), 0);

	free(arr);
}

MyTest(base, test_overwrite32) {
	SlabAllocatorConfig sc;
	slab_allocator_config_build(&sc, false, false);
	u64 size = 24;
	u64 count = 100;
	SlabType t1 = {size, count, 1, count};
	slab_allocator_config_add_type(&sc, &t1);
	SlabAllocator sa;
	cr_assert(!slab_allocator_build(&sa, &sc));

	FatPtr fptr;
	int ret = slab_allocator_allocate(&sa, sizeof(u64), &fptr);
	void *data = fat_ptr_data(&fptr);
	cr_assert_eq(data, fat_ptr_data(&fptr));
	cr_assert_eq(ret, 0);
	u64 x = 101;
	u64 *ptr = fat_ptr_data(&fptr);
	memcpy(ptr, &x, sizeof(u64));
	cr_assert_eq(*ptr, 101);
	cr_assert_eq(data, fat_ptr_data(&fptr));
	slab_allocator_free(&sa, &fptr);
}

MyTest(base, test_resize) {
	SlabAllocatorConfig sc;
	slab_allocator_config_build(&sc, false, false);
	u64 size = 24;
	u64 count = 10;
	SlabType t1 = {size, count, 1, count * 3};
	slab_allocator_config_add_type(&sc, &t1);
	SlabAllocator sa;
	cr_assert(!slab_allocator_build(&sa, &sc));

	FatPtr *arr;
	arr = malloc(sizeof(FatPtr) * count);

	for (u64 i = 0; i < count; i++) {
		int ret = slab_allocator_allocate(&sa, 8, &arr[i]);
		cr_assert_eq(ret, 0);
		cr_assert_eq($size(arr[i]), size);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), count);

	// resize here
	FatPtr fptr;
	int ret = slab_allocator_allocate(&sa, 8, &fptr);
	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), count + 1);
	cr_assert_eq(ret, 0);
	slab_allocator_free(&sa, &fptr);

	for (u64 i = 0; i < count; i++) {
		slab_allocator_free(&sa, &arr[i]);
	}
	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), 0);

	free(arr);
}

MyTest(base, test_multi_resize) {
	SlabAllocatorConfig sc;
	slab_allocator_config_build(&sc, false, false);
	u64 size = 40;
	u64 count = 10;
	SlabType t1 = {size, count, 1, 15};
	slab_allocator_config_add_type(&sc, &t1);
	SlabAllocator sa;
	// max_slabs not divisible by slabs_per_resize
	cr_assert(slab_allocator_build(&sa, &sc));

	FatPtr *arr;
	arr = malloc(sizeof(FatPtr) * count * 3);

	SlabAllocatorConfig sc2;
	slab_allocator_config_build(&sc2, false, false);
	SlabType t2 = {size, count, 1, 30};

	slab_allocator_config_add_type(&sc2, &t2);
	cr_assert(!slab_allocator_build(&sa, &sc2));

	int itt = 30;
	for (u64 i = 0; i < itt; i++) {
		int ret = slab_allocator_allocate(&sa, 8, &arr[i]);
		cr_assert_eq(ret, 0);
		cr_assert_eq($size(arr[i]), size);
	}

	FatPtr tmp;
	// This is an error because we are 'no_malloc' and we're over the limit of slabs.
	cr_assert(slab_allocator_allocate(&sa, 8, &tmp));

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), itt);

	for (u64 i = 0; i < itt; i++) {
		slab_allocator_free(&sa, &arr[i]);
	}
	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), 0);

	free(arr);
}

MyTest(base, test_zeroed) {
	SlabAllocatorConfig sc;
	slab_allocator_config_build(&sc, true, false);
	u64 size = 24;
	u64 count = 10;
	SlabType t1 = {size, count, 1, count};
	slab_allocator_config_add_type(&sc, &t1);
	SlabAllocator sa;
	cr_assert(!slab_allocator_build(&sa, &sc));

	FatPtr *arr;
	arr = malloc(sizeof(FatPtr) * count * 3);

	int itt = 10;

	for (u64 i = 0; i < itt; i++) {
		FatPtr ptr1;
		int ret = slab_allocator_allocate(&sa, 8, &ptr1);
		cr_assert_eq(ret, 0);
		cr_assert_eq($size(ptr1), size);

		char *dataptr = fat_ptr_data(&ptr1);
		// memory is zeroed so even though we should have the same id as last time
		// it's '0'
		cr_assert_eq(dataptr[0], 0);
		cr_assert_eq(dataptr[1], 0);
		cr_assert_eq(dataptr[2], 0);
		cr_assert_eq(dataptr[3], 0);
		strcpy(dataptr, "1234");
		slab_allocator_free(&sa, &ptr1);
	}

	for (u64 i = 0; i < itt; i++) {
		int ret = slab_allocator_allocate(&sa, 8, &arr[i]);
		cr_assert_eq(ret, 0);
		cr_assert_eq($size(arr[i]), size);
	}

	FatPtr tmp;
	// full
	cr_assert(slab_allocator_allocate(&sa, 8, &tmp));

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), itt);

	for (u64 i = 0; i < itt; i++) {
		slab_allocator_free(&sa, &arr[i]);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), 0);

	free(arr);

	// simple test of non-zeroed case to see data is still there
	SlabAllocatorConfig sc2;
	slab_allocator_config_build(&sc2, false, false);
	slab_allocator_config_add_type(&sc2, &t1);
	SlabAllocator sa2;
	cr_assert(!slab_allocator_build(&sa2, &sc2));

	cr_assert(!slab_allocator_allocate(&sa2, 8, &tmp));
	char *dataptr = fat_ptr_data(&tmp);
	strcpy(dataptr, "test");
	cr_assert(!strcmp(dataptr, "test"));

	slab_allocator_free(&sa2, &tmp);
	cr_assert(!slab_allocator_allocate(&sa2, 8, &tmp));

	dataptr = fat_ptr_data(&tmp);
	cr_assert(!strcmp(dataptr, "test"));
}

MyTest(base, test_zero_size_initial) {
	SlabAllocatorConfig sc;
	slab_allocator_config_build(&sc, false, false);
	u64 size = 24;
	u64 count = 10;
	SlabType t1 = {
		.slab_size = size, .slabs_per_resize = 10, .initial_chunks = 0, .max_slabs = 3 * count};
	slab_allocator_config_add_type(&sc, &t1);
	SlabAllocator sa;
	cr_assert(!slab_allocator_build(&sa, &sc));

	FatPtr *arr;
	int itt = count * 3;
	arr = malloc(sizeof(FatPtr) * itt);

	for (u64 i = 0; i < itt; i++) {
		int ret = slab_allocator_allocate(&sa, 8, &arr[i]);
		cr_assert_eq(ret, 0);
		cr_assert_eq($size(arr[i]), size);
		char *test = fat_ptr_data(&arr[i]);
		strcpy(test, "test");
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), itt);
	FatPtr tmp;
	// over capacity / no_malloc
	cr_assert(slab_allocator_allocate(&sa, 8, &tmp));
	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), itt);

	for (u64 i = 0; i < itt; i++) {
		char *test = fat_ptr_data(&arr[i]);
		cr_assert(!strcmp(test, "test"));
		slab_allocator_free(&sa, &arr[i]);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(&sa), 0);

	free(arr);
}

MyTest(base, test_nil) {
	// 'null' and 'nil' macros may be used with the FatPtr type to simulate NULL/null/nil
	// conditions in other languages. 'null' is a macro which is a FatPtr type and has a length
	// of 0 and data
	// == NULL. It is not possible to have a FatPtr with these values without the use of the
	// 'null' macro or by calling one of the free functions because attempting to allocate a
	// length of 0 through the slab allocator will result in an error. The 'nil' macro just
	// tests this property and returns a boolean. After calling free (either directly from a
	// slab allocator or through chain_free as seen below), The value of the freed FatPtr will
	// be 'null' and will return true when passed to the 'nil' macro.

	// initialize a FatPtr to 'null'.
	FatPtr ptr = null;
	// assert that this ptr is 'nil'.
	cr_assert(nil(ptr));
	// call chain_malloc to allocate 1000 bytes to this FatPtr.
	cr_assert(!fam_alloc(&ptr, 1000));
	// assert that it is no longer 'nil'.
	cr_assert(!nil(ptr));
	// free the FatPtr via chain_free.
	fam_free(&ptr);
	// assert that the FatPtr is now 'nil'.
	cr_assert(nil(ptr));
}

MyTest(base, test_realloc) {
	FatPtr ptr = null;
	cr_assert(!fam_alloc(&ptr, 1000));
	strcpy($(ptr), "testing123");
	cr_assert(!strcmp(fat_ptr_data(&ptr), "testing123"));
	cr_assert($size(ptr) < 2000);
	cr_assert(!fam_realloc(&ptr, 2000));
	cr_assert($size(ptr) >= 2000);
	cr_assert(!strcmp(fat_ptr_data(&ptr), "testing123"));

	fam_free(&ptr);
}

MyTest(base, test_sync_allocator) {
	SendStateGuard sg = SetSend(true);
	FatPtr ptr = null;
	cr_assert(nil(ptr));
	fam_alloc(&ptr, 100);
	cr_assert(!nil(ptr));
	fam_free(&ptr);
	cr_assert(nil(ptr));
}

// create some shared variables to demonstrate the sync_allocator
FatPtr shared_ptr = null; // shared FatPtr
// Initialize a condition/lock
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// set  flag to false
bool flag = false;

// thread function
void *start_thread_sync_test() {
	// if we did not use the global allocator, it would result in
	// a panic
	fam_free(&shared_ptr);

	FatPtr ptr;
	fam_alloc(&ptr, 10);
	fam_free(&ptr);

	// set flag and notify the main thread that we're done
	pthread_mutex_lock(&lock);
	flag = true;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);

	return NULL;
}

// Test main entry point
MyTest(base, test_threaded_sync) {
	pthread_t thread;

	// create scope to get sync_allocator
	{
		SendStateGuard cg = SetSend(true);
		//  allocate shared_ptr from the global sync allocator
		cr_assert(nil(shared_ptr));
		cr_assert(!fam_alloc(&shared_ptr, 100));
		cr_assert(!nil(shared_ptr));
	}

	// spawn a new thread
	cr_assert(!pthread_create(&thread, NULL, &start_thread_sync_test, NULL));

	// wait for the condition to be met

	pthread_mutex_lock(&lock);
	while (!flag) {
		pthread_cond_wait(&cond, &lock);
	}
	pthread_mutex_unlock(&lock);

	cr_assert(!pthread_join(thread, NULL)); // Wait for thread to finish
}

MyTest(base, test_fam_alloc) {
	FatPtr ptr1;
	cr_assert(!fam_alloc(&ptr1, 100));

	fam_free(&ptr1);

	// try something bigger than our slabs
	FatPtr ptr2;
	cr_assert(!fam_alloc(&ptr2, 17000000));

	((u8 *)$(ptr2))[0] = '1';
	((u8 *)$(ptr2))[1] = '2';
	((u8 *)$(ptr2))[2] = '3';

	cr_assert(!fam_realloc(&ptr2, 100));
	cr_assert_eq(((u8 *)$(ptr2))[0], '1');
	cr_assert_eq(((u8 *)$(ptr2))[1], '2');
	cr_assert_eq(((u8 *)$(ptr2))[2], '3');
	fat_ptr_pin(&ptr2);

	// fails due to pin.
	cr_assert(fam_realloc(&ptr2, 200));

	fam_free(&ptr2);

	FatPtr ptr3;
	{
		SendStateGuard _ = SetSend(true);
		cr_assert(!fam_alloc(&ptr3, 100));
		cr_assert(fat_ptr_is_global(&ptr3));
	}

	fam_free(&ptr3);
}

MyTest(base, test_multi_scope) {
	{
		FatPtr ptr0 = null;
		fam_alloc(&ptr0, 100);
		cr_assert(!fat_ptr_is_global(&ptr0));
		fam_free(&ptr0);

		SendStateGuard sg = SetSend(true);
		FatPtr ptr = null;
		cr_assert(nil(ptr));
		fam_alloc(&ptr, 100);
		cr_assert(fat_ptr_is_global(&ptr));
		cr_assert(!nil(ptr));
		fam_free(&ptr);
		cr_assert(nil(ptr));

		{
			FatPtr ptr0 = null;
			fam_alloc(&ptr0, 100);
			cr_assert(fat_ptr_is_global(&ptr0));
			fam_free(&ptr0);
			SendStateGuard sg2 = SetSend(false);
			FatPtr ptr = null;
			cr_assert(nil(ptr));
			fam_alloc(&ptr, 100);
			cr_assert(!fat_ptr_is_global(&ptr));
			cr_assert(!nil(ptr));
			fam_free(&ptr);
			cr_assert(nil(ptr));

			{
				FatPtr ptr0 = null;
				fam_alloc(&ptr0, 100);
				cr_assert(!fat_ptr_is_global(&ptr0));
				fam_free(&ptr0);
				SendStateGuard sg2 = SetSend(false);
				FatPtr ptr = null;
				cr_assert(nil(ptr));
				fam_alloc(&ptr, 100);
				cr_assert(!fat_ptr_is_global(&ptr));
				cr_assert(!nil(ptr));
				fam_free(&ptr);
				cr_assert(nil(ptr));
			}

			FatPtr ptr2;
			fam_alloc(&ptr2, 100);
			cr_assert(!fat_ptr_is_global(&ptr2));
			fam_free(&ptr2);
		}
		FatPtr ptr2;
		fam_alloc(&ptr2, 100);
		cr_assert(fat_ptr_is_global(&ptr2));
		fam_free(&ptr2);
	}

	FatPtr ptr2;
	fam_alloc(&ptr2, 100);
	cr_assert(!fat_ptr_is_global(&ptr2));
	fam_free(&ptr2);
}

// Note: address sanitizer and criterion seem to have problems with this test on certain
// platforms/configurations. I tested both on linux/mac in the actual binary and it works
// for both explicit panic and signals. So, I think it works. Will leave this disabled for now.
/*
__attribute__((noreturn)) void on_panic(const char *msg) {
	printf("on_panic = %s\n", msg);
	cr_assert(!strcmp(msg, "test7"));
	exit(0);
}
*/

MyTest(base, test_panic) {
	// set_on_panic(on_panic);
	// panic("test7");
	//    ensure we never get here
	// cr_assert(false);
}

MyTest(base, test_bitflags) {
	u8 flags[3] = {};
	BitFlags bf1 = {.flags = flags, .capacity = 3};
	bitflags_set(&bf1, 3, true);

	cr_assert(bitflags_check(&bf1, 3));
	bitflags_set(&bf1, 3, false);
	cr_assert(!bitflags_check(&bf1, 3));

	for (u32 i = 0; i < 3 * 8; i++)
		bitflags_set(&bf1, 3, false);

	for (u32 i = 0; i < 3 * 8; i++)
		cr_assert(!bitflags_check(&bf1, i));

	cr_assert(!bitflags_set(&bf1, 0, true));
	cr_assert(!bitflags_set(&bf1, 4, true));
	cr_assert(!bitflags_set(&bf1, 7, true));
	cr_assert(!bitflags_set(&bf1, 19, true));
	cr_assert(!bitflags_set(&bf1, 20, true));

	for (u32 i = 0; i < 3 * 8; i++) {
		if (i == 0 || i == 4 || i == 7 || i == 19 || i == 20)
			cr_assert(bitflags_check(&bf1, i));
		else
			cr_assert(!bitflags_check(&bf1, i));
	}

	// test out of range
	cr_assert(bitflags_set(&bf1, 24, true));

	// test out of range
	cr_assert(!bitflags_check(&bf1, 24));
}

MyTest(base, test_backtrace) {
	Backtrace bt;
	backtrace_generate(&bt);
	backtrace_print(&bt);
}
