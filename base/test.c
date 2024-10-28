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

#include <base/lib.h>
#include <base/test.h>

MySuite(base);

MyTest(base, test_string) {
	string s = String("test");
	cr_assert(!nil(s));
	cr_assert(!strcmp(cstring(&s), "test"));
	string s2 = INIT_STRING;
	cr_assert(nil(s2));
	string s3 = String(s);
	cr_assert(!strcmp(cstring(&s3), "test"));

	string s4 = String();
	cr_assert(!nil(s4));
	cr_assert(!strcmp(cstring(&s4), ""));

	cr_assert(string_equal(&s, &s3));
	cr_assert(!string_equal(&s, &s2));
	cr_assert(!string_equal(&s, &s4));

	cr_assert_eq(len(s), 4);
	cr_assert_eq(len(s2), 0);
	cr_assert_eq(string_len(&s3), 4);
	cr_assert_eq(string_len(&s4), 0);
	string_append_s(&s, &s3);
	cr_assert_eq(string_len(&s), 8);

	string s7 = String("testtest");
	cr_assert(string_equal(&s, &s7));

	string s5 = String("abc");
	string_append_s(&s, &s5);
	cr_assert_eq(len(s), 11);
	string s6 = String("testtestabc");
	cr_assert(string_equal(&s, &s6));

	cr_assert_eq(string_index_of(&s, &s5), 8);
	cr_assert_eq(string_index_of(&s6, &s3), 0);
	cr_assert_eq(string_index_of(&s3, &s5), -1);

	cr_assert_eq(string_last_index_of(&s, &s5), 8);
	cr_assert_eq(string_last_index_of(&s6, &s3), 4);
	cr_assert_eq(string_last_index_of(&s3, &s5), -1);

	cr_assert_eq(string_char_at(&s5, 0), 'a');
	cr_assert_eq(string_char_at(&s5, 1), 'b');
	cr_assert_eq(string_char_at(&s5, 2), 'c');
	fam_err = NoErrors;
	cr_assert_eq(string_char_at(&s5, 3), '\0');
	cr_assert_eq(fam_err, IndexOutOfBounds);
	fam_err = NoErrors;
	cr_assert_eq(string_char_at(&s5, 4), '\0');
	cr_assert_eq(fam_err, IndexOutOfBounds);
	fam_err = NoErrors;
	cr_assert_eq(string_char_at(&s5, -1), '\0');
	cr_assert_eq(fam_err, IndexOutOfBounds);

	string s8;
	string_substring_s(&s8, &s6, 2, 4);
	string s9 = String("st");
	cr_assert(string_equal(&s8, &s9));

	append(s8, s6);
	// st + testtestabc
	string s10 = String("sttesttestabc");
	cr_assert(string_equal(&s8, &s10));
	append(s8, "ok");
	string s11 = String("sttesttestabcok");
	cr_assert(string_equal(&s8, &s11));

	append(s8, "0123456789", 5);
	string s12 = String("sttesttestabcok01234");
	cr_assert(string_equal(&s8, &s12));

	cr_assert(equal(s8, s12));
	cr_assert(!equal(s8, "abc"));
	cr_assert(equal(s8, "sttesttestabcok01234"));
	string s13 = substring(s8, 3, 5);
	string s14 = String("es");
	cr_assert(equal(s13, s14));
	string s15 = substring(s8, 6);
	string s16 = String("testabcok01234");
	cr_assert(equal(s15, s16));
	string s17 = String("abc");
	cr_assert_eq(index_of(s16, s17), 4);
	cr_assert_eq(index_of(s16, "abc"), 4);

	string s18 = String("okokok");
	cr_assert_eq(last_index_of(s18, "ok"), 4);
	cr_assert_eq(index_of(s18, "ok"), 0);

	string test_str = String("line1\nline2\nline3\nline4\n");
	int64 i;
	int64 count = 0;
	char compare[11];

	while ((i = index_of(test_str, "\n")) >= 0) {
		string line = substring(test_str, 0, i);
		string s = substring(test_str, i + 1);
		move(test_str, s);
		snprintf(compare, 10, "line%lli", ++count);
		cr_assert(!strcmp(compare, cstring(&line)));
	}
	cr_assert_eq(count, 4);
}

Test(base, test_ptr) {
	int64 alloc_sum_pre = alloc_sum();
	Ptr ptr = ptr_test_obj(123, 100, 0xF);
	cr_assert(ptr_flag_check(ptr, 0));
	cr_assert(ptr_flag_check(ptr, 1));
	cr_assert(ptr_flag_check(ptr, 2));
	cr_assert(ptr_flag_check(ptr, 3));
	for (int i = 4; i < 100; i++) {
		cr_assert(!ptr_flag_check(ptr, i));
	}
	for (int i = -1; i > -10; i--) {
		cr_assert(!ptr_flag_check(ptr, i));
	}
	cr_assert_eq(ptr_id(ptr), 123);
	cr_assert_eq($len(ptr), 100);

	ptr_flag_set(ptr, 4, true);
	cr_assert(ptr_flag_check(ptr, 0));
	cr_assert(ptr_flag_check(ptr, 1));
	cr_assert(ptr_flag_check(ptr, 2));
	cr_assert(ptr_flag_check(ptr, 3));
	cr_assert(ptr_flag_check(ptr, 4));
	cr_assert(!ptr_flag_check(ptr, 5));

	cr_assert_eq(ptr_id(ptr), 123);
	cr_assert_eq($len(ptr), 100);

	int64 alloc_sum_post = alloc_sum();
	cr_assert_eq(alloc_sum_post - alloc_sum_pre, 1);
	int64 release_sum_pre = release_sum();
	ptr_free_test_obj(ptr);
	int64 release_sum_post = release_sum();
	cr_assert_eq(release_sum_post - release_sum_pre, 1);
}

Test(base, test_slab_allocator_config) {
}

Test(base, test_slab_allocator) {
	SlabAllocator sa = slab_allocator_create();

	fam_err = NoErrors;
	SlabAllocator sa2 = slab_allocator_create();
	cr_assert_neq(sa2, NULL);
	cr_assert_eq(fam_err, NoErrors);

	Ptr ptr1, ptr2, ptr3;
	ptr1 = slab_allocator_allocate(sa2, 32);
	ptr2 = slab_allocator_allocate(sa2, 33);
	ptr3 = slab_allocator_allocate(sa2, 394133);
	cr_assert(!ptr3);
	cr_assert(ptr1);
	cr_assert(ptr2);
	printf("ptr_len%i\n", $len(ptr1));
	cr_assert_eq($len(ptr1), 32);
	cr_assert_eq($len(ptr2), 48);
	printf("ptr1.id=%i\n", ptr_id(ptr1));
	printf("ptr2.id=%i\n", ptr_id(ptr2));
	ptr3 = slab_allocator_allocate(sa2, 33);
	cr_assert_eq($len(ptr3), 48);
	printf("ptr3.id=%i\n", ptr_id(ptr3));
	printf("ptr3.id=%i\n", ptr_id(ptr3));
	printf("ptr3.id=%i\n", ptr_id(ptr3));

	cr_assert(!nil(ptr3));
	slab_allocator_free(sa2, ptr3);
	cr_assert(nil(ptr3));
	Ptr ptr4 = slab_allocator_allocate(sa2, 33);
	printf("ptr4.id=%i\n", ptr_id(ptr4));
	Ptr ptr5 = slab_allocator_allocate(sa2, 33);
	printf("ptr5.id=%i\n", ptr_id(ptr5));
}

Test(base, test_slab_allocator_resize) {
	SlabAllocator sa = slab_allocator_create();
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);
	int size = 256;
	Ptr arr[size];
	for (int i = 0; i < size; i++) {
		arr[i] = slab_allocator_allocate(sa, 32);
		cr_assert_eq(ptr_id(arr[i]), i);
		cr_assert_eq($len(arr[i]), 32);
		byte *data = $(arr[i]);
		data[0] = (i * 3) % 256;
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), size);

	for (int i = 0; i < size; i++) {
		cr_assert_eq(ptr_id(arr[i]), i);
		cr_assert_eq($len(arr[i]), 32);
		byte *data = $(arr[i]);
		cr_assert_eq(data[0], (i * 3) % 256);
		slab_allocator_free(sa, arr[i]);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);

	for (int i = 0; i < size; i++) {
		arr[i] = slab_allocator_allocate(sa, 32);
		cr_assert_eq(ptr_id(arr[i]), (size - i) - 1);
		cr_assert_eq($len(arr[i]), 32);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), size);

	for (int i = 0; i < size; i++) {
		cr_assert_eq(ptr_id(arr[i]), (size - i) - 1);
		cr_assert_eq($len(arr[i]), 32);
		byte *data = $(arr[i]);
		slab_allocator_free(sa, arr[i]);
	}
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);
}

Test(base, test_limits) {
	cr_assert_eq(INT64_MAX, 9223372036854775807LL);
	cr_assert_eq(INT64_MAX_IMPL, 9223372036854775807LL);
	cr_assert_eq(INT_MAX, 2147483647);
	cr_assert_eq(INT_MAX_IMPL, 2147483647);
	cr_assert_eq(BYTE_MAX, 255);
	cr_assert_eq(BYTE_MAX_IMPL, 255);
	cr_assert_eq(INT_MAX, 2147483647);
	cr_assert_eq(INT_MIN, -2147483648);
	cr_assert_eq(INT_MIN_IMPL, -2147483648);
	cr_assert_eq(UINT32_MAX, 4294967295);
	cr_assert_eq(UINT32_MAX_IMPL, 4294967295);

#pragma clang diagnostic ignored "-Wimplicitly-unsigned-literal"
	cr_assert_eq(INT64_MIN, (-9223372036854775808LL));
	cr_assert_eq(INT64_MIN_IMPL, (-9223372036854775808LL));
}
