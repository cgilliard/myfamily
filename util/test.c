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
	rbtree_build(&tree1, sizeof(MyKey), sizeof(MyValue), my_compare, true);
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
	loop {
		const void *next = rbtree_iterator_next(&itt);
		if (next == NULL)
			break;

		const void *next_value = next + 15 + sizeof(MyKey) + (16 - (sizeof(MyKey) % 16));

		MyKey k1;
		MyValue v1;
		memcpy(&k1, next + 15, sizeof(MyKey));
		memcpy(&v1, next_value, sizeof(MyValue));
		printf("i=%i k1.v=%llu,value=%s\n", i++, k1.v, v1.buf);
	}
}

MyTest(util, test_move_fatptr) {
	FatPtr ptr1, ptr2;
	ptr2 = null;
	chain_malloc(&ptr1, 100);
	ptr2 = ptr1;
	chain_free(&ptr2);
}
