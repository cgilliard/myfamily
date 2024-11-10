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

MyObject *create_my_object(int value, int value2) {
	Ptr ptr = slab_allocator_allocate(&sa);
	MyObject *ret = (MyObject *)slab_get(&sa, ptr);
	ret->ptr = ptr;
	ret->x = value;
	ret->v = value2;
	return ret;
}

OrbTreeNodeWrapper wrapper_for(MyObject *obj) {
	OrbTreeNodeWrapper ret = {.ptr = obj->ptr,
							  .offsetof = offsetof(MyObject, node1)};
	return ret;
}

OrbTreeNodeWrapper wrap_obj(int value, int value2) {
	MyObject *obj = create_my_object(value, value2);
	return wrapper_for(obj);
}

void free_wrapper(OrbTreeNodeWrapper wrapper) {
	slab_allocator_free(&sa, wrapper.ptr);
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
			cr_assert(current_black_count == *black_count);
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
	cr_assert_eq(lsum, orbtree_node_left_subtree_height(n));
	cr_assert_eq(rsum, orbtree_node_right_subtree_height(n));

	if (!orbtree_node_is_red(&obj->node1))
		current_black_count++;
	else {
		// check parent is not red (red property violation)
		MyObject *parent_obj = orbtree_node_parent(&obj->node1);
		OrbTreeNode *parent = &parent_obj->node1;
		cr_assert(parent);
		if (orbtree_node_is_red(parent)) {
			println("red violation at obj->x=%i", obj->x);
		}
		cr_assert(!orbtree_node_is_red(parent));
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
		cr_assert(!orbtree_node_is_red(&obj->node1));
		my_obj_validate_node(tree, root, &black_count, 0, false);
	}
}

MyTest(core, test_orbtree) {
	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), 100, 200);
	orbtree_init(&t, &sa);

	OrbTreeNodeWrapper obj1 = wrap_obj(1, 0);
	orbtree_put(&t, &obj1, my_obj_search);

	OrbTreeNodeWrapper obj2 = wrap_obj(0, 0);

	orbtree_put(&t, &obj2, my_obj_search);

	MyObject *obj2_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, &obj2, my_obj_search, 0));
	cr_assert(obj2_out);
	cr_assert_eq(obj2_out->x, 0);

	MyObject *obj1_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, &obj1, my_obj_search, 0));
	cr_assert(obj1_out);
	cr_assert_eq(obj1_out->x, 1);

	OrbTreeNodeWrapper obj3 = wrap_obj(2, 0);
	orbtree_put(&t, &obj3, my_obj_search);

	MyObject *obj3_out =
		(MyObject *)slab_get(&sa, orbtree_get(&t, &obj3, my_obj_search, 0));
	cr_assert(obj3_out);
	cr_assert_eq(obj3_out->x, 2);

	my_obj_validate(&t);

	free_wrapper(obj1);
	free_wrapper(obj2);
	free_wrapper(obj3);

	slab_allocator_cleanup(&sa);
}

MyTest(core, test_random_tree) {
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
		OrbTreeNodeWrapper obj1 = wrap_obj(x, 0);
		cr_assert(!orbtree_put(&t, &obj1, my_obj_search));
		arr[i] = obj1.ptr;
		my_obj_validate(&t);
	}

	my_obj_validate(&t);

	for (int i = 0; i < size; i++) {
		OrbTreeNodeWrapper obj = wrap_obj(vals[i], 0);
		MyObject *obj_out =
			(MyObject *)slab_get(&sa, orbtree_get(&t, &obj, my_obj_search, 0));
		cr_assert_eq(obj_out->ptr, arr[i]);
		slab_allocator_free(&sa, obj.ptr);
	}

	// try a node not in the tree
	OrbTreeNodeWrapper obj = wrap_obj(0, 1);
	Ptr obj_out = orbtree_get(&t, &obj, my_obj_search, 0);
	cr_assert_eq(obj_out, null);
	slab_allocator_free(&sa, obj.ptr);

	my_obj_validate(&t);

	// do transplants
	OrbTreeNodeWrapper replace1 = wrap_obj(vals[3], 1);
	Ptr rep_ptr = orbtree_put(&t, &replace1, my_obj_search);
	cr_assert_eq(rep_ptr, arr[3]);
	arr[3] = replace1.ptr;
	slab_allocator_free(&sa, rep_ptr);

	my_obj_validate(&t);
	OrbTreeNodeWrapper replace2 = wrap_obj(vals[7], 1);
	Ptr rep_ptr2 = orbtree_put(&t, &replace2, my_obj_search);
	cr_assert_eq(rep_ptr2, arr[7]);
	arr[7] = replace2.ptr;
	slab_allocator_free(&sa, rep_ptr2);

	my_obj_validate(&t);

	OrbTreeNodeWrapper obj_updated = wrap_obj(vals[3], 0);
	MyObject *obj_out_updated = (MyObject *)slab_get(
		&sa, orbtree_get(&t, &obj_updated, my_obj_search, 0));
	cr_assert_eq(obj_out_updated->v, 1);
	slab_allocator_free(&sa, obj_updated.ptr);

	obj_updated = wrap_obj(vals[7], 0);
	obj_out_updated = (MyObject *)slab_get(
		&sa, orbtree_get(&t, &obj_updated, my_obj_search, 0));
	cr_assert_eq(obj_out_updated->v, 1);
	slab_allocator_free(&sa, obj_updated.ptr);

	obj_updated = wrap_obj(vals[9], 0);
	obj_out_updated = (MyObject *)slab_get(
		&sa, orbtree_get(&t, &obj_updated, my_obj_search, 0));
	cr_assert_eq(obj_out_updated->v, 0);
	slab_allocator_free(&sa, obj_updated.ptr);

	for (int i = 0; i < size; i++) {
		OrbTreeNodeWrapper obj = wrap_obj(vals[i], 0);
		Ptr removed_ptr = orbtree_remove(&t, &obj, my_obj_search);
		cr_assert_eq(removed_ptr, arr[i]);
		slab_allocator_free(&sa, obj.ptr);
		slab_allocator_free(&sa, removed_ptr);
		my_obj_validate(&t);
	}

	cr_assert_eq(slab_allocator_free_size(&sa),
				 slab_allocator_total_slabs(&sa));

	slab_allocator_cleanup(&sa);
}

MyTest(core, test_orbtree_range) {
	int size = 1000;

	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), size + 10, size + 10);

	orbtree_init(&t, &sa);

	for (int i = 0; i < size; i++) {
		OrbTreeNodeWrapper obj = wrap_obj(i, 0);
		cr_assert(!orbtree_put(&t, &obj, my_obj_search));
		my_obj_validate(&t);
	}

	my_obj_validate(&t);

	for (int j = 0; j < size; j++) {
		OrbTreeNodeWrapper wrap = wrap_obj(j, 0);
		for (int i = 0; i < size - j; i++) {
			MyObject *obj = (MyObject *)slab_get(
				&sa, orbtree_get(&t, &wrap, my_obj_search, i));
			cr_assert_eq(obj->x, j + i);
		}

		slab_allocator_free(&sa, wrap.ptr);
	}

	for (int i = 0; i < size; i++) {
		OrbTreeNodeWrapper obj = wrap_obj(i, 0);
		Ptr removed_ptr = orbtree_remove(&t, &obj, my_obj_search);
		slab_allocator_free(&sa, obj.ptr);
		slab_allocator_free(&sa, removed_ptr);
		my_obj_validate(&t);
	}

	cr_assert_eq(slab_allocator_free_size(&sa),
				 slab_allocator_total_slabs(&sa));

	slab_allocator_cleanup(&sa);
}

int my_fun(int x, int y) {
	return x + y;
}

Object my_obj_fn(int x, int y) {
	return object_create_int(x * y);
}

MyTest(core, test_object) {
	Object obj1 = object_create_int(10);
	cr_assert_eq(object_value_of(&obj1), 10);
	cr_assert_eq(object_type(&obj1), ObjectTypeInt);
	Object obj2 = object_create_byte('a');
	cr_assert_eq(object_value_of(&obj2), 'a');
	cr_assert_eq(object_type(&obj2), ObjectTypeByte);
	Object obj3 = object_create_box(100);

	Object obj4 = object_create_function(&my_fun);
	cr_assert_eq(((int (*)(int, int))object_value_function(&obj4))(2, 4), 6);

	const Object obj5 = object_create_function(&my_obj_fn);
	const Object obj6 = $fn(obj5, 4, 96);
	cr_assert_eq(object_value_of(&obj6), 4 * 96);

	const Object obj7 = object_create_int(4);

	void *fptr = &my_fun;
	cr_assert_eq(((int (*)(int, int))fptr)(1, 2), 3);
}
