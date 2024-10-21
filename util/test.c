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

	OrbTreeData vx;
	vx.key = 40;
	vx.value = 0;
	rep.updated = false;
	cr_assert(!orbtree_get(&tree1, &vx, &rep));
	cr_assert_eq(rep.updated, true);
	OrbTreeData x = *(OrbTreeData *)rep.value;
	cr_assert_eq(x.value, 50);

	ORBTreeTray trayreplace;
	ORBTreeTray trayremoved;
	vx.key = 40;
	vx.value = 100;
	trayreplace.value = &vx;
	trayremoved.updated = false;

	OrbTreeData remval;
	trayremoved.value = &remval;
	cr_assert(!orbtree_put(&tree1, &trayreplace, &trayremoved));
	OrbTreeData *rem = trayremoved.value;
	cr_assert(trayremoved.updated);
	cr_assert_eq(rem->value, 50);

	vx.key = 40;
	vx.value = 0;
	rep.updated = false;
	rep.value = &remval;
	cr_assert(!orbtree_get(&tree1, &vx, &rep));
	cr_assert(rep.updated);
	OrbTreeData y = *(OrbTreeData *)rep.value;
	printf("rep value = %llu\n", y.value);
	cr_assert_eq(y.value, 100);
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
		cr_assert(!orbtree_iterator_reset(&itt1, &start, true, &end, true));

		count = start;
		while (orbtree_iterator_next(&itt1, &tray)) {
			cr_assert_eq(count++, *(u64 *)tray.value);
		}
		cr_assert_eq(count, end + 1);

		cr_assert(!orbtree_iterator_reset(&itt1, &start, true, &end, false));

		count = start;
		while (orbtree_iterator_next(&itt1, &tray)) {
			cr_assert_eq(count++, *(u64 *)tray.value);
		}

		cr_assert_eq(count, end);

		cr_assert(!orbtree_iterator_reset(&itt1, &start, false, &end, false));
		count = start + 1;
		while (orbtree_iterator_next(&itt1, &tray)) {
			if (count != *(u64 *)tray.value) {
				printf("count=%llu,v=%llu\n", count, *(u64 *)tray.value);
			}
			cr_assert_eq(count++, *(u64 *)tray.value);
		}

		cr_assert_eq(count, end);

		cr_assert(!orbtree_iterator_reset(&itt1, &start, false, &end, true));
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

MyTest(util, test_object) {
	var x = object_create(false, ObjectTypeString, "my string");
	cr_assert(!nil(x));
	cr_assert(!strcmp(object_as_string(&x), "my string"));
	u64 v = 1234;
	var y = object_create(false, ObjectTypeU64, &v);
	cr_assert(!nil(y));
	cr_assert_eq(object_as_u64(&y), 1234);

	let z = object_set_property(&x, "yval", &y);
	cr_assert(!nil(z));
	var y_out = object_get_property(&x, "yval");
	let m = object_create(false, ObjectTypeString, "other str");
	let r = object_set_property(&y_out, "v2", &m);
	cr_assert(!nil(y_out));
	cr_assert_eq(object_as_u64(&y_out), 1234);
	let not_found = object_get_property(&x, "notfound");
	cr_assert(nil(not_found));

	var a = object_create(false, ObjectTypeObject, NULL);
	var b = object_create(false, ObjectTypeString, "bval");
	var res = object_set_property(&a, "b", &b);
	cr_assert(!nil(res));
	let b_out = object_get_property(&a, "b");
	cr_assert(!nil(b_out));
	cr_assert(!strcmp(object_as_string(&b_out), "bval"));
}

MyTest(util, test_object_mix_send) {
	var x = object_create(true, ObjectTypeString, "my string");
	cr_assert(!nil(x));
	cr_assert(!strcmp(object_as_string(&x), "my string"));
	u64 v = 1234;
	var y = object_create(false, ObjectTypeU64, &v);
	cr_assert(!nil(y));
	cr_assert_eq(object_as_u64(&y), 1234);

	let z = object_set_property(&x, "yval", &y);
	cr_assert(!nil(z));
	var y_out = object_get_property(&x, "yval");
	let m = object_create(false, ObjectTypeString, "other str");
	let r = object_set_property(&y_out, "v2", &m);
	cr_assert(!nil(y_out));
	cr_assert_eq(object_as_u64(&y_out), 1234);
	let not_found = object_get_property(&x, "notfound");
	cr_assert(nil(not_found));

	var a = object_create(false, ObjectTypeObject, NULL);
	var b = object_create(true, ObjectTypeString, "bval");
	var res = object_set_property(&a, "b", &b);
	cr_assert(!nil(res));
	let b_out = object_get_property(&a, "b");
	cr_assert(!nil(b_out));
	cr_assert(!strcmp(object_as_string(&b_out), "bval"));
}

MyTest(util, test_object_ref) {
	var n2;
	{
		let n1 = object_create(false, ObjectTypeString, "n1");
		n2 = object_ref(&n1);
		cr_assert(!nil(n1));
		cr_assert(!nil(n2));
		cr_assert(!strcmp(object_as_string(&n1), "n1"));
		cr_assert(!strcmp(object_as_string(&n2), "n1"));
	}
	cr_assert(!strcmp(object_as_string(&n2), "n1"));
}

MyTest(util, test_object_ref_send) {
	var n2;
	{
		let n1 = object_create(true, ObjectTypeString, "n1");
		n2 = object_ref(&n1);
		cr_assert(!nil(n1));
		cr_assert(!nil(n2));
		cr_assert(!strcmp(object_as_string(&n1), "n1"));
		cr_assert(!strcmp(object_as_string(&n2), "n1"));
	}
	cr_assert(!strcmp(object_as_string(&n2), "n1"));
}

MyTest(util, test_object_nested) {
	{
		// create object x1 in the outer most scope
		var x1 = object_create(false, ObjectTypeObject, NULL);
		{
			// create three objects in the inner scope
			var x2 = object_create(false, ObjectTypeObject, NULL);
			var x3 = object_create(false, ObjectTypeObject, NULL);
			// this is a string so we can ensure that the data is correct
			var x4 = object_create(false, ObjectTypeString, "testx4");

			// set x3's "x4" property to the x4 object
			let ret3 = object_set_property(&x3, "x4", &x4);
			cr_assert(!nil(ret3));
			// set x2's "x3" property to the x3 object
			let ret2 = object_set_property(&x2, "x3", &x3);
			cr_assert(!nil(ret2));
			// set x1's "x2" property to the x2 object
			let ret1 = object_set_property(&x1, "x2", &x2);
			cr_assert(!nil(ret1));

			// assert that we can get the inner most property in the chain
			let y2 = object_get_property(&x1, "x2");
			cr_assert(!nil(y2));
			let y3 = object_get_property(&y2, "x3");
			cr_assert(!nil(y3));
			let y4 = object_get_property(&y3, "x4");
			cr_assert(!nil(y4));
			// make sure the returned value matches expectations
			cr_assert(!strcmp(object_as_string(&y4), "testx4"));

			// at this point x2, x3, and x4 go out of scope, but they
			// have all been consumed and so they do not deallocate memory
			// the ownership of this data has been transfered to x1.
			// in addition, y2, y3, and y4 also go out of scope here,
			// but they are references only to the data which is still owned by x1.
			// the reference counter for each of these values as stored by x1, is decremented
			// so it goes fromm 2 -> 1, so the memory is not deallocated.

			// we assert that there are still 4 items in our thread local rbtree (x1-x4)
			// before the scope ends
			cr_assert_eq(get_thread_local_orbtree_size(), 4);
		}

		// we confirm that the properties are still accessible.
		let y2 = object_get_property(&x1, "x2");
		cr_assert(!nil(y2));
		let y3 = object_get_property(&y2, "x3");
		cr_assert(!nil(y3));
		let y4 = object_get_property(&y3, "x4");
		cr_assert(!nil(y4));
		cr_assert(!strcmp(object_as_string(&y4), "testx4"));

		// we assert that there are still 4 items in our thread local rbtree (x1-x4)
		// before the scope ends
		cr_assert_eq(get_thread_local_orbtree_size(), 4);
	}

	// now that x1 has gone out of scope the final cleanup is called and that recursively
	// calls cleanup on our other owned objects. All of them are deleted from the thread local
	// rbtree and internally their memory is deallocated.
	cr_assert_eq(get_thread_local_orbtree_size(), 0);
}

MyTest(util, test_object_nested_send) {
	{
		// create object x1 in the outer most scope
		var x1 = object_create(true, ObjectTypeObject, NULL);
		{
			// create three objects in the inner scope
			var x2 = object_create(true, ObjectTypeObject, NULL);
			var x3 = object_create(true, ObjectTypeObject, NULL);
			// this is a string so we can ensure that the data is correct
			var x4 = object_create(true, ObjectTypeString, "testx4");

			// set x3's "x4" property to the x4 object
			let ret3 = object_set_property(&x3, "x4", &x4);
			cr_assert(!nil(ret3));
			// set x2's "x3" property to the x3 object
			let ret2 = object_set_property(&x2, "x3", &x3);
			cr_assert(!nil(ret2));
			// set x1's "x2" property to the x2 object
			let ret1 = object_set_property(&x1, "x2", &x2);
			cr_assert(!nil(ret1));

			// assert that we can get the inner most property in the chain
			let y2 = object_get_property(&x1, "x2");
			cr_assert(!nil(y2));
			let y3 = object_get_property(&y2, "x3");
			cr_assert(!nil(y3));
			let y4 = object_get_property(&y3, "x4");
			cr_assert(!nil(y4));
			// make sure the returned value matches expectations
			cr_assert(!strcmp(object_as_string(&y4), "testx4"));

			// at this point x2, x3, and x4 go out of scope, but they
			// have all been consumed and so they do not deallocate memory
			// the ownership of this data has been transfered to x1.
			// in addition, y2, y3, and y4 also go out of scope here,
			// but they are references only to the data which is still owned by x1.
			// the reference counter for each of these values as stored by x1, is decremented
			// so it goes fromm 2 -> 1, so the memory is not deallocated.

			// we assert that there are still 4 items in our thread local rbtree (x1-x4)
			// before the scope ends
			cr_assert_eq(get_global_orbtree_size(), 4);
		}

		// we confirm that the properties are still accessible.
		let y2 = object_get_property(&x1, "x2");
		cr_assert(!nil(y2));
		let y3 = object_get_property(&y2, "x3");
		cr_assert(!nil(y3));
		let y4 = object_get_property(&y3, "x4");
		cr_assert(!nil(y4));
		cr_assert(!strcmp(object_as_string(&y4), "testx4"));

		// we assert that there are still 4 items in our thread local rbtree (x1-x4)
		// before the scope ends
		cr_assert_eq(get_global_orbtree_size(), 4);
	}

	// now that x1 has gone out of scope the final cleanup is called and that recursively
	// calls cleanup on our other owned objects. All of them are deleted from the thread local
	// rbtree and internally their memory is deallocated.
	cr_assert_eq(get_global_orbtree_size(), 0);
}

MyTest(util, test_object_remove) {
	var x1 = object_create(false, ObjectTypeObject, NULL);
	let x2 = object_create(false, ObjectTypeString, "testx2");
	let ret1 = object_get_property(&x1, "x2");
	cr_assert(nil(ret1));
	let ret2 = object_set_property(&x1, "x2", &x2);
	cr_assert(!nil(ret2));
	let ret3 = object_get_property(&x1, "x2");
	cr_assert(!nil(ret3));
	cr_assert(!strcmp(object_as_string(&ret3), "testx2"));

	let ret4 = object_remove_property(&x1, "x2");
	cr_assert(!nil(ret4));
	cr_assert(!strcmp(object_as_string(&ret4), "testx2"));

	let ret5 = object_get_property(&x1, "x2");
	cr_assert(nil(ret5));

	let ret6 = object_remove_property(&x1, "blah");
	cr_assert(nil(ret6));
}

MyTest(util, test_object_remove_send) {
	var x1 = object_create(true, ObjectTypeObject, NULL);
	let x2 = object_create(true, ObjectTypeString, "testx2");
	let ret1 = object_get_property(&x1, "x2");
	cr_assert(nil(ret1));
	let ret2 = object_set_property(&x1, "x2", &x2);
	cr_assert(!nil(ret2));
	let ret3 = object_get_property(&x1, "x2");
	cr_assert(!nil(ret3));
	cr_assert(!strcmp(object_as_string(&ret3), "testx2"));

	let ret4 = object_remove_property(&x1, "x2");
	cr_assert(!nil(ret4));
	cr_assert(!strcmp(object_as_string(&ret4), "testx2"));

	let ret5 = object_get_property(&x1, "x2");
	cr_assert(nil(ret5));

	let ret6 = object_remove_property(&x1, "blah");
	cr_assert(nil(ret6));
}

MyTest(util, test_object_overwrite) {
	var x1 = object_create(false, ObjectTypeObject, NULL);
	let x2 = object_create(false, ObjectTypeString, "testx2");
	let x3 = object_create(false, ObjectTypeString, "testx3");

	let res1 = object_set_property(&x1, "v", &x2);
	cr_assert(!nil(res1));

	let res2 = object_get_property(&x1, "v");
	cr_assert(!nil(res2));
	cr_assert(!strcmp(object_as_string(&res2), "testx2"));
	let res3 = object_set_property(&x1, "v", &x3);
	cr_assert(!nil(res3));

	let res4 = object_get_property(&x1, "v");
	cr_assert(!nil(res4));
	cr_assert(!strcmp(object_as_string(&res4), "testx3"));
}

MyTest(util, test_object_overwrite_send) {
	var x1 = object_create(true, ObjectTypeObject, NULL);
	let x2 = object_create(true, ObjectTypeString, "testx2");
	let x3 = object_create(true, ObjectTypeString, "testx3");

	let res1 = object_set_property(&x1, "v", &x2);
	cr_assert(!nil(res1));

	let res2 = object_get_property(&x1, "v");
	cr_assert(!nil(res2));
	cr_assert(!strcmp(object_as_string(&res2), "testx2"));
	let res3 = object_set_property(&x1, "v", &x3);
	cr_assert(!nil(res3));

	let res4 = object_get_property(&x1, "v");
	cr_assert(!nil(res4));
	cr_assert(!strcmp(object_as_string(&res4), "testx3"));
}

MyTest(util, test_object_get_index) {
	// create x1 to hold properties
	var x1 = object_create(false, ObjectTypeObject, NULL);

	// create 4 properties with values 0,1,2,3.
	u64 val = 0;
	let x2 = object_create(false, ObjectTypeU64, &val);
	val++;
	let x3 = object_create(false, ObjectTypeU64, &val);
	val++;
	let x4 = object_create(false, ObjectTypeU64, &val);
	val++;
	let x5 = object_create(false, ObjectTypeU64, &val);

	// set x1's properties to the corresponding values
	let res1 = object_set_property(&x1, "x2", &x2);
	cr_assert(!nil(res1));

	let res2 = object_set_property(&x1, "x3", &x3);
	cr_assert(!nil(res2));

	let res3 = object_set_property(&x1, "x4", &x4);
	cr_assert(!nil(res2));

	let res4 = object_set_property(&x1, "x5", &x5);
	cr_assert(!nil(res2));

	// iterate through the object and get the property at each index. Assert its value.
	i64 properties = object_properties(&x1);
	cr_assert_eq(properties, 4);
	for (u64 i = 0; i < properties; i++) {
		let v = object_get_property_index(&x1, i);
		cr_assert(!nil(v));
		cr_assert_eq(object_as_u64(&v), i);
	}

	/*
		// the 4th index is nil
		let v = object_get_property_index(&x1, 4);
		printf("v\n");
		cr_assert(nil(v));
		printf("y\n");

			let rem = object_remove_property_index(&x1, 1);
			cr_assert(!nil(rem));
			printf("z\n");

			properties = object_properties(&x1);
			cr_assert_eq(properties, 3);
			u64 j = 0;
			printf("here\n");
			for (u64 i = 0; i < properties; i++) {
				let v = object_get_property_index(&x1, i);
				cr_assert(!nil(v));
				printf("expect %llu %llu\n", object_as_u64(&v), j);
				// cr_assert_eq(object_as_u64(&v), j);
				if (i == 1)
					j++;
				j++;
			}
		*/
}
