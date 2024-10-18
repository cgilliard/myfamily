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
#include <base/macro_utils.h>
#include <crypto/psrng.h>
#include <math.h>
#include <string.h>
#include <util/object.h>
#include <util/orbtree.h>
#include <util/rbtree.h>
#include <util/test.h>

MySuite(util);

int u64_compare(const void *v1, const void *v2) {
	const u64 *k1 = v1;
	const u64 *k2 = v2;
	if (*k1 < *k2)
		return -1;
	else if (*k1 > *k2)
		return 1;
	return 0;
}

MyTest(util, test_rbtree) {
	RBTree test1 = INIT_RBTREE;
	cr_assert(!RBTreeIsInit(test1));
	u64 k, v;
	rbtree_create(&test1, sizeof(u64), sizeof(u64), &u64_compare, false);

	cr_assert(RBTreeIsInit(test1));

	k = 0;
	v = 10;
	rbtree_put(&test1, &k, &v);

	k = 1;
	v = 11;
	rbtree_put(&test1, &k, &v);

	k = 2;
	v = 12;
	rbtree_put(&test1, &k, &v);

	k = 3;
	v = 13;
	rbtree_put(&test1, &k, &v);

	k = 0;
	const u64 *value1 = rbtree_get(&test1, &k);
	cr_assert_eq(*value1, 10);

	k = 1;
	const u64 *value2 = rbtree_get(&test1, &k);
	cr_assert_eq(*value2, 11);

	k = 2;
	const u64 *value3 = rbtree_get(&test1, &k);
	cr_assert_eq(*value3, 12);

	k = 3;
	const u64 *value4 = rbtree_get(&test1, &k);
	cr_assert_eq(*value4, 13);

	k = 4;
	const u64 *value5 = rbtree_get(&test1, &k);
	cr_assert_eq(value5, NULL);

	cr_assert_eq(rbtree_size(&test1), 4);
}

MyTest(util, validate_rbtree) {
	RBTree valid1;
	u64 k, v;
	cr_assert(!rbtree_create(&valid1, sizeof(u64), sizeof(u64), u64_compare, false));

	u64 max = 10;
	for (u64 i = 0; i < max; i++) {
		k = i;
		v = i + 10;
		cr_assert(!rbtree_put(&valid1, &k, &v));
		rbtree_validate(&valid1);
	}
	rbtree_validate(&valid1);

	for (u64 i = 0; i < max; i++) {
		k = i;
		v = i + 10;
		const u64 *value = rbtree_get(&valid1, &k);
		cr_assert_eq(*value, v);
	}

	for (u64 i = 0; i < max; i++) {
		k = i;
		v = *(u64 *)rbtree_get(&valid1, &k);
		cr_assert_eq(v, k + 10);
		cr_assert(!rbtree_remove(&valid1, &k, NULL));
		cr_assert_eq(rbtree_size(&valid1), (max - 1) - i);
		rbtree_validate(&valid1);
	}
}

MyTest(util, test_random_rbtree) {
	// seed rng for reproducibility
	u8 key[32] = {7};
	u8 iv[16] = {};
	psrng_test_seed(iv, key);
	RBTree rand1;
	cr_assert(!rbtree_create(&rand1, sizeof(u64), sizeof(u64), u64_compare, false));
	u64 size = 4096;
	u64 arr[size];
	for (u64 i = 0; i < size; i++) {
		arr[i] = 0; // ensure zeroed before calling rng for reproducibility.
		psrng_rand_u64(&arr[i]);
		arr[i] %= INT64_MAX;
		u64 k = arr[i];
		u64 v = k + 100;
		cr_assert(!rbtree_put(&rand1, &k, &v));
		rbtree_validate(&rand1);
		rbtree_max_depth(&rand1);
	}

	for (u64 i = 0; i < size; i++) {
		const u64 *value = rbtree_get(&rand1, &arr[i]);
		cr_assert_eq(*value, arr[i] + 100);
	}

	RBTreeIterator itt1;
	cr_assert(!rbtree_iterator(&rand1, &itt1, NULL, false, NULL, false));

	u64 i = 0;
	u64 last = 0;
	loop {
		RbTreeKeyValue kv;
		bool has_next = rbtree_iterator_next(&itt1, &kv);
		if (!has_next)
			break;
		i++;

		u64 *k1 = kv.key;
		u64 *v1 = kv.value;
		cr_assert(last < *k1);
		last = *k1;
		cr_assert_eq(*k1 + 100, *v1);
	}
	cr_assert_eq(i, size);

	cr_assert_eq(rbtree_size(&rand1), size);

	int depth = rbtree_max_depth(&rand1);
	i = 0;

	loop {
		if (i == size)
			break;
		cr_assert(!rbtree_remove(&rand1, &arr[i], NULL));
		cr_assert_eq(rbtree_size(&rand1), (size - 1) - i);
		rbtree_validate(&rand1);
		i++;
	}
	cr_assert_eq(rbtree_size(&rand1), 0);
}

MyTest(util, test_validation_and_other) {
	FatPtr p;
	fam_alloc(&p, 1);
	fam_free(&p);
	RBTree test1;
	cr_assert(rbtree_create(NULL, 10, 10, u64_compare, false));
	__is_debug_malloc = true;
	cr_assert(rbtree_create(&test1, sizeof(u64), sizeof(u64), u64_compare, false));
	__is_debug_malloc = false;
	test1.impl = null;

	cr_assert(!rbtree_create(&test1, sizeof(u64), sizeof(u64), u64_compare, false));
	__is_debug_malloc = true;
	u64 k1 = 0;
	u64 v1 = 0;
	cr_assert(rbtree_put(&test1, &k1, &v1));
	__is_debug_malloc = false;
	cr_assert(rbtree_put(NULL, &k1, &v1));
	cr_assert(rbtree_size(NULL));
	cr_assert(!rbtree_put(&test1, &k1, &v1));
	k1 = 1;
	v1 = 0;
	cr_assert(!rbtree_put(&test1, &k1, &v1));
	cr_assert_eq(*(u64 *)rbtree_get(&test1, &k1), 0);
	v1 = 123;
	cr_assert(!rbtree_put(&test1, &k1, &v1));
	cr_assert_eq(*(u64 *)rbtree_get(&test1, &k1), 123);
	k1 = 10;
	v1 = 0;
	cr_assert(rbtree_remove(&test1, &k1, NULL));
	cr_assert(!rbtree_get(NULL, NULL));
	cr_assert_eq(rbtree_max_depth(&test1), 2);
	rbtree_print(&test1);
	cr_assert(rbtree_iterator(NULL, NULL, NULL, false, NULL, false));
	rbtree_remove(NULL, NULL, NULL);
}

MyTest(util, test_rbtree_random_ordered_insert_delete) {
	// seed rng for reproducibility
	u8 key[32] = {1};
	u8 iv[16] = {};
	psrng_test_seed(iv, key);

	RBTree rand1;
	cr_assert(!rbtree_create(&rand1, sizeof(u64), sizeof(u64), u64_compare, false));
	u64 size = 10000;
	u64 arr[size];
	u64 arr_index = 0;
	u64 delete_index = 0;
	u64 i = 0;

	u8 opcode = 0;
	loop {
		if (++i == size)
			break;
		psrng_rand_u8(&opcode);
		if (opcode % 3 == 0 || opcode % 3 == 1 || delete_index >= arr_index) {
			arr[arr_index] = 0;
			psrng_rand_u64(&arr[arr_index]);
			u64 v = arr[arr_index] + 123;
			cr_assert(!rbtree_put(&rand1, &arr[arr_index], &v));
			arr_index++;
		} else {
			cr_assert(!rbtree_remove(&rand1, &arr[delete_index], NULL));

			delete_index++;
		}
		u64 cur_size = rbtree_size(&rand1);
		u64 max_depth = rbtree_max_depth(&rand1);
		u64 max_valid_depth = 1;
		// unfortunately there are some problems linking math.h (disable for now)
		/*
			if (cur_size)
				max_valid_depth = (log2(cur_size) * 2) + 1;
			// max theoretical size of a valid rbtree is 2 log(n) + 1. Ensure we're no worse
			// than that. cr_assert(max_valid_depth >= max_depth);
		*/
		cr_assert_eq(cur_size, arr_index - delete_index);
		rbtree_validate(&rand1);
	}
}

MyTest(util, test_rbtree_range_itt) {
	RBTree tree1;
	cr_assert(!rbtree_create(&tree1, sizeof(u64), sizeof(u64), u64_compare, false));
	u64 size = 10;
	u64 arr[size];
	for (u64 i = 0; i < size; i++) {
		arr[i] = (i * 3) + 1;
		u64 k = arr[i];
		u64 v = k + 100;
		cr_assert(!rbtree_put(&tree1, &k, &v));
		rbtree_validate(&tree1);
		rbtree_max_depth(&tree1);
	}

	for (u64 i = 0; i < size; i++) {
		const u64 *value = rbtree_get(&tree1, &arr[i]);
		cr_assert_eq(*value, arr[i] + 100);
	}

	RBTreeIterator itt1;
	u64 start = 14;
	u64 end = 20;
	cr_assert(!rbtree_iterator(&tree1, &itt1, &start, true, &end, true));

	u64 i = 0;
	u64 last = 15;
	loop {
		RbTreeKeyValue kv;
		bool has_next = rbtree_iterator_next(&itt1, &kv);
		if (!has_next)
			break;
		i++;

		u64 *k1 = kv.key;
		u64 *v1 = kv.value;
		cr_assert(last < *k1);
		last = *k1;
		cr_assert_eq(*k1 + 100, *v1);
	}
	cr_assert_eq(last, 19);
	// we find 16 and 19
	cr_assert_eq(i, 2);

	cr_assert_eq(rbtree_size(&tree1), size);
}

typedef struct MyRecord {
	char name[100];
	u64 value;
} MyRecord;

int my_record_compare(const void *v1, const void *v2) {
	const MyRecord *k1 = v1;
	const MyRecord *k2 = v2;
	return strcmp(k1->name, k2->name);
}

MyTest(util, test_string_compare_fn) {
	RBTree tree1;
	cr_assert(!rbtree_create(&tree1, sizeof(MyRecord), sizeof(u64), my_record_compare, false));
	u64 size = 10;
	for (u64 i = 0; i < size; i++) {
		char s[101];
		snprintf(s, 100, "test%llu", i);
		MyRecord k;
		strcpy(k.name, s);
		k.value = i;
		u64 v = k.value + 100;
		cr_assert(!rbtree_put(&tree1, &k, &v));
		rbtree_validate(&tree1);
		rbtree_max_depth(&tree1);
	}

	RBTreeIterator itt1;
	cr_assert(!rbtree_iterator(&tree1, &itt1, NULL, false, NULL, false));
	int i = 0;
	loop {
		RbTreeKeyValue kv;
		bool has_next = rbtree_iterator_next(&itt1, &kv);
		if (!has_next)
			break;

		MyRecord *k1 = kv.key;
		u64 *v1 = kv.value;
		cr_assert_eq(*v1, i + 100);
		i++;
	}
	cr_assert_eq(i, size);
}

int cleanup_u64_count = 0;

void cleanup_u64(void *value) {
	cr_assert_eq(*(u64 *)value, 101);
	cleanup_u64_count++;
}

MyTest(util, test_rc) {
	{
		Weak weak1;
		{

			// outer reference
			Rc outer1;
			{
				// data
				u64 my_data = 101;
				// inner reference
				Rc rc1;
				// build rc
				rc_build(&rc1, &my_data, sizeof(u64), false, cleanup_u64);
				// clone to outer reference
				rc_clone(&outer1, &rc1);
				rc_weak(&weak1, &outer1);
			}
			// even though rc1 has been cleaned up, outer still lives on
			u64 *outer_value = rc_access(&outer1);
			cr_assert_eq(*outer_value, 101);

			// no cleanup occurred yet
			cr_assert_eq(cleanup_u64_count, 0);
		}
		// cleanup occurs here as all strongs gone
		cr_assert_eq(cleanup_u64_count, 1);
		// cleanup occurs here (for weak)
	}
	// verify incrememented count not higher after weak exits
	cr_assert_eq(cleanup_u64_count, 1);
}

int cleanup2_count = 0;
void cleanup_u642(void *value) {
	cr_assert_eq(*(u64 *)value, 202);
	cleanup2_count++;
}

MyTest(util, test_rc_upgrade) {
	Weak weak1;
	{
		Rc rc1;
		Rc rc2;
		Rc rc3;

		u64 my_data = 202;
		rc_build(&rc1, &my_data, sizeof(u64), false, cleanup_u642);
		rc_clone(&rc2, &rc1);
		rc_weak(&weak1, &rc2);

		rc_upgrade(&rc3, &weak1);
		u64 *upgrade_value = rc_access(&rc3);
		cr_assert_eq(*upgrade_value, 202);

		cr_assert_eq(cleanup2_count, 0);
	}
	cr_assert_eq(cleanup2_count, 1);
}

MyTest(util, test_atomic_rc_upgrade) {
	Weak weak1;
	{
		Rc rc1;
		Rc rc2;
		Rc rc3;

		u64 my_data = 202;
		rc_build(&rc1, &my_data, sizeof(u64), true, cleanup_u642);
		rc_clone(&rc2, &rc1);
		rc_weak(&weak1, &rc2);

		rc_upgrade(&rc3, &weak1);
		u64 *upgrade_value = rc_access(&rc3);
		cr_assert_eq(*upgrade_value, 202);

		cr_assert_eq(cleanup2_count, 0);
	}
	cr_assert_eq(cleanup2_count, 1);
}

MyTest(util, test_weak_cleanup) {
	{
		Weak weak1;
		{
			Rc rc1;
			Rc rc2;
			u64 my_data = 202;
			rc_build(&rc1, &my_data, sizeof(u64), true, cleanup_u642);
			rc_clone(&rc2, &rc1);
			rc_weak(&weak1, &rc2);

			u64 *upgrade_value = rc_access(&rc2);
			cr_assert_eq(*upgrade_value, 202);

			cr_assert_eq(cleanup2_count, 0);
		}
		// last ref is a weak so cleanup should be called by now
		cr_assert_eq(cleanup2_count, 1);
		Rc rc;
		cr_assert(rc_upgrade(&rc, &weak1));
		rc.impl = null;
	}
	// assert still 1
	cr_assert_eq(cleanup2_count, 1);
}

int drop_count = 0;

void my_cleanup(u64 *obj) {
	printf("my cleanup obj v=%llu\n", *obj);
	cr_assert_eq(*obj, 8);
	drop_count++;
}

MyTest(util, test_print_err) {
	// cycle through and print all errors
	for (int i = 0; i < FamErrCount; i++) {
		SetErr(i);
		print_err("generated error");
	}
	// reset fam_err to indicate no error
	fam_err = NoErrors;

	// output of test:
	// NoErrors: generated error
	// IllegalArgument: generated error
	// AllocError: generated error
	// InitErr: generated error
	// AlreadyInitialized: generated error
	// IndexOutOfBounds: generated error
	// IllegalState: generated error
	// TooBig: generated error
	// ResourceNotAvailable: generated error
	// Permission: generated error
}

int drop_count2 = 0;

void my_cleanup2(u64 *obj) {
	printf("my cleanup 2 v=%llu\n", *obj);
	drop_count2++;
}

/*
MyTest(util, test_ref) {
	{
		Object test1 = INIT_OBJECT;
		Object test3 = INIT_OBJECT;
		FatPtr fptr;
		fam_alloc(&fptr, sizeof(u64));
		u64 v = 10;
		*(u64 *)$(fptr) = v;

		{
			Object test2 = INIT_OBJECT;
			cr_assert(!object_create(&test2, false));
			cr_assert(!object_ref(&test1, &test2));
			cr_assert(object_mut_ref(&test3, &test1));
			cr_assert(!object_mut_ref(&test3, &test2));
			cr_assert_eq(drop_count2, 0);
		}
		// still not dropped because there is a remaining reference (test1).
		cr_assert_eq(drop_count2, 0);
	}
	// now test1 dropped and the rc is dropped.
	cr_assert_eq(drop_count2, 1);
}
*/

MyTest(util, test_object) {
	Object test1 = NIL;
	cr_assert(!object_create(&test1, false, ObjectTypeObject, NULL));
	Object test2 = NIL;
	cr_assert(!object_create(&test2, false, ObjectTypeString, "this is a test"));
	Object test3 = NIL;
	u64 v = 1234;
	cr_assert(!object_create(&test3, true, ObjectTypeU64, &v));
	Object test4 = NIL;
	cr_assert(!object_create(&test4, false, ObjectTypeString, "another test"));

	cr_assert(!strcmp(object_as_string(&test2), "this is a test"));
	u64 test3_out = 0;
	cr_assert(!object_as_u64(&test3, &test3_out));
	cr_assert_eq(test3_out, 1234);
	cr_assert_eq(object_as_string(&test1), NULL);
	cr_assert_eq(object_as_string(&test3), NULL);

	cr_assert(object_as_u64(&test1, &test3_out));
	cr_assert(object_as_u64(&test2, &test3_out));

	object_set_property(&test1, "aaa", &test2);
	object_set_property(&test1, "bbb", &test3);

	const Object test1_out = object_get_property(&test1, "aaa");
	const Object test1_out2 = object_get_property(&test1, "bbb");
	u64 test1_out2_u64 = 0;
	object_as_u64(&test1_out2, &test1_out2_u64);
	cr_assert_eq(test1_out2_u64, 1234);
	cr_assert(!strcmp(object_as_string(&test1_out), "this is a test"));

	object_set_property(&test1, "aaa", &test4);
	const Object test1_out3 = object_get_property(&test1, "aaa");
	cr_assert(!strcmp(object_as_string(&test1_out3), "another test"));

	Object x1 = NIL;
	cr_assert(!object_create(&x1, false, ObjectTypeObject, NULL));
	Object x2 = NIL;
	cr_assert(!object_create(&x2, false, ObjectTypeObject, NULL));
	Object x3 = NIL;
	cr_assert(!object_create(&x3, false, ObjectTypeObject, NULL));
	Object text1 = NIL;
	cr_assert(!object_create(&text1, false, ObjectTypeString, "text1"));

	object_set_property(&x3, "text", &text1);
	object_set_property(&x2, "x3", &x3);
	object_set_property(&x1, "x2", &x2);

	const Object x2_out = object_get_property(&x1, "x2");
	const Object x3_out = object_get_property(&x2_out, "x3");
	const Object text_out = object_get_property(&x3_out, "text");
	cr_assert(!strcmp(object_as_string(&text_out), "text1"));
}

MyTest(util, test_overwrite) {
	Object test1 = NIL;
	cr_assert(!object_create(&test1, false, ObjectTypeObject, NULL));
	Object test2 = NIL;
	cr_assert(!object_create(&test2, false, ObjectTypeString, "this is a test"));
	Object test3 = NIL;
	cr_assert(!object_create(&test3, false, ObjectTypeString, "another test"));
	Object test4 = NIL;
	cr_assert(!object_create(&test4, false, ObjectTypeString, "another test2"));

	object_set_property(&test1, "aaa", &test2);
	const Object x2_out = object_get_property(&test1, "aaa");
	cr_assert(!strcmp(object_as_string(&x2_out), "this is a test"));

	object_set_property(&test1, "aaa", &test3);
	const Object x3_out = object_get_property(&test1, "aaa");
	cr_assert(!strcmp(object_as_string(&x3_out), "another test"));

	object_set_property(&test1, "aaa", &test4);
	const Object x4_out = object_get_property(&test1, "aaa");
	cr_assert(!strcmp(object_as_string(&x4_out), "another test2"));

	Object x1 = NIL;
	cr_assert(!object_create(&x1, false, ObjectTypeObject, NULL));

	Object x2 = NIL;
	u64 x2_val = 10;
	cr_assert(!object_create(&x2, false, ObjectTypeU64, &x2_val));
	Object x3 = NIL;
	u64 x3_val = 20;
	cr_assert(!object_create(&x3, false, ObjectTypeU64, &x3_val));
	Object x4 = NIL;
	u64 x4_val = 30;
	cr_assert(!object_create(&x4, false, ObjectTypeU64, &x4_val));

	object_set_property(&x1, "aaa", &x2);
	const Object xx2_out = object_get_property(&x1, "aaa");
	u64 xx2_out_val = 0;
	object_as_u64(&xx2_out, &xx2_out_val);
	cr_assert_eq(xx2_out_val, 10);

	object_set_property(&x1, "aaa", &x3);
	const Object xx3_out = object_get_property(&x1, "aaa");
	u64 xx3_out_val = 0;
	object_as_u64(&xx3_out, &xx3_out_val);
	cr_assert_eq(xx3_out_val, 20);

	object_set_property(&x1, "aaa", &x4);
	const Object xx4_out = object_get_property(&x1, "aaa");
	u64 xx4_out_val = 0;
	object_as_u64(&xx4_out, &xx4_out_val);
	cr_assert_eq(xx4_out_val, 30);
}

MyTest(util, test_object_macros) {
	// create an empty object
	var x = $();

	// create a string object
	let y = $("abc");

	// set the 'test' property of x to object y
	$(x, "test", y);
	$(x, "test9", "ok");

	// retrieve the 'test' property of x, store in object out
	let out = $(x, "test");
	// compare out (as a string) to "abc", assert equal
	cr_assert(!strcmp($string(out), "abc"));

	// create a u64 value
	u64 v1 = 1234;

	// set the 'test2' property of x to v1
	$(x, "test2", v1);
	// retrieve the 'test2' property of x, store in object out2
	let out2 = $(x, "test2");
	cr_assert(!nil(out2));
	// assert that out2 as u64 is 1234
	cr_assert_eq($u64(out2), 1234);

	var v = $(x, "aadf");
	cr_assert(nil(v));

	let x0 = $("testing123");
	var x1 = $();
	var x2 = $();
	var x3 = $();
	var x4 = $();

	$(x1, "x0", x0);
	$(x2, "x1", x1);
	$(x3, "x2", x2);
	$(x4, "x3", x3);
	// x3 has been consumed. If uncommented, this will generate a runtime error
	// $(x3, "test", "test");

	let x3_out = $(x4, "x3");
	let x2_out = $(x3_out, "x2");
	let x1_out = $(x2_out, "x1");
	let x0_out = $(x1_out, "x0");
	cr_assert(!strcmp($string(x0_out), "testing123"));
}

MyTest(util, test_move_ref) {
	var y;
	{
		let x = $("test");
		y = $ref(x);
		cr_assert(!strcmp($string(x), "test"));
	}
	cr_assert(!strcmp($string(y), "test"));

	var a;
	{
		let b = $("abc");
		a = $move(b);
	}
	cr_assert(!strcmp($string(a), "abc"));
}

MyTest(util, test_ok_nil) {
	let a = Ok;
	let b = NIL;
	cr_assert(!nil(a));
	cr_assert(nil(b));
}

MyTest(util, test_remove_property) {
	var x1 = $();
	$(x1, "test", "test");
	$(x1, "test2", "test2");
	$remove(x1, "ajsdl");
	$remove(x1, "test");
	let xtest = $(x1, "test");
	cr_assert(nil(xtest));
	let x2test = $(x1, "test2");
	cr_assert(!nil(x2test));

	/*

		var x1 = $();
		$(x1, "key1", "value1");	// set property key1 -> value1 (index 0)
		$(x1, "key2", "value2");	// set property key2 -> value2 (index 1)
		$(x1, 0, "replacedvalue1"); // set the first value (based on insertion order) to
	   replacedvalue1. That would mean key1 -> replacedvalue1.
		$(x1, 1, "replacedvalue2"); // set the first value (based on insertion order) to
	   replacedvalue2. That would mean key2 -> replacedvalue2.
		$(x1, "key3", "value3");	// set property key3 -> value3 (index 2)
		$insert_after(x1, 1, "key2.5", "value2.5"); // inserts the k/v pair after index 1 key2.5 ->
	   value 2.5, shifting all other entries by 1. $insert_before(x1, 0, "key0", "value0"); //
	   inserts the k/v pair before index 0, shifting all other entries by 1.

		var x1 = $();
			x1.key1 = "value1";        // set property key1 -> value1 (index 0)
			x1.key2 = "value2";        // set property key2 -> value2 (index 1)
			x1[0] = "replacedvalue1"; // set the first value (based on insertion order) to
	   replacedvalue1. That would mean key1 -> replacedvalue1. x1[1] = "replacedvalue2"; // set the
	   first value (based on insertion order) to replacedvalue2. That would mean key2 ->
	   replacedvalue2. x1.key3 = "value3";        // set property key3 -> value3 (index 2) x1[1] >=
	   ("key2.5", "value2.5"); // inserts the k/v pair after index 1 key2.5 -> value 2.5, shifting
	   all other entries by 1. x1[0] <= ("key0", "value0"); // inserts the k/v pair before index 0,
	   shifting all other entries by 1.

		typedef struct Property {
			void *key;
			ObjectNc value;
		} Property;

		// a regular (simple) version with string properties
		var x2 = $();
		$(x1, "key1", "value1");
		$(x2, "key2", "value2");

		// with a custom sort function (my_sort) compares the void *key data in SortableKey (like
	   qsort) var x3 = $(with(sort, my_sort)); Property p1, p2; p1.key = &some_key; p1.value =
	   &some_value; p2.key = &some_key2; p2.value = &some_value2;

		// insert kv/pairs.
		$(x1, p1);
		$(x2, p2);

		// for each
		foreach (k, v, x1) {
			debug("k={},v={}", k, v);
		}

		var x = $();
		x1["testa"] = 123;
		x1.push("test");
		x1.push(64ULL);
		x1.pop();
		x1[0] = "hi";
		foreach (x, x1) {
			debug("x={}", x);
		}
	*/
}

typedef struct OrbTreeData {
	u64 key;
	u64 value;
} OrbTreeData;

int compare_orbs(const void *k1, const void *k2) {
	const OrbTreeData *k1o = k1;
	const OrbTreeData *k2o = k2;

	if (k1o->key < k2o->key)
		return -1;
	else if (k1o->key > k2o->key)
		return 1;
	return 0;
}

MyTest(util, test_orbtree) {
	ORBTree tree1;
	orbtree_create(&tree1, sizeof(OrbTreeData), compare_orbs);
	ORBTreeTray tray;
	for (int i = 0; i < 1500; i++)
		orbtree_allocate_tray(&tree1, &tray);

	orbtree_allocate_tray(&tree1, &tray);

	orbtree_deallocate_tray(&tree1, &tray);
	tray.id = 1300;
	orbtree_deallocate_tray(&tree1, &tray);

	tray.id = 0;
	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 1300);
	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 1500);
	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 1501);

	tray.id = 1200;
	orbtree_deallocate_tray(&tree1, &tray);
	tray.id = 1204;
	orbtree_deallocate_tray(&tree1, &tray);
	tray.id = 1208;
	orbtree_deallocate_tray(&tree1, &tray);
	tray.id = 16;
	orbtree_deallocate_tray(&tree1, &tray);

	tray.id = 0;
	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 16);

	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 1208);

	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 1204);

	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 1200);

	orbtree_allocate_tray(&tree1, &tray);
	cr_assert_eq(tray.id, 1502);

	ORBTreeTray rep;
	OrbTreeData *otd = tray.value;
	otd->key = 1;
	otd->value = 2;
	orbtree_put(&tree1, &tray, &rep);
	orbtree_allocate_tray(&tree1, &tray);
	otd = tray.value;
	otd->key = 3;
	otd->value = 4;
	orbtree_put(&tree1, &tray, &rep);
	orbtree_allocate_tray(&tree1, &tray);
	otd = tray.value;
	otd->key = 0;
	otd->value = 5;
	orbtree_put(&tree1, &tray, &rep);

	orbtree_allocate_tray(&tree1, &tray);
	otd = tray.value;
	otd->key = 10;
	otd->value = 50;
	orbtree_put(&tree1, &tray, &rep);

	orbtree_print(&tree1);
	orbtree_validate(&tree1);

	orbtree_allocate_tray(&tree1, &tray);
	otd = tray.value;
	otd->key = 30;
	otd->value = 50;
	orbtree_put(&tree1, &tray, &rep);

	orbtree_print(&tree1);
	orbtree_validate(&tree1);

	orbtree_allocate_tray(&tree1, &tray);
	otd = tray.value;
	otd->key = 40;
	otd->value = 50;
	orbtree_put(&tree1, &tray, &rep);

	orbtree_print(&tree1);
	orbtree_validate(&tree1);

	ORBTreeTray find;
	find.value = &otd;
	otd->key = 40;
	rep.updated = false;
	cr_assert(!orbtree_get(&tree1, otd, &rep));
	cr_assert_eq(rep.updated, true);
	OrbTreeData x = *(OrbTreeData *)rep.value;
	cr_assert_eq(x.value, 50);
}

MyTest(util, test_orbtree_loop) {
	// seed rng for reproducibility
	u8 key[32] = {9};
	u8 iv[16] = {};
	psrng_test_seed(iv, key);

	ORBTree tree1;
	orbtree_create(&tree1, sizeof(OrbTreeData), compare_orbs);
	int size = 1000;
	ORBTreeTray tray;
	ORBTreeTray ret;
	OrbTreeData *otd;
	u64 arr[size];

	for (int i = 0; i < size; i++) {
		cr_assert(!orbtree_allocate_tray(&tree1, &tray));
		otd = tray.value;
		arr[i] = 0;
		psrng_rand_u64(&arr[i]);
		otd->key = arr[i];
		otd->value = otd->key + 7;
		cr_assert(!orbtree_put(&tree1, &tray, &ret));
		// orbtree_print(&tree1);
		orbtree_validate(&tree1);
	}

	OrbTreeData search;
	for (int i = 0; i < size; i++) {
		search.key = arr[i];
		ret.updated = false;
		cr_assert(!orbtree_get(&tree1, &search, &ret));
		cr_assert(ret.updated);
		otd = ret.value;
		cr_assert_eq(otd->key, arr[i]);
		cr_assert_eq(otd->value, arr[i] + 7);
	}
	cr_assert_eq(orbtree_size(&tree1), size);
	orbtree_validate(&tree1);

	for (int i = 0; i < size; i++) {
		search.key = arr[i];
		ret.updated = false;
		cr_assert(!orbtree_remove(&tree1, &search, &ret));
		orbtree_deallocate_tray(&tree1, &ret);
		cr_assert(orbtree_remove(&tree1, &search, &ret));
		orbtree_validate(&tree1);
	}
	search.key = UINT64_MAX;
	cr_assert(orbtree_remove(&tree1, &search, &ret));

	cr_assert_eq(orbtree_size(&tree1), 0);
}

typedef struct ObjTreeData {
	u64 namespace;
	u64 seqno;
	u64 key;
	u64 value;
} ObjTreeData;

int compare_objs(const void *k1, const void *k2) {
	const ObjTreeData *o1 = k1;
	const ObjTreeData *o2 = k2;
	if (o1->namespace > o2->namespace)
		return 1;
	if (o1->namespace < o2->namespace)
		return -1;
	if (o1->key > o2->key)
		return 1;
	if (o1->key < o2->key)
		return -1;
	return 0;
}

int compare_seq_objs(const void *k1, const void *k2) {
	const ObjTreeData *o1 = k1;
	const ObjTreeData *o2 = k2;
	if (o1->namespace > o2->namespace)
		return 1;
	if (o1->namespace < o2->namespace)
		return -1;
	if (o1->seqno > o2->seqno)
		return 1;
	if (o1->seqno < o2->seqno)
		return -1;
	return 0;
}

MyTest(util, test_dual_trees) {
	// seed rng for reproducibility
	u8 key[32] = {8};
	u8 iv[16] = {};
	psrng_test_seed(iv, key);

	ORBTree tree1, tree2;
	orbtree_create(&tree1, sizeof(ObjTreeData), compare_seq_objs);
	orbtree_create(&tree2, sizeof(OrbTreeData), compare_objs);
	int size = 10;
	ORBTreeTray tray;
	ORBTreeTray ret;
	ObjTreeData *otd;
	u64 arr[size];

	for (int i = 0; i < size; i++) {
		cr_assert(!orbtree_allocate_tray(&tree1, &tray));
		otd = tray.value;
		arr[i] = 0;
		psrng_rand_u64(&arr[i]);
		otd->namespace = arr[i];
		otd->value = otd->namespace + 7;
		otd->seqno = 0;
		cr_assert(!orbtree_put(&tree1, &tray, &ret));

		// cr_assert(!orbtree_allocate_tray(&tree2, &tray));
		otd = tray.value;
		otd->namespace = arr[i];
		otd->seqno = 0;
		otd->value = otd->namespace + 7;

		// cr_assert(!orbtree_put(&tree2, &tray, &ret));
		orbtree_validate(&tree1);
		//  orbtree_validate(&tree2);
	}

	orbtree_validate(&tree1);
	orbtree_print(&tree1);

	ObjTreeData search;
	for (int i = 0; i < size; i++) {
		search.namespace = arr[i];
		search.seqno = 0;
		ret.updated = false;
		cr_assert(!orbtree_get(&tree1, &search, &ret));
		cr_assert(ret.updated);
		otd = ret.value;
		cr_assert_eq(otd->namespace, arr[i]);
		cr_assert_eq(otd->value, arr[i] + 7);
	}

	for (int i = 0; i < size; i++) {
		search.namespace = arr[i];
		search.seqno = 0;
		ret.updated = false;
		cr_assert(!orbtree_remove(&tree1, &search, &ret));
		orbtree_deallocate_tray(&tree1, &ret);
		cr_assert(orbtree_remove(&tree1, &search, &ret));
		orbtree_validate(&tree1);
	}
	search.key = UINT64_MAX;
	cr_assert(orbtree_remove(&tree1, &search, &ret));

	cr_assert_eq(orbtree_size(&tree1), 0);
}

MyTest(util, test_orbtree_index) {
	ORBTree tree1;
	ORBTreeTray tray, ret;
	orbtree_create(&tree1, sizeof(u64), u64_compare);
	u64 size = 10;

	for (u64 i = 0; i < size; i++) {
		cr_assert(!orbtree_allocate_tray(&tree1, &tray));
		u64 *v = tray.value;
		*v = i;
		cr_assert(!orbtree_put(&tree1, &tray, &ret));
		orbtree_validate(&tree1);
	}

	orbtree_print(&tree1);

	for (u32 i = 0; i < size; i++) {
		ret.updated = false;
		cr_assert(!orbtree_get_index(&tree1, i, &ret));
		cr_assert(ret.updated);
		cr_assert_eq(*(u64 *)ret.value, i);
	}

	cr_assert(orbtree_get_index(&tree1, size, &ret));
	SetErr(NoErrors);

	for (u32 i = 0; i < size; i++)
		cr_assert(!orbtree_remove_index(&tree1, 0, &ret));
	cr_assert(orbtree_size(&tree1) == 0);
}

MyTest(util, test_orbtree_iterator) {
	ORBTree tree1;
	ORBTreeTray tray, ret;
	orbtree_create(&tree1, sizeof(u64), u64_compare);
	u64 size = 1000;

	for (u64 i = 0; i < size; i++) {
		cr_assert(!orbtree_allocate_tray(&tree1, &tray));
		u64 *v = tray.value;
		*v = i;
		cr_assert(!orbtree_put(&tree1, &tray, &ret));
		orbtree_validate(&tree1);
	}

	ORBTreeIterator itt1;
	cr_assert(!orbtree_iterator(&tree1, &itt1, NULL, true, NULL, true, false));

	u64 count = 0;
	while (orbtree_iterator_next(&itt1, &tray)) {
		cr_assert_eq(count++, *(u64 *)tray.value);
	}

	for (u64 i = 0; i < 100; i++) {
		u64 start = 200 + i;
		u64 end = 500 + i;
		cr_assert(!orbtree_iterator_reset(&tree1, &itt1, &start, true, &end, true));

		count = start;
		while (orbtree_iterator_next(&itt1, &tray)) {
			cr_assert_eq(count++, *(u64 *)tray.value);
		}
		cr_assert_eq(count, end + 1);

		cr_assert(!orbtree_iterator_reset(&tree1, &itt1, &start, true, &end, false));

		count = start;
		while (orbtree_iterator_next(&itt1, &tray)) {
			cr_assert_eq(count++, *(u64 *)tray.value);
		}

		cr_assert_eq(count, end);

		cr_assert(!orbtree_iterator_reset(&tree1, &itt1, &start, false, &end, false));
		count = start + 1;
		while (orbtree_iterator_next(&itt1, &tray)) {
			if (count != *(u64 *)tray.value) {
				printf("count=%llu,v=%llu\n", count, *(u64 *)tray.value);
			}
			cr_assert_eq(count++, *(u64 *)tray.value);
		}

		cr_assert_eq(count, end);

		cr_assert(!orbtree_iterator_reset(&tree1, &itt1, &start, false, &end, true));
		count = start + 1;
		while (orbtree_iterator_next(&itt1, &tray)) {
			cr_assert_eq(count++, *(u64 *)tray.value);
		}

		cr_assert_eq(count, end + 1);
	}
}

/*
let x, y;
x = $();
$weak(&y, &x);
let z = $upgrade($y);
if(!nil(z)) {
	// use z
} else {
	// handle non-upgrade case
}
*/

/*
int my_compare(Object *a, Object *b) {
	let au64 = $(a, "u64_val");
	let bu64 = $(b, "u64_val");
	if (!nil(au64) & !nil(bu64)) {
		if ($u64(au64) > $u64(bu64))
			return 1;
		else if ($u64(au64) < $u64(bu64)) {
			return -1;
		} else
			return 0;
	} else
		return strcmp($(a, "str"), $(b, "str"));
}

int main() {
var x = $();
let y = $();
$(x, "test", y);

var a = $(with(commpare, my_compare));
var b = $();
$(b, "str", "test");
var c = $();
$(c, "str", "test2");
$(a, b);
$(a, c);

foreach(v , a) {
	info("k={}", v);
}

}
*/

// weak, iterator, and index
