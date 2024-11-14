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
#include <stddef.h>

Suite(core);

typedef struct MyObject {
	int x;
	int v;
	Ptr ptr;
	OrbTreeNode node1;
	OrbTreeNode node2;
} MyObject;

SlabAllocator sa;

MyObject *create_my_object(int value, int value2) {
	Ptr ptr = slab_allocator_allocate(&sa);
	MyObject *ret = (MyObject *)slab_get(&sa, ptr);
	ret->ptr = ptr;
	ret->x = value;
	ret->v = value2;
	return ret;
}

OrbTreeNodeSearchWrapper wrapper_for(MyObject *obj) {
	OrbTreeNodeSearchWrapper ret = {.ptr = obj,
									.offsetof = offsetof(MyObject, node1)};
	return ret;
}

OrbTreeNodeSearchWrapper wrap_obj(int value, int value2) {
	MyObject *obj = create_my_object(value, value2);
	return wrapper_for(obj);
}

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
				current_black_count;  // Set the black count for the first path
		} else {
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
		(MyObject *)slab_get(&sa, orbtree_get(&t, obj2, off, my_obj_search, 0));
	fam_assert(obj2_out);
	fam_assert_eq(obj2_out->x, 0);

	MyObject *obj1_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, obj1, off, my_obj_search, 0));
	fam_assert(obj1_out);
	fam_assert_eq(obj1_out->x, 1);

	Ptr ptr3 = slab_allocator_allocate(&sa);
	MyObject *obj3 = (MyObject *)slab_get(&sa, ptr3);
	obj3->x = 2;
	obj3->v = 0;
	orbtree_put(&t, ptr3, off, my_obj_search);

	MyObject *obj3_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, obj3, off, my_obj_search, 0));
	fam_assert(obj3_out);
	fam_assert_eq(obj3_out->x, 2);

	my_obj_validate(&t);

	slab_allocator_free(&sa, ptr1);
	slab_allocator_free(&sa, ptr2);
	slab_allocator_free(&sa, ptr3);

	slab_allocator_cleanup(&sa);
}
