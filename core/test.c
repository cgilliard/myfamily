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
#include <stddef.h>

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

/*
Test(orbtree) {
	OrbTree tree = INIT_ORBTREE;
	MyObject obj1 = {.value = 1};
	MyObject obj2 = {.value = 2};
	MyObject obj3 = {.value = 3};

	println("insert 1");
	orbtree_put(&tree, &obj2.node, my_obj_search);
	println("insert 2");
	orbtree_put(&tree, &obj1.node, my_obj_search);
	println("insert 3");
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
*/

SlabAllocator sa;

Test(orbtree_rand) {
	// seed rng for reproducibility
	byte key[32] = {9};
	byte iv[16] = {};
	cpsrng_test_seed(iv, key);

	OrbTree tree = INIT_ORBTREE;
	int size = 1000;
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

/*

typedef struct MyObject {
	int x;
	int v;
	Ptr ptr;
	OrbTreeNode node1;
	OrbTreeNode node2;
} MyObject;

SlabAllocator sa;

int my_obj_search(const OrbTreeNode *root, const OrbTreeNode *value,
				  OrbTreeNodePair *retval) {
	retval->parent = null;
	retval->is_right = true;
	const OrbTreeNode *cur = root;
	unsigned int offsetof = offsetof(MyObject, node1);
	loop {
		int x1 = ((MyObject *)((byte *)cur - offsetof))->x;
		int x2 = ((MyObject *)((byte *)value - offsetof))->x;

		retval->self = orbtree_node_ptr(cur, retval->is_right);

		if (x1 == x2) {
			break;
		} else if (x1 < x2) {
			MyObject *right = orbtree_node_right(cur);
			if (right == NULL) {
				retval->parent = retval->self;
				retval->self = null;
				retval->is_right = true;
				break;
			}
			Ptr rptr = orbtree_node_ptr(&right->node1, true);
			retval->parent = orbtree_node_ptr(cur, retval->is_right);
			retval->is_right = true;
			cur = (const OrbTreeNode *)(slab_get(&sa, rptr) +
										offsetof(MyObject, node1));
		} else {
			MyObject *left = orbtree_node_left(cur);
			if (left == NULL) {
				retval->parent = retval->self;
				retval->self = null;
				retval->is_right = false;
				break;
			}
			Ptr lptr = orbtree_node_ptr(&left->node1, false);
			retval->parent = orbtree_node_ptr(cur, retval->is_right);
			retval->is_right = false;
			cur = (const OrbTreeNode *)(slab_get(&sa, lptr) +
										offsetof(MyObject, node1));
		}
	}
	return 0;
}

void my_obj_print_node1(Ptr ptr, int depth) {
	if (ptr == null) {
		for (int i = 0; i < depth; i++) {
			print("    ");
		}
		println("NIL (B)");
		return;
	}

	MyObject *obj = (MyObject *)slab_get(&sa, ptr);
	OrbTreeNode *node = &obj->node1;
	MyObject *right = orbtree_node_right(node);
	Ptr right_ptr;
	if (right)
		right_ptr = orbtree_node_ptr(&right->node1, true);
	else
		right_ptr = null;

	// print right child
	my_obj_print_node1(right_ptr, depth + 1);

	// Indent according to depth
	for (int i = 0; i < depth; i++) {
		print("    ");
	}

	println("%i (%s,ptr=%u,rh=%u,lh=%u)", obj->x,
			orbtree_node_is_red(&obj->node1) ? "R" : "B", obj->ptr,
			orbtree_node_right_subtree_height(&obj->node1),
			orbtree_node_left_subtree_height(&obj->node1));

	MyObject *left = orbtree_node_left(node);
	Ptr left_ptr;
	if (left)
		left_ptr = orbtree_node_ptr(&left->node1, false);
	else
		left_ptr = null;

	// print right child
	my_obj_print_node1(left_ptr, depth + 1);
}

void my_obj_print_root(Ptr ptr) {
	if (ptr == null) {
		println("Red-Black Tree (root = null) Empty Tree!");
	} else {
		MyObject *root = (MyObject *)slab_get(&sa, ptr);
		println(
			"Red-Black Tree "
			"(root = %i)",
			root->x);
		println(
			"==================="
			"================");
		my_obj_print_node1(ptr, 0);
		println(
			"==================="
			"================");
	}
}

void my_obj_print_tree(const OrbTree *tree) {
	Ptr root = orbtree_root(tree);
	my_obj_print_root(root);
}

void my_obj_validate_node(const OrbTree *tree, Ptr node, int *black_count,
						  int current_black_count, bool is_right) {
	if (node == null) {
		if (*black_count == 0) {
			*black_count =
				current_black_count;  // Set the black count for the first
path } else {
			// Check for black count
			// consistency
			fam_assert(current_black_count == *black_count);
		}
		return;
	}

	MyObject *obj = (MyObject *)slab_get(&sa, node);
	OrbTreeNode *n = &obj->node1;
	OrbTreeNode *right_node = NULL;
	if (orbtree_node_right(n))
		right_node = &((MyObject *)orbtree_node_right(n))->node1;
	OrbTreeNode *left_node = NULL;
	if (orbtree_node_left(n))
		left_node = &((MyObject *)orbtree_node_left(n))->node1;
	unsigned int rsum = 0;
	if (right_node)
		rsum += 1 + orbtree_node_right_subtree_height(right_node) +
				orbtree_node_left_subtree_height(right_node);
	unsigned int lsum = 0;
	if (left_node)
		lsum += 1 + orbtree_node_right_subtree_height(left_node) +
				orbtree_node_left_subtree_height(left_node);

	if (lsum != orbtree_node_left_subtree_height(n) ||
		rsum != orbtree_node_right_subtree_height(n)) {
		println("lsum=%u,lh=%u,rsum=%u,rh=%u", lsum,
				orbtree_node_left_subtree_height(n), rsum,
				orbtree_node_right_subtree_height(n));
	}
	fam_assert_eq(lsum, orbtree_node_left_subtree_height(n));
	fam_assert_eq(rsum, orbtree_node_right_subtree_height(n));

	if (!orbtree_node_is_red(&obj->node1))
		current_black_count++;
	else {
		// check parent is not red (red property violation)
		MyObject *parent_obj = orbtree_node_parent(&obj->node1);
		OrbTreeNode *parent = &parent_obj->node1;
		fam_assert(parent);
		if (orbtree_node_is_red(parent)) {
			println("red violation at obj->x=%i", obj->x);
		}
		fam_assert(!orbtree_node_is_red(parent));
		Ptr parent_ptr = orbtree_node_ptr(parent, is_right);
	}

	// Recursive calls for left and
	// right children
	MyObject *left = orbtree_node_left(&obj->node1);
	Ptr left_ptr;
	if (left == NULL)
		left_ptr = null;
	else
		left_ptr = orbtree_node_ptr(&left->node1, false);
	MyObject *right = orbtree_node_right(&obj->node1);

	Ptr right_ptr;
	if (right == NULL)
		right_ptr = null;
	else
		right_ptr = orbtree_node_ptr(&right->node1, true);

	my_obj_validate_node(tree, right_ptr, black_count, current_black_count,
						 true);
	my_obj_validate_node(tree, left_ptr, black_count, current_black_count,
						 false);
}

void my_obj_validate(const OrbTree *tree) {
	int black_count = 0;
	Ptr root = orbtree_root(tree);
	// Validate from the root and
	// check if the root is black
	if (root != null) {
		MyObject *obj = (MyObject *)slab_get(&sa, root);
		fam_assert(!orbtree_node_is_red(&obj->node1));
		my_obj_validate_node(tree, root, &black_count, 0, false);
	}
}

Test(test_orbtree) {
	unsigned int off = offsetof(MyObject, node1);
	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), 100, 200);
	orbtree_init(&t, &sa);

	Ptr ptr1 = slab_allocator_allocate(&sa);
	MyObject *obj1 = (MyObject *)slab_get(&sa, ptr1);
	obj1->x = 1;
	obj1->v = 0;
	orbtree_put(&t, ptr1, off, my_obj_search);

	Ptr ptr2 = slab_allocator_allocate(&sa);
	MyObject *obj2 = (MyObject *)slab_get(&sa, ptr2);
	obj2->x = 0;
	obj2->v = 0;

	orbtree_put(&t, ptr2, off, my_obj_search);

	MyObject *obj2_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, obj2, off, my_obj_search,
0)); fam_assert(obj2_out); fam_assert_eq(obj2_out->x, 0);

	MyObject *obj1_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, obj1, off, my_obj_search,
0)); fam_assert(obj1_out); fam_assert_eq(obj1_out->x, 1);

	Ptr ptr3 = slab_allocator_allocate(&sa);
	MyObject *obj3 = (MyObject *)slab_get(&sa, ptr3);
	obj3->x = 2;
	obj3->v = 0;
	orbtree_put(&t, ptr3, off, my_obj_search);

	MyObject *obj3_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, obj3, off, my_obj_search,
0)); fam_assert(obj3_out); fam_assert_eq(obj3_out->x, 2);

	my_obj_validate(&t);

	slab_allocator_free(&sa, ptr1);
	slab_allocator_free(&sa, ptr2);
	slab_allocator_free(&sa, ptr3);

	slab_allocator_cleanup(&sa);
}

Test(test_random_tree) {
	unsigned int off = offsetof(MyObject, node1);

	// seed rng for reproducibility
	byte key[32] = {6};
	byte iv[16] = {};
	cpsrng_test_seed(iv, key);

	int size = 1000;
	Ptr arr[size];
	int vals[size];

	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), size + 10, size + 10);

	orbtree_init(&t, &sa);

	for (int i = 0; i < size; i++) {
		int x = 0;
		cpsrng_rand_int(&x);
		vals[i] = x;
		Ptr ptr = slab_allocator_allocate(&sa);
		MyObject *obj = (MyObject *)slab_get(&sa, ptr);
		obj->x = x;
		obj->v = 0;
		obj->ptr = ptr;
		fam_assert(!orbtree_put(&t, ptr, off, my_obj_search));
		arr[i] = ptr;
		my_obj_validate(&t);
	}
	my_obj_validate(&t);

	for (int i = 0; i < size; i++) {
		MyObject obj = {.x = vals[i], 0};
		MyObject *obj_out = (MyObject *)slab_get(
			&sa, orbtree_get(&t, &obj, off, my_obj_search, 0));
		fam_assert_eq(obj_out->ptr, arr[i]);
	}

	// try a node not in the tree
	MyObject obj = {.x = -1, .v = 0};
	Ptr obj_out = orbtree_get(&t, &obj, off, my_obj_search, 0);
	fam_assert_eq(obj_out, null);

	my_obj_validate(&t);

	Ptr ptr, rep_ptr;
	MyObject *replace;

	// do transplants
	ptr = slab_allocator_allocate(&sa);
	replace = (MyObject *)slab_get(&sa, ptr);
	replace->x = vals[3];
	replace->v = 1;
	rep_ptr = orbtree_put(&t, ptr, off, my_obj_search);
	fam_assert_eq(rep_ptr, arr[3]);
	arr[3] = ptr;
	slab_allocator_free(&sa, rep_ptr);

	ptr = slab_allocator_allocate(&sa);
	replace = (MyObject *)slab_get(&sa, ptr);
	replace->x = vals[7];
	replace->v = 1;
	rep_ptr = orbtree_put(&t, ptr, off, my_obj_search);
	fam_assert_eq(rep_ptr, arr[7]);
	arr[7] = ptr;
	slab_allocator_free(&sa, rep_ptr);

	ptr = slab_allocator_allocate(&sa);
	replace = (MyObject *)slab_get(&sa, ptr);
	replace->x = vals[17];
	replace->v = 1;
	rep_ptr = orbtree_put(&t, ptr, off, my_obj_search);
	fam_assert_eq(rep_ptr, arr[17]);
	arr[17] = ptr;
	slab_allocator_free(&sa, rep_ptr);

	my_obj_validate(&t);

	MyObject obj_updated = {.x = vals[3], .v = 0};
	MyObject *obj_out_updated = (MyObject *)slab_get(
		&sa, orbtree_get(&t, &obj_updated, off, my_obj_search, 0));
	fam_assert_eq(obj_out_updated->v, 1);

	MyObject obj_updated2 = {.x = vals[4], .v = 0};
	MyObject *obj_out_updated2 = (MyObject *)slab_get(
		&sa, orbtree_get(&t, &obj_updated2, off, my_obj_search, 0));
	fam_assert_eq(obj_out_updated2->v, 0);

	MyObject obj_updated3 = {.x = vals[7], .v = 0};
	MyObject *obj_out_updated3 = (MyObject *)slab_get(
		&sa, orbtree_get(&t, &obj_updated3, off, my_obj_search, 0));
	fam_assert_eq(obj_out_updated3->v, 1);

	for (int i = 0; i < size; i++) {
		MyObject obj = {.x = vals[i]};
		// OrbTreeNodeWrapper obj = wrap_obj(vals[i], 0);
		Ptr removed_ptr = orbtree_remove(&t, &obj, off, my_obj_search);
		fam_assert_eq(removed_ptr, arr[i]);
		slab_allocator_free(&sa, removed_ptr);
		my_obj_validate(&t);
	}

	// assert that all slabs are freed
	fam_assert_eq(slab_allocator_free_size(&sa),
				  slab_allocator_total_slabs(&sa));
	slab_allocator_cleanup(&sa);
}

Test(test_orbtree_range) {
	unsigned int off = offsetof(MyObject, node1);
	int size = 100;

	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), size + 10, size + 10);

	orbtree_init(&t, &sa);

	for (int i = 0; i < size; i++) {
		Ptr ptr = slab_allocator_allocate(&sa);
		MyObject *obj = (MyObject *)slab_get(&sa, ptr);
		obj->x = i;
		obj->v = 0;
		obj->ptr = ptr;
		fam_assert(!orbtree_put(&t, ptr, off, my_obj_search));
		my_obj_validate(&t);
	}

	my_obj_validate(&t);

	for (int j = 0; j < size; j++) {
		MyObject obj_in = {.x = j, .v = 0};
		for (int i = 0; i < size - j; i++) {
			MyObject *obj = (MyObject *)slab_get(
				&sa, orbtree_get(&t, &obj_in, off, my_obj_search, i));
			fam_assert_eq(obj->x, j + i);
		}
		fam_assert(!orbtree_get(&t, &obj_in, off, my_obj_search, size - j));
	}

	for (int i = 0; i < size; i++) {
		MyObject obj = {.x = i, .v = 0};
		Ptr removed_ptr = orbtree_remove(&t, &obj, off, my_obj_search);
		slab_allocator_free(&sa, removed_ptr);
		my_obj_validate(&t);
	}

	fam_assert_eq(slab_allocator_free_size(&sa),
				  slab_allocator_total_slabs(&sa));

	slab_allocator_cleanup(&sa);
}

Test(test_orbtree_range_rev) {
	unsigned int off = offsetof(MyObject, node1);
	int size = 100;

	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), size + 10, size + 10);

	orbtree_init(&t, &sa);

	for (int i = 0; i < size; i++) {
		Ptr ptr = slab_allocator_allocate(&sa);
		MyObject *obj = (MyObject *)slab_get(&sa, ptr);
		obj->x = i;
		obj->v = 0;
		obj->ptr = ptr;
		fam_assert(!orbtree_put(&t, ptr, off, my_obj_search));
		my_obj_validate(&t);
	}

	my_obj_validate(&t);

	for (int j = size - 1; j >= 0; j--) {
		MyObject obj_in = {.x = j, .v = 0};
		for (int i = j; i >= 0; i--) {
			MyObject *obj = (MyObject *)slab_get(
				&sa, orbtree_get(&t, &obj_in, off, my_obj_search, 0 - i));
			fam_assert_eq(obj->x, j - i);
		}
		fam_assert(!orbtree_get(&t, &obj_in, off, my_obj_search, 0 - (1 +
j)));
	}

	for (int i = 0; i < size; i++) {
		MyObject obj = {.x = i, .v = 0};
		Ptr removed_ptr = orbtree_remove(&t, &obj, off, my_obj_search);
		slab_allocator_free(&sa, removed_ptr);
		my_obj_validate(&t);
	}

	fam_assert_eq(slab_allocator_free_size(&sa),
				  slab_allocator_total_slabs(&sa));

	slab_allocator_cleanup(&sa);
}
*/

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
	byte key[32] = {7};
	byte iv[16] = {};
	cpsrng_test_seed(iv, key);

	// int size = 10 * 1000;
	int size = 100;
	int count = 1000;

	int64 *keys = mmap_allocate(size * sizeof(int64));
	OrbTree t = INIT_ORBTREE;
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
