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
#include <crypto/cpsrng.h>

Suite(core);

typedef struct MyObject {
	int x;
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
	loop {
		int x1 = container_of(cur, MyObject, node1)->x;
		int x2 = container_of(value, MyObject, node1)->x;

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

MyObject *create_my_object(int value) {
	Ptr ptr = slab_allocator_allocate(&sa);
	MyObject *ret = (MyObject *)slab_get(&sa, ptr);
	ret->ptr = ptr;
	ret->x = value;
	return ret;
}

OrbTreeNodeWrapper wrapper_for(MyObject *obj) {
	OrbTreeNodeWrapper ret = {.ptr = obj->ptr, offsetof(MyObject, node1)};
	return ret;
}

OrbTreeNodeWrapper wrap_obj(int value) {
	MyObject *obj = create_my_object(value);
	return wrapper_for(obj);
}

void free_wrapper(OrbTreeNodeWrapper wrapper) {
	slab_allocator_free(&sa, wrapper.ptr);
}

void my_obj_print_node1(Ptr ptr, int depth) {
	if (depth > 10) panic("depth > 10");
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

	println("%i (%s,ptr=%u)", obj->x,
			orbtree_node_is_red(&obj->node1) ? "R" : "B", obj->ptr);

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

void my_object_orbtree_print(const OrbTree *tree) {
	Ptr root = orbtree_root(tree);
	my_obj_print_root(root);
}

void my_obj_validate_node(const OrbTree *tree, Ptr node, int *black_count,
						  int current_black_count) {
	if (node == null) {
		if (*black_count == 0) {
			*black_count =
				current_black_count;  // Set the black count for the first path
		} else {
			// Check for black count
			// consistency
			cr_assert(current_black_count == *black_count);
		}
		return;
	}

	MyObject *obj = (MyObject *)slab_get(&sa, node);
	if (!orbtree_node_is_red(&obj->node1))
		current_black_count++;
	else {
		// check parent is not red (red property violation)
		OrbTreeNode *parent = orbtree_node_parent(&obj->node1);
		cr_assert(parent);
		cr_assert(!orbtree_node_is_red(parent));
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

	my_obj_validate_node(tree, right_ptr, black_count, current_black_count);
	my_obj_validate_node(tree, left_ptr, black_count, current_black_count);
}

void my_obj_validate(const OrbTree *tree) {
	int black_count = 0;
	Ptr root = orbtree_root(tree);
	// Validate from the root and
	// check if the root is black
	if (root != null) {
		MyObject *obj = (MyObject *)slab_get(&sa, root);
		cr_assert(!orbtree_node_is_red(&obj->node1));
		my_obj_validate_node(tree, root, &black_count, 0);
	}
}

MyTest(core, test_orbtree) {
	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), 100, 200);
	orbtree_init(&t, &sa);

	OrbTreeNodeWrapper obj1 = wrap_obj(1);
	orbtree_put(&t, &obj1, my_obj_search);

	OrbTreeNodeWrapper obj2 = wrap_obj(0);

	orbtree_put(&t, &obj2, my_obj_search);

	MyObject *obj2_out = orbtree_get(&t, &obj2, my_obj_search, 0);
	cr_assert(obj2_out);
	cr_assert_eq(obj2_out->x, 0);

	MyObject *obj1_out = orbtree_get(&t, &obj1, my_obj_search, 0);
	cr_assert(obj1_out);
	cr_assert_eq(obj1_out->x, 1);

	OrbTreeNodeWrapper obj3 = wrap_obj(2);
	orbtree_put(&t, &obj3, my_obj_search);

	MyObject *obj3_out = orbtree_get(&t, &obj3, my_obj_search, 0);
	cr_assert(obj3_out);
	cr_assert_eq(obj3_out->x, 2);

	my_object_orbtree_print(&t);
	my_obj_validate(&t);

	free_wrapper(obj1);
	free_wrapper(obj2);
	free_wrapper(obj3);

	slab_allocator_cleanup(&sa);
}

MyTest(core, test_random_tree) {
	// seed rng for reproducibility
	byte key[32] = {7};
	byte iv[16] = {};
	cpsrng_test_seed(iv, key);

	int size = 100;
	Ptr arr[size];

	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), 1000, 2000);

	orbtree_init(&t, &sa);

	for (int i = 0; i < size; i++) {
		int x;
		cpsrng_rand_int(&x);
		OrbTreeNodeWrapper obj1 = wrap_obj(x);
		orbtree_put(&t, &obj1, my_obj_search);
		arr[i] = obj1.ptr;
	}

	my_object_orbtree_print(&t);
	my_obj_validate(&t);

	for (int i = 0; i < size; i++) slab_allocator_free(&sa, arr[i]);
	slab_allocator_cleanup(&sa);
}
