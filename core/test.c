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
#include <core/lib.h>
#include <crypto/lib.h>

Suite(core);

typedef struct MyObject {
	int64 version;
	int64 value;
	OrbTreeNode node;
	Ptr ptr;
} MyObject;

unsigned int offsetof_node = offsetof(MyObject, node);

int my_obj_search(OrbTreeNode *cur, const OrbTreeNode *value,
				  OrbTreeNodePair *retval) {
	while (cur) {
		int64 v1 = ((MyObject *)((byte *)cur - offsetof_node))->value;
		int64 v2 = ((MyObject *)((byte *)value - offsetof_node))->value;

		if (v1 == v2) {
			retval->self = cur;
			break;
		} else if (v1 < v2) {
			retval->parent = cur;
			retval->is_right = true;
			cur = cur->right;
		} else {
			retval->parent = cur;
			retval->is_right = false;
			cur = cur->left;
		}
		retval->self = cur;
	}
	return 0;
}

#define IS_BLACK(node) (((unsigned long long)node->parent_color % 2) == 0)
#define PARENT(node) \
	((OrbTreeNode *)((unsigned long long)node->parent_color & ~0x1))

void my_obj_validate_node(const OrbTreeNode *node, int *black_count,
						  int current_black_count) {
	if (node == NULL) {
		if (*black_count == 0) {
			*black_count =
				current_black_count;  // Set the black count for the first path
		} else {
			// Check for black count
			// consistency
			fam_assert(current_black_count == *black_count);
		}
		return;
	}

	if (IS_BLACK(node))
		current_black_count++;
	else {
		// check parent is not red (red property violation)
		fam_assert((unsigned long long)node->parent_color > 1);
		if (!IS_BLACK(PARENT(node))) {
			println("red violation!");
		}
		fam_assert(IS_BLACK(PARENT(node)));
	}
	my_obj_validate_node(node->right, black_count, current_black_count);
	my_obj_validate_node(node->left, black_count, current_black_count);
}

void my_obj_validate(const OrbTree *tree) {
	int black_count = 0;
	OrbTreeNode *root = tree->root;
	// Validate from the root and
	// check if the root is black
	if (root != NULL) {
		// assert root is black
		fam_assert(IS_BLACK(root));
		my_obj_validate_node(root, &black_count, 0);
	}
}

Test(orbtree) {
	OrbTree tree = INIT_ORBTREE;
	MyObject obj1 = {.value = 1};
	MyObject obj2 = {.value = 2};
	MyObject obj3 = {.value = 3};

	orbtree_put(&tree, &obj2.node, my_obj_search);
	orbtree_put(&tree, &obj1.node, my_obj_search);
	orbtree_put(&tree, &obj3.node, my_obj_search);

	OrbTreeNodePair empty = {};
	OrbTreeNodePair pair = empty;

	my_obj_search(tree.root, &obj1.node, &pair);
	fam_assert_eq(pair.self, &obj1.node);
	fam_assert_eq(pair.parent, &obj2.node);

	pair = empty;
	my_obj_search(tree.root, &obj2.node, &pair);
	fam_assert_eq(pair.self, &obj2.node);
	fam_assert_eq(pair.parent, NULL);

	pair = empty;
	my_obj_search(tree.root, &obj3.node, &pair);
	fam_assert_eq(pair.self, &obj3.node);
	fam_assert_eq(pair.parent, &obj2.node);
}

Test(orbtree_rand) {
	// seed rng for reproducibility
	byte key[32] = {9};
	byte iv[16] = {};
	cpsrng_test_seed(iv, key);

	OrbTree tree = INIT_ORBTREE;
	int size = 1000;
	SlabAllocator sa;
	slab_allocator_init(&sa, sizeof(MyObject), size + 10, size + 10);
	Ptr *arr = mmap_allocate(sizeof(Ptr) * size);
	int64 *values = mmap_allocate(sizeof(int64) * size);

	for (int i = 0; i < size; i++) {
		arr[i] = slab_allocator_allocate(&sa);
		MyObject *obj = (MyObject *)slab_get(&sa, arr[i]);
		int64 v = 0;
		cpsrng_rand_int64(&v);
		values[i] = v;
		obj->value = v;
		obj->version = 0;
		obj->ptr = arr[i];
		fam_assert(!orbtree_put(&tree, &obj->node, my_obj_search));
		my_obj_validate(&tree);
	}

	Ptr tmp;
	MyObject *obj, *obj_out;
	OrbTreeNode *prev;

	tmp = slab_allocator_allocate(&sa);
	obj = (MyObject *)slab_get(&sa, tmp);
	obj->value = values[3];
	obj->version = 1;
	obj->ptr = tmp;
	prev = orbtree_put(&tree, &obj->node, my_obj_search);
	fam_assert(prev);
	obj_out = (MyObject *)((byte *)prev - offsetof_node);
	fam_assert_eq(obj_out->value, values[3]);
	my_obj_validate(&tree);
	slab_allocator_free(&sa, obj_out->ptr);

	tmp = slab_allocator_allocate(&sa);
	obj = (MyObject *)slab_get(&sa, tmp);
	obj->value = values[7];
	obj->version = 1;
	obj->ptr = tmp;
	prev = orbtree_put(&tree, &obj->node, my_obj_search);
	fam_assert(prev);
	obj_out = (MyObject *)((byte *)prev - offsetof_node);
	fam_assert_eq(obj_out->value, values[7]);
	my_obj_validate(&tree);
	slab_allocator_free(&sa, obj_out->ptr);

	for (int i = 0; i < size; i++) {
		tmp = slab_allocator_allocate(&sa);
		obj = (MyObject *)slab_get(&sa, tmp);
		obj->value = values[i];
		prev = orbtree_remove(&tree, &obj->node, my_obj_search);
		fam_assert(prev);
		obj_out = (MyObject *)((byte *)prev - offsetof_node);
		fam_assert_eq(obj_out->value, values[i]);
		fam_assert(!orbtree_remove(&tree, &obj->node, my_obj_search));
		slab_allocator_free(&sa, tmp);
		slab_allocator_free(&sa, obj_out->ptr);
		my_obj_validate(&tree);
	}

	// assert that all slabs are freed
	fam_assert_eq(slab_allocator_free_size(&sa),
				  slab_allocator_total_slabs(&sa));
	slab_allocator_cleanup(&sa);

	mmap_free(arr, sizeof(Ptr) * size);
	mmap_free(values, sizeof(int64) * size);
}

typedef struct MyLong {
	Ptr ptr;
	int64 x;
	int64 y;
	OrbTreeNode node;
} MyLong;

static int long_offt = offsetof(MyLong, node);

int my_long_search(OrbTreeNode *cur, const OrbTreeNode *value,
				   OrbTreeNodePair *retval) {
	while (cur) {
		int64 v1 = ((MyLong *)((byte *)cur - long_offt))->x;
		int64 v2 = ((MyLong *)((byte *)value - long_offt))->x;

		if (v1 == v2) {
			retval->self = cur;
			break;
		} else if (v1 < v2) {
			retval->parent = cur;
			retval->is_right = true;
			cur = cur->right;
		} else {
			retval->parent = cur;
			retval->is_right = false;
			cur = cur->left;
		}
		retval->self = cur;
	}
	return 0;
}

Test(test_orbtree_perf) {
	// seed rng for reproducibility
	char key[32] = {7};

	char iv[16] = {};

	cpsrng_test_seed(iv, key);

	// int size = 10 * 1000;
	int size = 100;
	int count = 1000;

	int64 *keys = mmap_allocate(size * sizeof(int64));
	OrbTree t = INIT_ORBTREE;
	SlabAllocator sa;
	slab_allocator_init(&sa, sizeof(MyLong), size + 10, size + 10);

	for (int64 i = 0; i < size; i++) {
		keys[i] = 0;
		cpsrng_rand_int64(&keys[i]);
	}

	for (int x = 0; x < count; x++) {
		for (int64 i = 0; i < size; i++) {
			Ptr ptr = slab_allocator_allocate(&sa);
			MyLong *obj = (MyLong *)slab_get(&sa, ptr);
			obj->ptr = ptr;
			obj->x = keys[i];
			obj->y = i;
			fam_assert(!orbtree_put(&t, &obj->node, my_long_search));
		}

		for (int64 i = 0; i < size; i++) {
			MyLong obj = {.x = keys[i]};
			OrbTreeNode *node = orbtree_remove(&t, &obj.node, my_long_search);
			MyLong *ref = (MyLong *)((byte *)node - long_offt);
			slab_allocator_free(&sa, ref->ptr);
		}
	}

	// assert that all slabs have been freed
	int fs = slab_allocator_free_size(&sa);
	int ts = slab_allocator_total_slabs(&sa);
	fam_assert_eq(fs, ts);

	slab_allocator_cleanup(&sa);
	mmap_free(keys, size * sizeof(int64));
}
