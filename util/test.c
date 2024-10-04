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

#include <base/test.h>
#include <string.h>
#include <util/rbtree.h>

MySuite(util);

typedef struct MyKey {
	char buf[100];
} MyKey;

typedef struct MyValue {
	char buf[200];
} MyValue;

int my_compare(const void *v1, const void *v2) {
	const MyKey *k1 = v1;
	const MyKey *k2 = v2;
	return strcmp(k1->buf, k2->buf);
}

MyTest(util, test_rbtree) {
	RBTree tree1;
	rbtree_build(&tree1, sizeof(MyKey), sizeof(MyValue), my_compare, true);
	MyKey k1;
	strcpy(k1.buf, "test1");
	MyValue v1;
	strcpy(v1.buf, "value1");
	rbtree_insert(&tree1, &k1, &v1);

	MyKey k2;
	strcpy(k2.buf, "test3");
	MyValue v2;
	strcpy(v2.buf, "value3");
	rbtree_insert(&tree1, &k2, &v2);

	MyKey k3;
	strcpy(k3.buf, "test2");
	MyValue v3;
	strcpy(v3.buf, "value2");
	rbtree_insert(&tree1, &k3, &v3);
	// rbtree_insert(&tree1, &k3, &v3);

	const MyValue *v1_out = rbtree_get(&tree1, &k1);
	printf("v1_out='%s' %p\n", v1_out->buf, v1_out);

	const MyValue *v2_out = rbtree_get(&tree1, &k2);
	printf("v2_out='%s' %p\n", v2_out->buf, v2_out);

	const MyValue *v3_out = rbtree_get(&tree1, &k3);
	printf("v3_out='%s' %p\n", v3_out->buf, v3_out);

	MyKey k4;
	strcpy(k4.buf, "other");
	const MyValue *v4_out = rbtree_get(&tree1, &k4);
	cr_assert_eq(v4_out, NULL);
}

MyTest(util, test_move_fatptr) {
	FatPtr ptr1, ptr2;
	ptr2 = null;
	chain_malloc(&ptr1, 100);
	ptr2 = ptr1;
	chain_free(&ptr2);
}
