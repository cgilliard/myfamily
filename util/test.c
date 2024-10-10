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

#include <base/macro_utils.h>
#include <base/test.h>
#include <crypto/psrng.h>
#include <math.h>
#include <string.h>
#include <util/bitflags.h>
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
	RBTree test1 = RBTREE_INITIALIZE;
	cr_assert(!RBTreeIsInit(test1));
	u64 k, v;
	rbtree_build(&test1, sizeof(u64), sizeof(u64), &u64_compare, false);

	cr_assert(RBTreeIsInit(test1));

	k = 0;
	v = 10;
	rbtree_insert(&test1, &k, &v);

	k = 1;
	v = 11;
	rbtree_insert(&test1, &k, &v);

	k = 2;
	v = 12;
	rbtree_insert(&test1, &k, &v);

	k = 3;
	v = 13;
	rbtree_insert(&test1, &k, &v);

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
	cr_assert(!rbtree_build(&valid1, sizeof(u64), sizeof(u64), u64_compare, false));

	u64 max = 10;
	for (u64 i = 0; i < max; i++) {
		k = i;
		v = i + 10;
		cr_assert(!rbtree_insert(&valid1, &k, &v));
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
		cr_assert(!rbtree_delete(&valid1, &k));
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
	cr_assert(!rbtree_build(&rand1, sizeof(u64), sizeof(u64), u64_compare, false));
	u64 size = 4096;
	u64 arr[size];
	for (u64 i = 0; i < size; i++) {
		arr[i] = 0; // ensure zeroed before calling rng for reproducibility.
		psrng_rand_u64(&arr[i]);
		arr[i] %= INT64_MAX;
		u64 k = arr[i];
		u64 v = k + 100;
		cr_assert(!rbtree_insert(&rand1, &k, &v));
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
		cr_assert(!rbtree_delete(&rand1, &arr[i]));
		cr_assert_eq(rbtree_size(&rand1), (size - 1) - i);
		rbtree_validate(&rand1);
		i++;
	}
	cr_assert_eq(rbtree_size(&rand1), 0);
}

MyTest(util, test_validation_and_other) {
	RBTree test1;
	cr_assert(rbtree_build(NULL, 10, 10, u64_compare, false));
	__is_debug_malloc = true;
	cr_assert(rbtree_build(&test1, sizeof(u64), sizeof(u64), u64_compare, false));
	__is_debug_malloc = false;
	test1.impl = null;

	cr_assert(!rbtree_build(&test1, sizeof(u64), sizeof(u64), u64_compare, false));
	__is_debug_malloc = true;
	u64 k1 = 0;
	u64 v1 = 0;
	cr_assert(rbtree_insert(&test1, &k1, &v1));
	__is_debug_malloc = false;
	cr_assert(rbtree_insert(NULL, &k1, &v1));
	cr_assert(rbtree_size(NULL));
	cr_assert(!rbtree_insert(&test1, &k1, &v1));
	k1 = 1;
	v1 = 0;
	cr_assert(!rbtree_insert(&test1, &k1, &v1));
	cr_assert(rbtree_insert(&test1, &k1, &v1));
	k1 = 10;
	v1 = 0;
	cr_assert(rbtree_delete(&test1, &k1));
	cr_assert(!rbtree_get(NULL, NULL));
	cr_assert_eq(rbtree_max_depth(&test1), 2);
	rbtree_print(&test1);
	cr_assert(rbtree_iterator(NULL, NULL, NULL, false, NULL, false));
	rbtree_delete(NULL, NULL);
}

MyTest(util, test_rbtree_random_ordered_insert_delete) {
	// seed rng for reproducibility
	u8 key[32] = {1};
	u8 iv[16] = {};
	psrng_test_seed(iv, key);

	RBTree rand1;
	cr_assert(!rbtree_build(&rand1, sizeof(u64), sizeof(u64), u64_compare, false));
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
			cr_assert(!rbtree_insert(&rand1, &arr[arr_index], &v));
			arr_index++;
		} else {
			cr_assert(!rbtree_delete(&rand1, &arr[delete_index]));

			delete_index++;
		}
		u64 cur_size = rbtree_size(&rand1);
		u64 max_depth = rbtree_max_depth(&rand1);
		u64 max_valid_depth = 1;
		if (cur_size)
			max_valid_depth = (log2(cur_size) * 2) + 1;
		// max theoretical size of a valid rbtree is 2 log(n) + 1. Ensure we're no worse than that.
		cr_assert(max_valid_depth >= max_depth);
		cr_assert_eq(cur_size, arr_index - delete_index);
		rbtree_validate(&rand1);
	}
}

MyTest(util, test_rbtree_range_itt) {
	RBTree tree1;
	cr_assert(!rbtree_build(&tree1, sizeof(u64), sizeof(u64), u64_compare, false));
	u64 size = 10;
	u64 arr[size];
	for (u64 i = 0; i < size; i++) {
		arr[i] = (i * 3) + 1;
		u64 k = arr[i];
		u64 v = k + 100;
		cr_assert(!rbtree_insert(&tree1, &k, &v));
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
	cr_assert(!rbtree_build(&tree1, sizeof(MyRecord), sizeof(u64), my_record_compare, false));
	u64 size = 10;
	for (u64 i = 0; i < size; i++) {
		char s[101];
		snprintf(s, 100, "test%llu", i);
		MyRecord k;
		strcpy(k.name, s);
		k.value = i;
		u64 v = k.value + 100;
		cr_assert(!rbtree_insert(&tree1, &k, &v));
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

MyTest(util, test_bitflags) {
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
