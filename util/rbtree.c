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

#include <assert.h>
#include <base/bitflags.h>
#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/macro_utils.h>
#include <base/panic.h>
#include <stdio.h>
#include <string.h>
#include <util/rbtree.h>

// Node id counter
static u64 node_id_counter = 10;

// RBTreeNode (overhead on 64 bit system is: four pointers * 8 = 32 bytes + Fat pointer's slab has a
// next pointer (8 bytes) and the FatPointer impl (with 32 bit slabs, which is default) has 16 bytes
// overhead. So, total overhead is 56 bytes. With 64 bit slabs there's an additional 8 bytes.
// It's also important to note that key/values are padded to 16 bytes. So that may introduce
// additional overhead. The 'data' section holds both key/value and 1 byte for node color.
typedef struct RBTreeNode {
	FatPtr self;
	struct RBTreeNode *right;
	struct RBTreeNode *left;
	struct RBTreeNode *parent;
#ifdef TEST
	u64 node_id;
	u64 pad;
#endif // TEST
	char data[];
} RBTreeNode;

// The internal RBTreeImpl storage data structure
typedef struct RBTreeImpl {
	u64 key_size;								// size of the keys
	u64 value_size;								// size of the values
	int (*compare)(const void *, const void *); // a comparion function (like qsort)
	RBTreeNode *root;							// pointer to the root node.
	u64 size;									// current size of the tree.
	bool send; // whether this RBTree can be sent to other threads (using global fam_alloc)
} RBTreeImpl;

// Iterator impl
typedef struct RBTreeIteratorImpl {
	int (*compare)(const void *, const void *);
	RBTreeNode *cur;
	RBTreeNode *min;
	RBTreeNode *max;
	RBTreeNode *stack[128];
	u64 key_size;
	u8 stack_pointer;
	bool send;
	RBTreeImpl *impl;
} RBTreeIteratorImpl;

// Data structure used for searching RBTrees.
typedef struct RBTreeNodePair {
	RBTreeNode *parent;
	RBTreeNode *self;
	bool is_right;
} RBTreeNodePair;

// utility macros
#define BOOLEAN_SIZE 1
#define VALUE_PAD(key_size) (key_size % 16)
#define DATA_SIZE(impl)                                                                            \
	(sizeof(RBTreeNode) + (impl->key_size + impl->value_size) * sizeof(char) +                     \
	 VALUE_PAD(impl->key_size) + BOOLEAN_SIZE)
#define RED_OFFSET(key_size, value_size) (key_size + value_size + VALUE_PAD(key_size))
#define RBTREE_FLAGS_RED 0
#define INIT_FLAGS(impl, node)                                                                     \
	({                                                                                             \
		u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                                 \
		node->data[offset] = 0;                                                                    \
	})

#define SET_RED(impl, node)                                                                        \
	({                                                                                             \
		u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                                 \
		BitFlags bf = {.flags = (u8 *)(node->data + offset), .capacity = 1};                       \
		bitflags_set(&bf, RBTREE_FLAGS_RED, true);                                                 \
	})
#define SET_BLACK(impl, node)                                                                      \
	({                                                                                             \
		u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                                 \
		BitFlags bf = {.flags = (u8 *)(node->data + offset), .capacity = 1};                       \
		bitflags_set(&bf, RBTREE_FLAGS_RED, false);                                                \
	})

#define IS_RED(impl, node)                                                                         \
	({                                                                                             \
		u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                                 \
		BitFlags bf = {.flags = (u8 *)(node->data + offset), .capacity = 1};                       \
		node != NIL &&bitflags_check(&bf, RBTREE_FLAGS_RED);                                       \
	})
#define IS_BLACK(impl, node)                                                                       \
	({                                                                                             \
		u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                                 \
		BitFlags bf = {.flags = (u8 *)(node->data + offset), .capacity = 1};                       \
		node == NIL || !bitflags_check(&bf, RBTREE_FLAGS_RED);                                     \
	})

#if defined(__clang__)
// Clang-specific pragma
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__) && !defined(__clang__)
// GCC-specific pragma
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#warning "Unknown compiler or platform. No specific warning pragmas applied."
#endif
const static RBTreeNode NIL_DEFN = {
	.self = null, .right = &NIL_DEFN, .left = &NIL_DEFN, .parent = &NIL_DEFN};
static RBTreeNode *NIL = &NIL_DEFN;
#pragma GCC diagnostic pop

// cleanup function for iterators
void rbtree_iterator_cleanup(RBTreeIteratorNc *ptr) {
	if (!nil(ptr->impl)) {
		// obtain referent to internal RBTreeImpl structure
		RBTreeIteratorImpl *impl = $(ptr->impl);
		// set appropriate chainguard status
		SendStateGuard _ = SetSend(impl->send);

		// free RBTreeImpl structure
		fam_free(&ptr->impl);
	}
}

// returns the next item in the iterator
bool rbtree_iterator_next(RBTreeIterator *ptr, RbTreeKeyValue *kv) {
	RBTreeIteratorImpl *impl = $(ptr->impl);

	// If the iterator is empty, we're done
	if (impl->cur == NIL && impl->stack_pointer == 0) {
		return false; // No more nodes to traverse
	}

	// Traverse the tree
	while (impl->cur != NIL || impl->stack_pointer > 0) {
		// Traverse left subtree
		if (impl->cur != NIL) {
			// based on worst case log(n) * 2 + 1 this should not be possible
			assert(impl->stack_pointer < 128);
			int v = 0;
			if (impl->min != NIL)
				v = impl->compare(impl->cur->data, impl->min->data);
			if (v < 0) {
				// we haven't hit the minimum yet
				impl->cur = impl->cur->right;
			} else {
				// Push the current node pointer onto the stack
				impl->stack[impl->stack_pointer++] = impl->cur;

				// Move to the left child
				impl->cur = impl->cur->left;
			}
		} else {
			// Pop the top node from the stack
			impl->cur = impl->stack[--impl->stack_pointer];

			// Store the current node's data to return
			void *ret = impl->cur->data;

			// check if we hit our max node. If so, next will return false.
			if (impl->cur == impl->max) {
				impl->cur = NIL;
				impl->stack_pointer = 0;
			}

			// Move to the right child after visiting this node
			impl->cur = impl->cur->right;

			kv->key = ret;
			kv->value = ret + VALUE_PAD(impl->key_size) + impl->key_size;
			break;
		}
	}
	return true;
}

void rbtree_free_node(RBTreeNode *ptr) {
	if (ptr != NIL) {
		// recursively free right node
		rbtree_free_node(ptr->right);
		// recursively free left node
		rbtree_free_node(ptr->left);
		// chain free current node
		fam_free(&ptr->self);
	}
}

// internal search function used by get/insert/delete.
void rbtree_search(RBTreeImpl *impl, const void *key, RBTreeNodePair *nodes) {
	nodes->parent = NIL;
	nodes->self = impl->root;

	int i = 0;
	while (nodes->self != NIL) {
#ifdef TEST
		// should never have a case where this is not true
		// only NIL accepted
		assert(nodes->self->parent);
		assert(nodes->self->left);
		assert(nodes->self->right);
#endif // TEST
		nodes->parent = nodes->self;
		int v = impl->compare(nodes->self->data, key);
		if (v == 0) {
			break;
		} else if (v < 0) {
			nodes->self = nodes->self->right;
			nodes->is_right = true;
		} else {
			nodes->self = nodes->self->left;
			nodes->is_right = false;
		}
	}
}

// do a left rotation
void rbtree_left_rotate(RBTreeImpl *impl, RBTreeNode *x) {

#ifdef TEST
	// should never have a case where this is not true
	// only NIL accepted
	assert(x->parent);
	assert(x->left);
	assert(x->right);
#endif // TEST

	RBTreeNode *y = x->right;

	// Move y's left subtree to x's right subtree
	x->right = y->left;
	if (y->left != NIL) {
		y->left->parent = x;
	}

	// Update y's parent to x's parent
	y->parent = x->parent;

	// If x was the root, now y becomes the root
	if (x->parent == NIL) {
		impl->root = y;
	} else if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}

	// Place x as y's left child
	y->left = x;
	x->parent = y;
}

// do a right rotation
void rbtree_right_rotate(RBTreeImpl *impl, RBTreeNode *x) {

#ifdef TEST
	// should never have a case where this is not true
	// only NIL accepted
	assert(x->parent);
	assert(x->left);
	assert(x->right);
#endif // TEST

	RBTreeNode *y = x->left;

	// Move y's right subtree to x's left subtree
	x->left = y->right;
	if (y->right != NIL) {
		y->right->parent = x;
	}

	// Update y's parent to x's parent
	y->parent = x->parent;

	// If x was the root, now y becomes the root
	if (x->parent == NIL) {
		impl->root = y;
	} else if (x == x->parent->right) {
		x->parent->right = y;
	} else {
		x->parent->left = y;
	}

	// Place x as y's right child
	y->right = x;
	x->parent = y;
}

// Fixup operation after insertion to maintain Red-Black Tree properties
void rbtree_put_fixup(RBTreeImpl *impl, RBTreeNode *k) {
	int i = 0;
	// Loop until the node is the root or its parent's color is black
	while (k != impl->root && IS_RED(impl, k->parent)) {
#ifdef TEST
		// Ensure that the parent, left, and right children are not NULL
		// The only exception is NIL nodes, which can be accepted
		assert(k->parent);
		assert(k->left);
		assert(k->right);
#endif // TEST

		if (k->parent == k->parent->parent->left) {
			// Case 1: Uncle is on the right
			RBTreeNode *u = k->parent->parent->right;

			if (IS_RED(impl, u)) {
				// Case 1a: Uncle is red
				// Recolor the parent and uncle to black
				SET_BLACK(impl, k->parent);
				SET_BLACK(impl, u);
				// Recolor the grandparent to red
				SET_RED(impl, k->parent->parent);
				// Move up the tree
				k = k->parent->parent;
			} else {
				// Case 1b: Uncle is black
				if (k == k->parent->right) {
					// Case 1b1: Node is a right child
					// Rotate left to make the node the left child
					k = k->parent;
					rbtree_left_rotate(impl, k);
				}
				// Recolor and rotate
				SET_BLACK(impl, k->parent);
				SET_RED(impl, k->parent->parent);
				rbtree_right_rotate(impl, k->parent->parent);
			}
		} else {
			// Case 2: Uncle is on the left
			RBTreeNode *u = k->parent->parent->left;

			if (IS_RED(impl, u)) {
				// Case 2a: Uncle is red
				// Recolor the parent and uncle to black
				SET_BLACK(impl, k->parent);
				SET_BLACK(impl, u);
				// Recolor the grandparent to red
				SET_RED(impl, k->parent->parent);
				// Move up the tree
				k = k->parent->parent;
			} else {
				// Case 2b: Uncle is black
				if (k == k->parent->left) {
					// Case 2b1: Node is a left child
					// Rotate right to make the node the right child
					k = k->parent;
					rbtree_right_rotate(impl, k);
				}
				// Recolor and rotate
				SET_BLACK(impl, k->parent);
				SET_RED(impl, k->parent->parent);
				rbtree_left_rotate(impl, k->parent->parent);
			}
		}
	}
	// Ensure the root is always black
	SET_BLACK(impl, impl->root);
}

// cleanup function selects appropriate SendStateGuard based on configuration and
// deallocates memory
void rbtree_cleanup(RBTreeNc *ptr) {
	// check non-initialized conditions
	if (!nil(ptr->impl)) {
		// obtain referent to internal RBTree structure
		RBTreeImpl *impl = $(ptr->impl);
		// set appropriate chainguard status
		SendStateGuard _ = SetSend(impl->send);

		rbtree_free_node(impl->root);

		// free RBTreeImpl structure
		fam_free(&ptr->impl);
	}
}

int rbtree_create(RBTree *ptr, const u64 key_size, const u64 value_size,
				  int (*compare)(const void *, const void *), bool send) {
	// validate input
	if (ptr == NULL || key_size == 0 || value_size == 0 || compare == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	RBTreeImpl *impl;

	{
		SendStateGuard _ = SetSend(send);
		// try to allocate required size for RBTreeImpl
		if (fam_alloc(&ptr->impl, sizeof(RBTreeImpl))) {
			// set the fatptr to null so it's not cleaned up in cleanup function
			ptr->impl = null;
			return -1;
		}
	}

	impl = $(ptr->impl);
	// initialize values of the RBTreeImpl structure
	impl->send = send;
	impl->key_size = key_size;
	impl->value_size = value_size;
	impl->compare = compare;
	impl->root = NIL;
	impl->size = 0;

	return 0;
}

int rbtree_put_swap(RBTree *ptr, const void *key, const void *value, RbTreeKeyValue *swap) {
	// validate input
	if (ptr == NULL || key == NULL || value == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	// obtain the impl from the fat ptr
	RBTreeImpl *impl = $(ptr->impl);
	// this pair is used to search
	RBTreeNodePair pair;
	// pointer for our node
	RBTreeNode *node;

	// perform search for the key
	rbtree_search(impl, key, &pair);

	// retreive the data size
	u64 size = DATA_SIZE(impl);
	if (pair.self != NIL) {
		node = $(pair.self->self);
		if (swap) {
			if (swap->key)
				memcpy(swap->key, node->data, impl->key_size);
			if (swap->value)
				memcpy(swap->value, node->data + VALUE_PAD(impl->key_size) + impl->key_size,
					   impl->value_size);
			swap->update = true;
		}
		memcpy(node->data, key, impl->key_size);
		memcpy(node->data + VALUE_PAD(impl->key_size) + impl->key_size, value, impl->value_size);
		return 0;
	}

	FatPtr self;

	// using fam_alloc allocate memory for this node
	{
		SendStateGuard _ = SetSend(impl->send);
		if (fam_alloc(&self, size)) {
			self = null;
			return -1;
		}
	}

	// using the allocated memory set node properties
	node = $(self);
	node->self = self;
	if (impl->root == NIL) {
		impl->root = node;
	} else if (pair.is_right) {
		pair.parent->right = node;
	} else {
		pair.parent->left = node;
	}
	node->right = NIL;			// always must be set to NIL at first.
	node->left = NIL;			// always must be set to NIL at first.
	node->parent = pair.parent; // set our parent pointer.

	// copy data
	memcpy(node->data, key, impl->key_size);
	// copy value
	memcpy(node->data + VALUE_PAD(impl->key_size) + impl->key_size, value, impl->value_size);
	// nodes are initially red
	INIT_FLAGS(impl, node);
	SET_RED(impl, node);

#ifdef TEST
	node->node_id = node_id_counter++;
#endif // TEST

	// increment the size counter
	impl->size++;

	// insert_fixup
	rbtree_put_fixup(impl, node);

	return 0;
}

// insert function
int rbtree_put(RBTree *ptr, const void *key, const void *value) {
	// validate input
	if (ptr == NULL || key == NULL || value == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	// obtain the impl from the fat ptr
	RBTreeImpl *impl = $(ptr->impl);
	// this pair is used to search
	RBTreeNodePair pair;
	// pointer for our node
	RBTreeNode *node;

	// perform search for the key
	rbtree_search(impl, key, &pair);

	// retreive the data size
	u64 size = DATA_SIZE(impl);
	if (pair.self != NIL) {
		node = $(pair.self->self);
		memcpy(node->data + VALUE_PAD(impl->key_size) + impl->key_size, value, impl->value_size);
		return 0;
	}

	FatPtr self;

	// using fam_alloc allocate memory for this node
	{
		SendStateGuard _ = SetSend(impl->send);
		if (fam_alloc(&self, size)) {
			self = null;
			return -1;
		}
	}

	// using the allocated memory set node properties
	node = $(self);
	node->self = self;
	if (impl->root == NIL) {
		impl->root = node;
	} else if (pair.is_right) {
		pair.parent->right = node;
	} else {
		pair.parent->left = node;
	}
	node->right = NIL;			// always must be set to NIL at first.
	node->left = NIL;			// always must be set to NIL at first.
	node->parent = pair.parent; // set our parent pointer.

	// copy data
	memcpy(node->data, key, impl->key_size);
	// copy value
	memcpy(node->data + VALUE_PAD(impl->key_size) + impl->key_size, value, impl->value_size);
	// nodes are initially red
	INIT_FLAGS(impl, node);
	SET_RED(impl, node);

#ifdef TEST
	node->node_id = node_id_counter++;
#endif // TEST

	// increment the size counter
	impl->size++;

	// insert_fixup
	rbtree_put_fixup(impl, node);

	return 0;
}

// find the successor node
RBTreeNode *rbtree_find_successor(RBTreeImpl *impl, RBTreeNode *x) {
	RBTreeNode *successor = x->right;
	while (successor->left != NIL) {
		successor = successor->left;
	}
	return successor;
}

// perform transplant operation
void rbtree_transplant(RBTreeImpl *impl, RBTreeNode *dst, RBTreeNode *src) {
	if (dst->parent == NIL)
		impl->root = src;
	else if (dst == dst->parent->left)
		dst->parent->left = src;
	else
		dst->parent->right = src;
	if (src != NIL) {
		src->parent = dst->parent;
	}
}

// set child's color to parent's
void set_color_based_on_parent(RBTreeImpl *impl, RBTreeNode *child, RBTreeNode *parent) {
	if (child != NIL) {
		if (IS_RED(impl, parent)) {
			SET_RED(impl, child);
		} else {
			SET_BLACK(impl, child);
		}
	}
}

// delete fixup
void rbtree_remove_fixup(RBTreeImpl *impl, RBTreeNode *parent, RBTreeNode *w, RBTreeNode *x) {
	int i = 0;
	while (x != impl->root && IS_BLACK(impl, x)) {
		if (w == parent->right) {
			// Case 1: Sibling is red
			if (IS_RED(impl, w)) {
				SET_BLACK(impl, w);
				SET_RED(impl, parent);
				rbtree_left_rotate(impl, parent);
				w = parent->right;
			}

			// Case 2: Sibling's children are both black
			if (IS_BLACK(impl, w->left) && IS_BLACK(impl, w->right)) {
				SET_RED(impl, w);
				x = parent;
				parent = parent->parent;
				if (x == x->parent->left) {
					w = x->parent->right;
				} else {
					w = x->parent->left;
				}
			} else {
				// Case 3: Sibling's right child is black, left child is red
				if (IS_BLACK(impl, w->right)) {
					SET_BLACK(impl, w->left);
					SET_RED(impl, w);
					rbtree_right_rotate(impl, w);
					w = parent->right;
				}

				// Case 4: Sibling's right child is red
				set_color_based_on_parent(impl, w, parent);
				SET_BLACK(impl, parent);
				SET_BLACK(impl, w->right);
				rbtree_left_rotate(impl, parent);
				x = impl->root; // Set x to root at the end, only once
			}
		} else {
			// Case 1: Sibling is red
			if (IS_RED(impl, w)) {
				SET_BLACK(impl, w);
				SET_RED(impl, parent);
				rbtree_right_rotate(impl, parent);
				w = parent->left;
			}
			// Case 2: Sibling's children are both black
			if (IS_BLACK(impl, w->right) && IS_BLACK(impl, w->left)) {
				SET_RED(impl, w);
				x = parent;
				parent = parent->parent;
				if (x == x->parent->left) {
					w = x->parent->right;
				} else {
					w = x->parent->left;
				}
			} else {
				// Case 3: Sibling's left child is black, right child is red
				if (IS_BLACK(impl, w->left)) {
					SET_BLACK(impl, w->right);
					SET_RED(impl, w);
					rbtree_left_rotate(impl, w);
					w = parent->left;
				}
				// Case 4: Sibling's left child is red
				set_color_based_on_parent(impl, w, parent);
				SET_BLACK(impl, parent);
				SET_BLACK(impl, w->left);
				rbtree_right_rotate(impl, parent);
				x = impl->root; // Set x to root at the end, only once
			}
		}
	}

	// Ensure x is black at the end of fixup
	SET_BLACK(impl, x);
}

// delete function
int rbtree_remove(RBTree *ptr, const void *key, RbTreeKeyValue *swap) {
	// validate input
	if (ptr == NULL || nil(ptr->impl) || key == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	// obtain impl from our opaque pointer
	RBTreeImpl *impl = $(ptr->impl);

	// search for the node based on this key.
	RBTreeNodePair pair;
	rbtree_search(impl, key, &pair);

	// this node doesn't exist, return -1
	if (pair.self == NIL)
		return -1;

	if (swap) {
		if (swap->key)
			memcpy(swap->key, pair.self->data, impl->key_size);
		if (swap->value)
			memcpy(swap->value, pair.self->data + VALUE_PAD(impl->key_size) + impl->key_size,
				   impl->value_size);
		swap->update = true;
	}

	// node exists, initialize variables
	RBTreeNode *node_to_delete = pair.self;
	RBTreeNode *x = NIL;
	RBTreeNode *w = NIL;
	RBTreeNode *parent = NIL;

	// only do fixups if the node_to_delete is black.
	bool do_fixup = IS_BLACK(impl, node_to_delete);

	if (node_to_delete->left == NIL) {
		// set variables for fixup and transplant
		x = node_to_delete->right;
		rbtree_transplant(impl, node_to_delete, node_to_delete->right);
		if (node_to_delete->parent->left == NIL)
			w = node_to_delete->parent->right;
		else
			w = node_to_delete->parent->left;
		if (x != NIL)
			parent = x->parent;
		else if (w != NIL)
			parent = w->parent;
	} else if (node_to_delete->right == NIL) {
		// set variables for fixup and transplant
		x = node_to_delete->left;
		rbtree_transplant(impl, node_to_delete, node_to_delete->left);
		w = node_to_delete->parent->left;
		parent = x->parent;
	} else {
		// two children case
		// find successor
		RBTreeNode *successor = rbtree_find_successor(impl, node_to_delete);
		// if it's black we need to do a fixup.
		do_fixup = IS_BLACK(impl, successor);

		x = successor->right;
		w = successor->parent->right;

		if (w->parent == node_to_delete) {
			w = node_to_delete->left;
			parent = successor;
		} else {
			parent = w->parent;
		}

		// if parent is not node_to_delete, do transplant and update children of successor
		if (successor->parent != node_to_delete) {
			rbtree_transplant(impl, successor, successor->right);
			successor->right = node_to_delete->right;
			successor->right->parent = successor;
		}

		// do final transpalnnnt and update including color match
		rbtree_transplant(impl, node_to_delete, successor);
		successor->left = node_to_delete->left;
		successor->left->parent = successor;
		set_color_based_on_parent(impl, successor, node_to_delete);
	}

	// if do_fixup, do fixup.
	if (do_fixup) {
		if (w != NIL && parent != NIL) {
			rbtree_remove_fixup(impl, parent, w, x);
		} else {
			// in these cases SET_BLACK only
			if (impl->size > 1)
				SET_BLACK(impl, impl->root);
		}
	}

	// Free the node which has been transplanted
	{
		SendStateGuard _ = SetSend(impl->send);
		fam_free(&node_to_delete->self);
	}

	impl->size--;

	return 0;
}

// get operation
const void *rbtree_get(const RBTree *ptr, const void *key) {
	// validate input
	if (ptr == NULL || key == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}

	// set impl
	RBTreeImpl *impl = $(ptr->impl);
	RBTreeNodePair pair;

	// search for the pair.
	rbtree_search(impl, key, &pair);

	// if found return value, otherwise NULL.
	if (pair.self != NIL)
		return pair.self->data + impl->key_size + VALUE_PAD(impl->key_size);
	else {
		return NULL;
	}
}

void *rbtree_get_mut(const RBTree *ptr, const void *key) {
	// validate input
	if (ptr == NULL || key == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}

	// set impl
	RBTreeImpl *impl = $(ptr->impl);
	RBTreeNodePair pair;

	// search for the pair.
	rbtree_search(impl, key, &pair);

	// if found return value, otherwise NULL.
	if (pair.self != NIL)
		return pair.self->data + impl->key_size + VALUE_PAD(impl->key_size);
	else {
		return NULL;
	}
}

// size in terms of number of nodes
i64 rbtree_size(const RBTree *ptr) {
	if (ptr == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	RBTreeImpl *impl = $(ptr->impl);
	return impl->size;
}

int rbtree_iterator_impl(RBTreeImpl *impl, RBTreeIterator *iter, const void *start_key,
						 bool start_inclusive, const void *end_key, bool end_inclusive) {

	RBTreeIteratorImpl *rbimpl = $(iter->impl);
	rbimpl->impl = impl;
	rbimpl->compare = impl->compare;
	rbimpl->stack_pointer = 0;
	rbimpl->send = impl->send;
	rbimpl->key_size = impl->key_size;
	rbimpl->cur = impl->root;
	rbimpl->min = NIL;
	rbimpl->max = NIL;

	if (start_key != NULL) {
		RBTreeNode *itt = impl->root;
		while (itt != NIL) {
			int v = impl->compare(itt->data, start_key);
			if (v == 0) {
				// exact match
				if (start_inclusive) {
					rbimpl->min = itt;
					break;
				}
			} else if (v < 0) {
				// continue down the chain to look for more
				itt = itt->right;
			} else {
				// higher value found update min
				rbimpl->min = itt;
				itt = itt->left;
			}
		}
	}

	if (end_key != NULL) {
		RBTreeNode *itt = impl->root;
		while (itt != NIL) {
			int v = impl->compare(itt->data, end_key);
			if (v == 0) {
				// exact match
				if (end_inclusive) {
					rbimpl->max = itt;
					break;
				}
			} else if (v < 0) {
				// lower value found update max
				rbimpl->max = itt;
				itt = itt->right;
			} else {
				// continue down the chain to look for more
				itt = itt->left;
			}
		}
	}

	return 0;
}

// reset the iterator (for reuse)
int rbtree_iterator_reset(const RBTree *ptr, RBTreeIterator *iter, const void *start_key,
						  bool start_inclusive, const void *end_key, bool end_inclusive) {
	if (iter == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	RBTreeImpl *impl = $(ptr->impl);
	return rbtree_iterator_impl(impl, iter, start_key, start_inclusive, end_key, end_inclusive);
}

// return iterator object
int rbtree_iterator(const RBTree *ptr, RBTreeIterator *iter, const void *start_key,
					bool start_inclusive, const void *end_key, bool end_inclusive) {
	if (ptr == NULL || nil(ptr->impl)) {
		SetErr(IllegalArgument);
		return -1;
	}

	RBTreeImpl *impl = $(ptr->impl);

	{
		SendStateGuard _ = SetSend(impl->send);
		if (fam_alloc(&iter->impl, sizeof(RBTreeIteratorImpl))) {
			iter->impl = null;
			return -1;
		}
	}
	return rbtree_iterator_impl(impl, iter, start_key, start_inclusive, end_key, end_inclusive);
}

#ifdef TEST
void rbtree_validate_node(const RBTree *ptr, const RBTreeNode *node, int *black_count,
						  int current_black_count, u64 ids[100]) {
	ids[current_black_count] = node->node_id;
	RBTreeImpl *impl = $(ptr->impl);
	u64 key_size = impl->key_size;
	u64 value_size = impl->value_size;

	// Base case: when we reach a NIL
	if (node == NIL) {
		// If this is the first NIL node reached, set the black count
		if (*black_count == 0) {
			*black_count = current_black_count; // Set the black count for the first path
		} else {
			// Check for black count consistency
			assert(current_black_count == *black_count);
		}
		return; // Return for NIL nodes
	}

	// Increment black count if the current node is black
	if (IS_BLACK(impl, node)) {
		current_black_count++;
	} else {
		//   Check if the node is red
		//   If the parent is red, return false (Red property violation)
		assert(!(node->parent != NIL && IS_RED(impl, node->parent)));
	}

	// Recursive calls for left and right children
	rbtree_validate_node(ptr, node->left, black_count, current_black_count, ids);
	rbtree_validate_node(ptr, node->right, black_count, current_black_count, ids);
}

void rbtree_validate(const RBTree *ptr) {
	RBTreeImpl *impl = $(ptr->impl);
	int black_count = 0;
	u64 ids[100];
	// Validate from the root and check if the root is black
	if (impl->root != NIL) {
		assert(IS_BLACK(impl, impl->root));
		rbtree_validate_node(ptr, impl->root, &black_count, 0, ids);
	}
}

void rbtree_node_depth(RBTreeImpl *impl, RBTreeNode *node, u64 *max_depth, u64 cur_depth) {
	if (cur_depth > *max_depth)
		*max_depth = cur_depth;
	if (node->right != NIL)
		rbtree_node_depth(impl, node->right, max_depth, cur_depth + 1);
	if (node->left != NIL)
		rbtree_node_depth(impl, node->left, max_depth, cur_depth + 1);
}

u64 rbtree_max_depth(const RBTree *ptr) {
	RBTreeImpl *impl = $(ptr->impl);
	u64 max_depth = 0;
	rbtree_node_depth(impl, impl->root, &max_depth, 1);
	return max_depth;
}

// Function to print a single node with its color
void rbtree_print_node(const RBTree *ptr, const RBTreeNode *node, int depth) {
	RBTreeImpl *impl = $(ptr->impl);
	if (node == NIL) {
		for (int i = 0; i < depth; i++) {
			printf("    ");
		}
		printf("%llu (%s)\n", node->node_id, (IS_BLACK(impl, node)) ? "B" : "R");
		return;
	}

	// Print the right child first (for visual representation)
	rbtree_print_node(ptr, node->right, depth + 1);

	// Indent according to depth
	for (int i = 0; i < depth; i++) {
		printf("    ");
	}

	// Print the current node with a clearer representation
	printf("%llu (%s)\n", node->node_id, (IS_BLACK(impl, node)) ? "B" : "R");

	// Print the left child
	rbtree_print_node(ptr, node->left, depth + 1);
}

// Function to print the entire tree
void rbtree_print(const RBTree *ptr) {
	RBTreeImpl *impl = $(ptr->impl);

	printf("Red-Black Tree (root = %llu)\n", impl->root->node_id);
	printf("===================================\n"); // Separator for better clarity
	rbtree_print_node(ptr, impl->root, 0);
	printf("===================================\n"); // Separator for better clarity
}

#endif // TEST
