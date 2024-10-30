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
	string s0 = string_create_cs("abc");
	cr_assert_eq(string_len(s0), 3);
	cr_assert(!strcmp(cstring(s0), "abc"));

	string s1 = string_create_cs("test1");
	string s2 = string_create_cs("test2");
	string s3 = string_append_string(s1, s2);

	cr_assert(!strcmp(cstring(s3), "test1test2"));
	cr_assert_eq(string_len(s3), strlen("test1test2"));
	for (int i = 0; i < string_len(s3); i++) {
		cr_assert_eq(string_char_at(s3, i), "test1test2"[i]);
	}
	cr_assert_eq(fam_err, NoErrors);
	cr_assert_eq(string_char_at(s3, 100), 0);
	cr_assert_eq(fam_err, IndexOutOfBounds);
	string s4 = string_substring_s(s3, 2, 4);
	cr_assert(!strcmp(cstring(s4), "st"));

	string s5 = string_create_cs("dddd");
	string s6 = string_ref(s5);
}

/*
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
	char compare[101];

	while ((i = index_of(test_str, "\n")) >= 0) {
		string line = substring(test_str, 0, i);
		string s = substring(test_str, i + 1);
		move(test_str, s);
		count++;
		snprintf(compare, 100, "line%lli", count);
		cr_assert(!strcmp(compare, cstring(&line)));
	}
	cr_assert_eq(count, 4);
}
*/

/*
MyTest(base, test_ptr) {
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
*/

MyTest(base, test_slab_allocator) {
	SlabAllocator sa = slab_allocator_create();

	fam_err = NoErrors;
	SlabAllocator sa2 = slab_allocator_create();
	cr_assert_neq(sa2, NULL);
	cr_assert_eq(fam_err, NoErrors);

	Ptr ptr0, ptr1, ptr2, ptr3;
	ptr0 = slab_allocator_allocate(sa2, 1);
	ptr1 = slab_allocator_allocate(sa2, 32);
	ptr2 = slab_allocator_allocate(sa2, 33);
	ptr3 = slab_allocator_allocate(sa2, 394133);
	cr_assert(!ptr3);
	cr_assert(ptr1);
	cr_assert(ptr2);
	cr_assert(ptr0);
	cr_assert_eq($len(ptr0), 16);
	cr_assert_eq($len(ptr1), 32);
	cr_assert_eq($len(ptr2), 48);
	ptr3 = slab_allocator_allocate(sa2, 33);
	cr_assert_eq($len(ptr3), 48);

	cr_assert(!nil(ptr3));
	slab_allocator_free(sa2, ptr3);
	cr_assert(nil(ptr3));
	Ptr ptr4 = slab_allocator_allocate(sa2, 33);
	Ptr ptr5 = slab_allocator_allocate(sa2, 33);
	Ptr ptr6 = slab_allocator_allocate(sa2, 0);
	cr_assert(ptr6);
	cr_assert(!nil(ptr0));
	cr_assert(!nil(ptr6));
	cr_assert_eq($len(ptr6), 0);
}

MyTest(base, test_slab_allocator_resize) {
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

MyTest(base, test_big) {
	SlabAllocator sa = slab_allocator_create();
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);
	// unsigned int size = ((1024LL * 1024LL * 1024LL * 4) - 128);
	unsigned int size = (1024LL * 4LL);
	int ss = 16;
	Ptr *arr = alloc(sizeof(Ptr) * size, false);
	for (unsigned int i = 0; i < size; i++) {
		arr[i] = slab_allocator_allocate(sa, ss);
		if (nil(arr[i]))
			printf("nil ptr at %u\n", i);
		cr_assert(!nil(arr[i]));
		cr_assert_eq(ptr_id(arr[i]), i);
		cr_assert_eq($len(arr[i]), ss);
		byte *data = $(arr[i]);
		for (int j = 0; j < ss; j++)
			data[j] = (j + i * 3) % 256;
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), size);

	for (unsigned int i = 0; i < size; i++) {
		cr_assert_eq(ptr_id(arr[i]), i);
		cr_assert_eq($len(arr[i]), ss);
		byte *data = $(arr[i]);

		for (int j = 0; j < ss; j++)
			cr_assert_eq(data[j], (j + i * 3) % 256);

		slab_allocator_free(sa, arr[i]);
	}

	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);

	Ptr p = slab_allocator_allocate(sa, ss);
	cr_assert_eq(ptr_id(p), size - 1);
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 1);
	slab_allocator_free(sa, p);
	cr_assert_eq(slab_allocator_cur_slabs_allocated(sa), 0);

	release(arr);
}

Type(test_type);
#define test_type DefineType(test_type)

void test_type_cleanup(const test_type *ptr) {
	fam_release(ptr);
}

test_type test_type_create(int64 value) {
	Ptr ret = fam_alloc(sizeof(int64), false);
	if (ret == NULL)
		return ret;

	int64 *val = $(ret);
	*val = value;

	return ret;
}

int64 test_type_value(test_type tt) {
	int64 *val = $(tt);
	return *val;
}

MyTest(base, test_test_type) {
	test_type abc = test_type_create(1234);
	test_type def = test_type_create(5678);
	cr_assert_eq(test_type_value(abc), 1234);
	cr_assert_eq(test_type_value(def), 5678);
}

MyTest(base, test_object) {
	int64 v = 123;
	Object obj = object_create(ObjectTypeInt64, &v);
	int64 v_out = *(int64 *)object_value_of(obj);
	cr_assert_eq(v_out, 123);
	v = 456;
	object_mutate(obj, &v);
	v_out = *(int64 *)object_value_of(obj);
	cr_assert_eq(v_out, 456);

	Object obj2 = object_move(obj);

	v_out = *(int64 *)object_value_of(obj2);
	cr_assert_eq(v_out, 456);

	int v32_out = 0;
	Object obj3;
	{
		int v32 = 777;
		Object obj4 = object_create(ObjectTypeInt32, &v32);
		obj3 = object_ref(obj4);
		v32_out = *(int *)object_value_of(obj4);
		cr_assert_eq(v32_out, 777);
	}
	v32_out = *(int *)object_value_of(obj3);
	cr_assert_eq(v32_out, 777);

	int objval = 2024;
	Object obj5 = object_create(ObjectTypeInt32, &objval);
	Object weak5 = object_weak(obj5);
	Object upgraded = object_upgrade(weak5);
	cr_assert(!nil(upgraded));
	int objval_out = *(int *)object_value_of(upgraded);
	cr_assert_eq(objval_out, 2024);

	Object outer_weak;

	{
		int vw = 2025;
		Object inner_strong = object_create(ObjectTypeInt32, &vw);
		outer_weak = object_weak(inner_strong);
		Object upgrade1 = object_upgrade(outer_weak);
		int vw_out = $int(upgrade1);
		cr_assert_eq(vw_out, 2025);
	}

	// now try to upgrade after all strong references are gone (result is NULL)
	cr_assert(!object_upgrade(outer_weak));
	Object obj6 = object_create_box(100);
	// retreive the pointer
	Ptr inner_out = *(Ptr *)object_value_of(obj6);

	((char *)$(inner_out))[0] = 1;

	// do assertions
	cr_assert_eq(((char *)$(inner_out))[0], 1);

	/*
		// create two int64 objects
		Object obj1 = object_create_int64(111);
		Object obj2 = object_create_int64(222);

		// assert their values
		cr_assert_eq(object_as_int64(obj1), 111);
		cr_assert_eq(object_as_int64(obj2), 222);
		// try to get obj1 as int (wrong type)
		cr_assert_eq(object_as_int(obj1), -1);
		cr_assert_eq(fam_err, TypeMismatch);

		// create an int
		Object obj3 = object_create_int(444);
		// reset err and try to get it as an int64
		fam_err = NoErrors;
		cr_assert_eq(object_as_int64(obj3), -1);
		cr_assert_eq(fam_err, TypeMismatch);
		// get correctly as int
		cr_assert_eq(object_as_int(obj3), 444);

		// create a byte and do similar checks
		Object obj4 = object_create_byte('a');
		fam_err = NoErrors;
		cr_assert_eq(object_as_int64(obj4), -1);
		cr_assert_eq(fam_err, TypeMismatch);
		cr_assert_eq(object_as_byte(obj4), 'a');

		// create an int object
		const Object obj5 = object_create_int(123);
		cr_assert_eq(object_as_int(obj5), 123);
		// move obj5 -> obj6 (const allowed)
		const Object obj6 = object_move(obj5);
		// assert obj6 has the correct value
		cr_assert_eq(object_as_int(obj6), 123);
		// assert that obj5 has been consumed
		fam_err = NoErrors;
		cr_assert_eq(object_as_int(obj5), -1);
		cr_assert_eq(fam_err, ObjectConsumed);

		Object obj7 = object_create_int64(567);
		Object obj8 = object_move(obj7);

		cr_assert_eq(object_as_int64(obj8), 567);
		Object obj9 = null;

		Object obj11;
		{
			Object obj10 = object_create_int(1112);
			cr_assert_eq(object_as_int(obj10), 1112);
			obj11 = object_ref(obj10);
			const Object obj12 = object_ref(obj11);
		}
		cr_assert_eq(object_as_int(obj11), 1112);

		string s1 = string_create_cs("test111");
		Object obj12 = object_create_string(s1);

		string s2 = object_as_string(obj12);
		cr_assert(!strcmp(cstring(s1), cstring(s2)));
		cr_assert(!strcmp(cstring(s1), "test111"));

		Object obj13 = object_move(obj12);
	*/
}

/*
MyTest(base, test_string2) {
	string2 x = string2_create("test");
	cr_assert_eq(string2_len(x), 4);
	Ptr y = fam_alloc(4096, true);
	cr_assert(y != NULL);
	Ptr z = fam_alloc(4097, true);
	cr_assert(z != NULL);
	fam_release(&y);
	fam_release(&z);
}
*/

MyTest(base, test_limits) {
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
}
