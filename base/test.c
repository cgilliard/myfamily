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

void test_obj(bool send) {
	int64 v = 123;
	Object obj = object_create(ObjectTypeInt64, &v, send);
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
		Object obj4 = object_create(ObjectTypeInt32, &v32, send);
		obj3 = object_ref(obj4);
		v32_out = *(int *)object_value_of(obj4);
		cr_assert_eq(v32_out, 777);
	}
	v32_out = *(int *)object_value_of(obj3);
	cr_assert_eq(v32_out, 777);

	int objval = 2024;
	Object obj5 = object_create(ObjectTypeInt32, &objval, send);
	Object weak5 = object_weak(obj5);
	Object upgraded = object_upgrade(weak5);
	cr_assert(!nil(upgraded));
	int objval_out = *(int *)object_value_of(upgraded);
	cr_assert_eq(objval_out, 2024);
	Object outer_weak;

	{
		int vw = 2025;
		Object inner_strong = object_create(ObjectTypeInt32, &vw, send);
		outer_weak = object_weak(inner_strong);
		Object upgrade1 = object_upgrade(outer_weak);
		int vw_out = $int(upgrade1);
		cr_assert_eq(vw_out, 2025);
	}

	// now try to upgrade after all strong references are gone (result is NULL)
	cr_assert(!object_upgrade(outer_weak));

	Object obj6 = object_create_box(100, send);
	// retreive the pointer
	Ptr inner_out = *(Ptr *)object_value_of(obj6);

	((char *)$(inner_out))[0] = 1;

	// do assertions
	cr_assert_eq(((char *)$(inner_out))[0], 1);
}

MyTest(base, test_obj2) {
	int x1 = -1234;
	Object obj1 = object_create(ObjectTypeInt, &x1, false);

	int x1_out;
	object_value_of_buf(obj1, &x1_out, sizeof(int));
	cr_assert_eq(x1_out, -1234);

	Object obj2 = move(obj1);

	int x2_out;
	object_value_of_buf(obj2, &x2_out, sizeof(int));
	cr_assert_eq(x2_out, -1234);

	int x3 = 9999;
	int x3_out = 0;
	Object obj4 = null;

	{
		Object obj3 = object_create(ObjectTypeInt, &x3, false);
		object_value_of_buf(obj3, &x3_out, sizeof(int));
		cr_assert_eq(x3_out, 9999);
		obj4 = object_ref(obj3);

		cr_assert(object_weak(obj4) == NULL);
	}

	x3_out = 0;
	object_value_of_buf(obj4, &x3_out, sizeof(int));
	cr_assert_eq(x3_out, 9999);

	cr_assert_eq(object_type(obj1), -1);
	cr_assert_eq(object_type(obj2), ObjectTypeInt);
	cr_assert_eq(object_type(obj4), ObjectTypeInt);

	Object obj5 = null;
	float x5 = 1.2;
	float x5_out = 0.0;
	{
		Object obj6 = object_create(ObjectTypeFloat, &x5, false);
		cr_assert(4 == object_value_of_buf(obj6, &x5_out, sizeof(float)));
		cr_assert_eq(x5_out, 1.2f);
		obj5 = object_ref(obj6);

		cr_assert(object_weak(obj5) == NULL);
	}

	x5_out = 0.0;
	object_value_of_buf(obj5, &x5_out, sizeof(float));
	cr_assert_eq(x5_out, 1.2f);

	for (int i = -100000; i < 100000; i++) {
		Object obj = object_create(ObjectTypeInt, &i, true);
		int x_out;
		object_value_of_buf(obj, &x_out, sizeof(int));
		cr_assert_eq(x_out, i);
	}

	int v = INT_MAX;
	Object obj = object_create(ObjectTypeInt, &v, false);
	int v_out = 0;
	object_value_of_buf(obj, &v_out, sizeof(int));
	cr_assert_eq(v_out, INT_MAX);

	v = INT_MIN;
	Object objmin = object_create(ObjectTypeInt, &v, true);
	object_value_of_buf(objmin, &v_out, sizeof(int));
	cr_assert_eq(v_out, v);

	byte b1 = 'y';
	Object obj7 = object_create(ObjectTypeByte, &b1, true);
	byte b1_out = 0;
	object_value_of_buf(obj7, &b1_out, sizeof(byte));
	cr_assert_eq(b1, b1_out);

	bool bool1 = true;
	Object obj8 = object_create(ObjectTypeBool, &bool1, true);
	byte bool1_out = false;
	object_value_of_buf(obj8, &bool1_out, sizeof(bool));
	cr_assert_eq(bool1, bool1_out);

	bool bool2 = false;
	Object obj9 = object_create(ObjectTypeBool, &bool2, true);
	byte bool2_out = true;
	object_value_of_buf(obj9, &bool2_out, sizeof(bool));
	cr_assert_eq(bool2, bool2_out);

	Object obj10 = object_create_box(100, true);
	Ptr ptr;
	object_value_of_buf(obj10, &ptr, 8);
	for (int i = 0; i < 100; i++)
		((byte *)$(ptr))[i] = i;
	for (int i = 0; i < 100; i++)
		cr_assert_eq(((byte *)$(ptr))[i], i);

	Object obj11 = move(obj10);

	Ptr ptr2;
	object_value_of_buf(obj11, &ptr2, 8);

	for (int i = 0; i < 100; i++)
		cr_assert_eq(((byte *)$(ptr2))[i], i);

	// consumed
	Ptr ptr3 = null;
	object_value_of_buf(obj10, &ptr3, 8);
	cr_assert(nil(ptr3));

	cr_assert_eq(object_type(obj11), ObjectTypeBox);

	Ptr ptr4 = fam_resize(ptr2, 200);
	object_mutate(obj11, &ptr4);
}

MyTest(base, test_box) {
	// create a weak reference for later use
	Object weak;
	{
		// create a box of size 100 bytes send = false (using thread local allocation
		// and no atomic operations on reference counting
		Object test_box = object_create_box(100, false);

		// access the box's data via Ptr structure
		Ptr box_ptr;
		object_value_of_buf(test_box, &box_ptr, 8);

		// set data in the box to specified values
		for (int i = 0; i < 100; i++)
			((byte *)$(box_ptr))[i] = i;

		// resize the box
		Ptr resized_box = fam_resize(box_ptr, 200);
		cr_assert(!nil(resized_box));

		// update the data on the heap
		for (int i = 100; i < 200; i++)
			((byte *)$(resized_box))[i] = i;

		// update the box with the new pointer
		object_mutate(test_box, &resized_box);

		// create a weak reference
		weak = object_weak(test_box);

		// upgrade our weak reference and assert it's not nil
		Object upgrade = object_upgrade(weak);
		cr_assert(!nil(upgrade));

		// iterate through the upgraded box's data and assert values match up
		Ptr upgrade_ptr;
		object_value_of_buf(upgrade, &upgrade_ptr, 8);
		for (int i = 0; i < 200; i++)
			cr_assert_eq(((byte *)$(upgrade_ptr))[i], i);
	}
	// Cleanup occurs as strong references go out of scope

	// upgrade returns nil now
	Object upgrade = object_upgrade(weak);
	cr_assert(nil(upgrade));
}

MyTest(base, test_object) {
	test_obj(true);
	test_obj(false);
}

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
