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
#include <base/test.h>
#include <crypto/psrng.h>
#include <math.h>
#include <string.h>
#include <util/object.h>
#include <util/rbtree.h>

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
		cr_assert(!rbtree_remove(&valid1, &k));
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
		cr_assert(!rbtree_remove(&rand1, &arr[i]));
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
	cr_assert(rbtree_remove(&test1, &k1));
	cr_assert(!rbtree_get(NULL, NULL));
	cr_assert_eq(rbtree_max_depth(&test1), 2);
	rbtree_print(&test1);
	cr_assert(rbtree_iterator(NULL, NULL, NULL, false, NULL, false));
	rbtree_remove(NULL, NULL);
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
			cr_assert(!rbtree_remove(&rand1, &arr[delete_index]));

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

MyTest(util, test_object) {
	{
		var test = NIL;
		var test2 = NIL;
		var test3 = NIL;
		var test4 = NIL;
		let aaaa = NIL;

		// create an empty object
		cr_assert(!object_create(&test, true, ObjectTypeObject, NULL));

		// create a string object
		cr_assert(!object_create(&test3, true, ObjectTypeString, "test123"));

		// create a u64 object
		u64 v2 = 101;
		cr_assert(!object_create(&test4, false, ObjectTypeU64, &v2));

		// move test to test2
		cr_assert(!object_move(&test2, &test));

		// compare test3 to expected value
		cr_assert(!strcmp("test123", object_as_string(&test3)));

		// set proprerty 'test' to test3 object.
		cr_assert(!object_set_property(&test2, "test", &test3));

		// confirm that the property is set by calling object_get_property and object_as_string
		let test3_out = object_get_property(&test2, "test");
		cr_assert(!strcmp(object_as_string(&test3_out), "test123"));

		// confirm the u64 object through object_as_u64
		u64 test4_out;
		cr_assert(!object_as_u64(&test4, &test4_out));
		cr_assert_eq(test4_out, 101);
		u64 x1 = 123ULL;
		$(test4, "test", x1);
		$(test4, "ax1", "ok");
		let vvv = $();
		$(test4, "ddd", vvv);
		let q = $(test4, "ddd");

		let test_nil = NIL;
		cr_assert(nil(test_nil));
		let v = $();
		cr_assert(!nil(v));
	}

	// for testing purposes we cleanup the global RBTrees to ensure all memory is freed
	object_cleanup_global();
}

MyTest(util, test_object2) {
	{
		// declare a mutable empty object
		var a = $();
		// set the 'abc' property of object a to a string value 'test'
		$(a, "abc", "test");
		// set the 'def' property of object a to a string value 'test2'
		$(a, "def", "test2");
		// get property 'abc' of the object a and store in an immutable object b
		let b = $(a, "abc");

		// assert that object b as a string is 'test'
		cr_assert(!strcmp("test", $string(b)));

		// get property 'def' of object a and store in an immutable object c
		let c = $(a, "def");

		// assert that object c as a string is 'test2'
		cr_assert(!strcmp("test2", $string(c)));

		// create a u64 value of 99
		u64 v = 99;
		// set the property 'ghi' of object a to the value of v (99)
		$(a, "ghi", v);
		// get property 'ghi' of object a and store in an immutable object d
		let d = $(a, "ghi");
		// assert that object d as a u64 is 99
		cr_assert_eq($u64(d), 99);
	}
	// for testing purposes we cleanup the global RBTrees to ensure all memory is freed
	object_cleanup_global();
}

MyTest(util, test_print_err) {
	{
		// instnatiate an object
		let x = $();
		// check for errors if x is 'nil' an error occured.
		if (nil(x)) {
			// print_err prints the associated 'fam_err' and a message.
			print_err("object creation error");
			exit(-1);
		}

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
	// for testing purposes we cleanup the global RBTrees to ensure all memory is freed
	object_cleanup_global();
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
