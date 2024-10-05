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
#include <base/rand.h>
#include <base/test.h>
#include <string.h>
#include <util/rbtree.h>

MySuite(util);

typedef struct MyKey {
	u64 v;
} MyKey;

typedef struct MyValue {
	char buf[200];
} MyValue;

int my_compare(const void *v1, const void *v2) {
	const MyKey *k1 = v1;
	const MyKey *k2 = v2;
	if (k1->v < k2->v)
		return -1;
	else if (k1->v > k2->v)
		return 1;
	return 0;
}

MyTest(util, test_rbtree) {
	RBTree tree1;
	cr_assert(!rbtree_build(&tree1, sizeof(MyKey), sizeof(MyValue), my_compare, true));
	MyKey k1 = {20};
	MyValue v1;
	strcpy(v1.buf, "value20");
	rbtree_insert(&tree1, &k1, &v1);

	MyKey k2 = {10};
	MyValue v2;
	strcpy(v2.buf, "value10");
	rbtree_insert(&tree1, &k2, &v2);

	MyKey k3 = {30};
	MyValue v3;
	strcpy(v3.buf, "value30");
	rbtree_insert(&tree1, &k3, &v3);

	MyKey k4 = {40};
	MyValue v4;
	strcpy(v4.buf, "value40");
	rbtree_insert(&tree1, &k4, &v4);

	MyKey k5 = {35};
	MyValue v5;
	strcpy(v5.buf, "value35");
	rbtree_insert(&tree1, &k5, &v5);

	const MyValue *v1_out = rbtree_get(&tree1, &k1);
	cr_assert(!strcmp(v1_out->buf, "value20"));

	const MyValue *v2_out = rbtree_get(&tree1, &k2);
	cr_assert(!strcmp(v2_out->buf, "value10"));

	const MyValue *v3_out = rbtree_get(&tree1, &k3);
	cr_assert(!strcmp(v3_out->buf, "value30"));

	MyKey k41 = {4};
	const MyValue *v41_out = rbtree_get(&tree1, &k41);
	cr_assert_eq(v41_out, NULL);

	cr_assert_eq(rbtree_size(&tree1), 5);
	printf("del1\n");
	cr_assert(!rbtree_delete(&tree1, &k3));

	const MyValue *v1_out2 = rbtree_get(&tree1, &k1);
	cr_assert(!strcmp(v1_out2->buf, "value20"));

	const MyValue *v2_out2 = rbtree_get(&tree1, &k2);
	cr_assert(!strcmp(v2_out2->buf, "value10"));

	const MyValue *v3_out2 = rbtree_get(&tree1, &k3);
	cr_assert_eq(v3_out2, NULL);
	cr_assert_eq(rbtree_size(&tree1), 4);

	RBTreeIterator itt;
	cr_assert(!rbtree_iterator(&tree1, &itt));

	int i = 0;
	u64 last = 0;
	loop {
		RbTreeKeyValue kv;
		bool has_next = rbtree_iterator_next(&itt, &kv);
		if (!has_next)
			break;

		MyKey *k1 = kv.key;
		MyValue *v1 = kv.value;
		printf("i=%i k1.v=%llu,value=%s\n", i++, k1->v, v1->buf);
		cr_assert(last < k1->v);
		last = k1->v;
		char buf[101];
		snprintf(buf, 100, "value%llu", k1->v);
		cr_assert(!strcmp(buf, v1->buf));
	}
	cr_assert_eq(i, 4);

	RBTree tree2;
	cr_assert(!rbtree_build(&tree2, sizeof(MyKey), sizeof(MyValue), my_compare, false));
	const MyValue *v42_out = rbtree_get(&tree2, &k4);
	cr_assert_eq(v42_out, NULL);

	cr_assert_eq(rbtree_size(&tree2), 0);
	cr_assert(!rbtree_insert(&tree2, &k1, &v1));
	cr_assert(!strcmp(rbtree_get(&tree2, &k1), "value20"));
	cr_assert_eq(rbtree_size(&tree2), 1);
	printf("del2\n");
	rbtree_delete(&tree2, &k1);

	cr_assert_eq(rbtree_size(&tree2), 0);
	RBTreeIterator itt2;
	RbTreeKeyValue kv2;
	cr_assert(!rbtree_iterator(&tree2, &itt2));
	cr_assert(!rbtree_iterator_next(&itt2, &kv2));

	cr_assert(!rbtree_insert(&tree2, &k1, &v2));

	cr_assert_eq(rbtree_size(&tree2), 1);

	RBTreeIterator itt3;
	RbTreeKeyValue kv3;
	cr_assert(!rbtree_iterator(&tree2, &itt3));
	cr_assert(rbtree_iterator_next(&itt3, &kv3));
	MyKey *kv1k = kv3.key;
	MyValue *kv1v = kv3.value;
	cr_assert_eq(kv1k->v, 20);
	cr_assert(!strcmp(kv1v->buf, "value10"));

	cr_assert(!strcmp(rbtree_get(&tree2, &k1), "value10"));
	cr_assert_eq(rbtree_get(&tree2, &k2), NULL);
}

int u64_compare(const void *v1, const void *v2) {
	const u64 *k1 = v1;
	const u64 *k2 = v2;
	if (*k1 < *k2)
		return -1;
	else if (*k1 > *k2)
		return 1;
	return 0;
}

MyTest(util, test_random_rbtree) {
	RBTree rand1;
	cr_assert(!rbtree_build(&rand1, sizeof(u64), sizeof(u64), u64_compare, false));
	u64 size = 1000;
	u64 arr[size];
	for (u64 i = 0; i < size; i++) {
		cr_assert(!rand_u64(&arr[i]));
		arr[i] %= INT64_MAX;
		u64 k = arr[i];
		u64 v = k + 100;
		cr_assert(!rbtree_insert(&rand1, &k, &v));
		cr_assert(rbtree_validate(&rand1));
	}

	for (u64 i = 0; i < size; i++) {
		const u64 *value = rbtree_get(&rand1, &arr[i]);
		cr_assert_eq(*value, arr[i] + 100);
	}

	RBTreeIterator itt1;
	cr_assert(!rbtree_iterator(&rand1, &itt1));

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

	i = 0;
	loop {
		if (i == size)
			break;
		cr_assert(!rbtree_delete(&rand1, &arr[i]));
		cr_assert_eq(rbtree_size(&rand1), (size - 1) - i);
		// printf("validate tree i = %llu\n", i);
		i++;
	}
	/*
			cr_assert_eq(rbtree_size(&rand1), 0);
		*/
}

MyTest(util, test_move_fatptr) {
	FatPtr ptr1, ptr2;
	ptr2 = null;
	chain_malloc(&ptr1, 100);
	ptr2 = ptr1;
	chain_free(&ptr2);
}

MyTest(util, validate_tree) {
	RBTree valid1;
	u64 k, v;
	cr_assert(!rbtree_build(&valid1, sizeof(u64), sizeof(u64), u64_compare, false));

	u64 max = 1000;
	for (u64 i = 0; i < max; i++) {
		k = i;
		v = i + 10;
		cr_assert(!rbtree_insert(&valid1, &k, &v));
		cr_assert(rbtree_validate(&valid1));
	}
	for (u64 i = 0; i < max; i++) {
		k = i;
		v = i + 10;
		cr_assert(!rbtree_delete(&valid1, &k));
		cr_assert_eq(rbtree_size(&valid1), (max - 1) - i);
		cr_assert(rbtree_validate(&valid1));
	}
}
