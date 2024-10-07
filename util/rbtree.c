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
#include <base/chain_alloc.h>
#include <base/macro_utils.h>
#include <base/panic.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <util/rbtree.h>

static u64 node_id_counter = 10;

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

typedef struct RBTreeImpl {
	u64 key_size;
	u64 value_size;
	int (*compare)(const void *, const void *);
	RBTreeNode *root;
	u64 size;
	bool send;
} RBTreeImpl;

typedef struct RBTreeIteratorImpl {
	RBTreeNode *cur;
	RBTreeNode *stack[128];
	u64 key_size;
	u8 stack_pointer;
	bool send;
} RBTreeIteratorImpl;

typedef struct RBTreeNodePair {
	RBTreeNode *parent;
	RBTreeNode *self;
	bool is_right;
} RBTreeNodePair;

#define BOOLEAN_SIZE 1
#define VALUE_PAD(key_size) (16 - (key_size % 16))
#define DATA_SIZE(impl)                                                                            \
	(sizeof(RBTreeNode) + (impl->key_size + impl->value_size) * sizeof(char) +                     \
	 VALUE_PAD(impl->key_size) + BOOLEAN_SIZE)
#define RED_OFFSET(key_size, value_size) (key_size + value_size + VALUE_PAD(key_size))
#define SET_RED(impl, node)                                                                        \
	({                                                                                             \
		if (node && node != NIL) {                                                                 \
			u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                             \
			*(bool *)(node->data + offset) = true;                                                 \
		} else                                                                                     \
			;                                                                                      \
		/*printf("WARN: SET_RED on invalid node. node is NIL.\n");*/                               \
	})
#define SET_BLACK(impl, node)                                                                      \
	({                                                                                             \
		if (node && node != NIL) {                                                                 \
			u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                             \
			*(bool *)(node->data + offset) = false;                                                \
		} else                                                                                     \
			;                                                                                      \
		/*printf("WARN: SET_BLACK on invalid node. node is NIL.\n");*/                             \
	})

#define IS_RED(impl, node)                                                                         \
	({                                                                                             \
		bool ret = false;                                                                          \
		u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                                 \
		if (node && node != NIL && node->data && *(bool *)(node->data + offset))                   \
			ret = true;                                                                            \
		ret;                                                                                       \
	})
#define IS_BLACK(impl, node)                                                                       \
	({                                                                                             \
		bool ret = true;                                                                           \
		u64 offset = RED_OFFSET(impl->key_size, impl->value_size);                                 \
		if (node && node != NIL && node->data && *(bool *)(node->data + offset))                   \
			ret = false;                                                                           \
		ret;                                                                                       \
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

void rbtree_iterator_cleanup(RBTreeIteratorNc *ptr) {
	if (!nil(ptr->impl)) {
		// obtain referent to internal RBTreeImpl structure
		RBTreeIteratorImpl *impl = $Ref(&ptr->impl);
		// set appropriate chainguard status
		ChainGuard _ = ChainSend(impl->send);

		// free RBTreeImpl structure
		chain_free(&ptr->impl);
	}
}

bool rbtree_iterator_next(RBTreeIterator *ptr, RbTreeKeyValue *kv) {
	RBTreeIteratorImpl *impl = $Ref(&ptr->impl);

	// If the iterator is empty, we're done
	if (impl->cur == NIL && impl->stack_pointer == 0) {
		return false; // No more nodes to traverse
	}

	// Traverse the tree
	while (impl->cur != NIL || impl->stack_pointer > 0) {
		// Traverse left subtree
		if (impl->cur != NIL) {
#ifdef TEST
			// should never have a case where this is not true
			// only NIL accepted
			assert(impl->cur->parent);
			assert(impl->cur->left);
			assert(impl->cur->right);
#endif // TEST

			// based on worst case log(n) * 2 + 1 this should not be possible
			assert(impl->stack_pointer < 128);
			// Push the current node pointer onto the stack
			impl->stack[impl->stack_pointer++] = impl->cur;

			// Move to the left child
			impl->cur = impl->cur->left;
		} else {
			// Pop the top node from the stack
			impl->cur = impl->stack[--impl->stack_pointer];

#ifdef TEST
			// should never have a case where this is not true
			// only NIL accepted
			assert(impl->cur->parent);
			assert(impl->cur->left);
			assert(impl->cur->right);
#endif // TEST

			// Store the current node's data to return
			void *ret = impl->cur->data;

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
		chain_free(&ptr->self);
	}
}

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

void rbtree_insert_fixup(RBTreeImpl *impl, RBTreeNode *k) {
	int i = 0;
	while (k != impl->root && IS_RED(impl, k->parent)) {
#ifdef TEST
		// should never have a case where this is not true
		// only NIL accepted
		assert(k->parent);
		assert(k->left);
		assert(k->right);
#endif // TEST

		if (k->parent == k->parent->parent->left) {
			RBTreeNode *u = k->parent->parent->right;

			if (IS_RED(impl, u)) {
				SET_BLACK(impl, k->parent);
				SET_BLACK(impl, u);
				SET_RED(impl, k->parent->parent);
				k = k->parent->parent;
			} else {
				if (k == k->parent->right) {
					k = k->parent;
					rbtree_left_rotate(impl, k);
				}
				SET_BLACK(impl, k->parent);
				SET_RED(impl, k->parent->parent);
				rbtree_right_rotate(impl, k->parent->parent);
			}
		} else {
			RBTreeNode *u = k->parent->parent->left;

			if (IS_RED(impl, u)) {
				SET_BLACK(impl, k->parent);
				SET_BLACK(impl, u);
				SET_RED(impl, k->parent->parent);
				k = k->parent->parent;
			} else {
				if (k == k->parent->left) {
					k = k->parent;
					rbtree_right_rotate(impl, k);
				}
				SET_BLACK(impl, k->parent);
				SET_RED(impl, k->parent->parent);
				rbtree_left_rotate(impl, k->parent->parent);
			}
		}
	}
	SET_BLACK(impl, impl->root);
}

// cleanup function selects appropriate ChainGuard based on configuration and
// deallocates memory
void rbtree_cleanup(RBTreeNc *ptr) {
	// check non-initialized conditions
	if (!nil(ptr->impl)) {
		// obtain referent to internal RBTree structure
		RBTreeImpl *impl = $Ref(&ptr->impl);
		// set appropriate chainguard status
		ChainGuard _ = ChainSend(impl->send);

		rbtree_free_node(impl->root);

		// free RBTreeImpl structure
		chain_free(&ptr->impl);
	}
}

int rbtree_build(RBTree *ptr, const u64 key_size, const u64 value_size,
				 int (*compare)(const void *, const void *), bool send) {
	// validate input
	if (ptr == NULL || key_size == 0 || value_size == 0 || compare == NULL) {
		errno = EINVAL;
		return -1;
	}

	RBTreeImpl *impl;

	{
		ChainGuard _ = ChainSend(send);
		// try to allocate required size for RBTreeImpl
		if (chain_malloc(&ptr->impl, sizeof(RBTreeImpl))) {
			// set the fatptr to null so it's not cleaned up in cleanup function
			ptr->impl = null;
			return -1;
		}

		impl = $Ref(&ptr->impl);
		// initialize values of the RBTreeImpl structure
		impl->send = send;
		impl->key_size = key_size;
		impl->value_size = value_size;
		impl->compare = compare;
		impl->root = NIL;
		impl->size = 0;
	}

	return 0;
}

int rbtree_insert(RBTree *ptr, const void *key, const void *value) {
	if (ptr == NULL || key == NULL || value == NULL) {
		errno = EINVAL;
		return -1;
	}

	RBTreeImpl *impl = $Ref(&ptr->impl);
	RBTreeNodePair pair;
	RBTreeNode *node;

	rbtree_search(impl, key, &pair);

	if (pair.self != NIL)
		return -1;

	u64 size = DATA_SIZE(impl);
	FatPtr self;

	{
		ChainGuard _ = ChainSend(impl->send);
		if (chain_malloc(&self, size))
			return -1;
	}

	node = $Ref(&self);
	node->self = self;
	if (impl->root == NIL) {
		impl->root = node;
	} else if (pair.is_right) {
		pair.parent->right = node;
	} else {
		pair.parent->left = node;
	}
	node->right = NIL;
	node->left = NIL;
	node->parent = pair.parent;

	memcpy(node->data, key, impl->key_size);
	memcpy(node->data + VALUE_PAD(impl->key_size) + impl->key_size, value, impl->value_size);
	SET_RED(impl, node);

#ifdef TEST
	node->node_id = node_id_counter++;
#endif // TEST

	impl->size++;

	rbtree_insert_fixup(impl, node);

	return 0;
}

RBTreeNode *rbtree_find_successor(RBTreeImpl *impl, RBTreeNode *x) {
	RBTreeNode *successor = x->right;
	while (successor->left != NIL) {
		successor = successor->left;
	}
	return successor;
}

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

void set_color_based_on_parent(RBTreeImpl *impl, RBTreeNode *child, RBTreeNode *parent) {
	if (child != NIL) {
		if (IS_RED(impl, parent)) {
			SET_RED(impl, child);
		} else {
			SET_BLACK(impl, child);
		}
	}
}

void rbtree_delete_fixup(RBTreeImpl *impl, RBTreeNode *parent, RBTreeNode *w, RBTreeNode *x) {
	// printf("rbtree_delete_fixup\n");
	int i = 0;
	while (x != impl->root && IS_BLACK(impl, x)) {
		// printf("i=%i,x=%llu,w=%llu,p=%llu\n", i++, x->node_id, w->node_id, parent->node_id);
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

int rbtree_delete(RBTree *ptr, const void *key) {
	if (ptr == NULL || nil(ptr->impl) || key == NULL) {
		errno = EINVAL;
		return -1;
	}

	RBTreeImpl *impl = $Ref(&ptr->impl);

	RBTreeNodePair pair;
	rbtree_search(impl, key, &pair);

	if (pair.self == NIL)
		return -1;

	// printf("deleting node %llu\n", pair.self->node_id);

	RBTreeNode *node_to_delete = pair.self;
	RBTreeNode *x;
	bool do_fixup = IS_BLACK(impl, node_to_delete);

	if (node_to_delete->left == NIL) {
		// printf("caseA\n");
		x = node_to_delete->right;
		rbtree_transplant(impl, node_to_delete, node_to_delete->right);
	} else if (node_to_delete->right == NIL) {
		// printf("caseB\n");
		x = node_to_delete->left;
		rbtree_transplant(impl, node_to_delete, node_to_delete->left);
	} else {
		// printf("caseC\n");
		RBTreeNode *successor = rbtree_find_successor(impl, node_to_delete);
		x = successor->right;

		if (successor->parent == node_to_delete) {
			if (x != NIL)
				x->parent = successor;
		} else {
			rbtree_transplant(impl, successor, successor->right);
			successor->right = node_to_delete->right;
			successor->right->parent = successor;
		}

		rbtree_transplant(impl, node_to_delete, successor);
		successor->left = node_to_delete->left;
		successor->left->parent = successor;
		set_color_based_on_parent(impl, successor, node_to_delete);
	}

	if (do_fixup) {
		if (x != NIL) {
			RBTreeNode *sibling;
			if (x->parent->left == x)
				sibling = x->parent->right;
			else
				sibling = x->parent->left;
			rbtree_delete_fixup(impl, x->parent, sibling, x);
		} else {
			RBTreeNode *sibling;
			if (node_to_delete->parent->left == NIL)
				sibling = node_to_delete->parent->right;
			else
				sibling = node_to_delete->parent->left;
			// rbtree_print(ptr);
			if (sibling != NIL && node_to_delete->parent != NIL)
				rbtree_delete_fixup(impl, node_to_delete->parent, sibling, x);
		}
	}

	// Free the node which has been transplanted
	{
		ChainGuard _ = ChainSend(impl->send);
		chain_free(&node_to_delete->self);
	}

	impl->size--;

	return 0;
}
const void *rbtree_get(const RBTree *ptr, const void *key) {
	if (ptr == NULL || key == NULL) {
		errno = EINVAL;
		return NULL;
	}

	RBTreeImpl *impl = $Ref(&ptr->impl);
	RBTreeNodePair pair;

	rbtree_search(impl, key, &pair);

	if (pair.self != NIL)
		return pair.self->data + impl->key_size + VALUE_PAD(impl->key_size);
	else {
		return NULL;
	}
}
i64 rbtree_size(const RBTree *ptr) {
	if (ptr == NULL) {
		errno = EINVAL;
		return -1;
	}

	RBTreeImpl *impl = $Ref(&ptr->impl);
	return impl->size;
}
int rbtree_iterator(const RBTree *ptr, RBTreeIterator *iter) {
	if (ptr == NULL || nil(ptr->impl)) {
		errno = EINVAL;
		return -1;
	}

	RBTreeImpl *impl = $Ref(&ptr->impl);
	ChainGuard _ = ChainSend(impl->send);

	chain_malloc(&iter->impl, sizeof(RBTreeIteratorImpl));
	RBTreeIteratorImpl *rbimpl = $Ref(&iter->impl);
	rbimpl->stack_pointer = 0;
	rbimpl->cur = impl->root;
	rbimpl->send = impl->send;
	rbimpl->key_size = impl->key_size;
	return 0;
}

#ifdef TEST
void rbtree_validate_node(const RBTree *ptr, const RBTreeNode *node, int *black_count,
						  int current_black_count, u64 ids[100]) {
	ids[current_black_count] = node->node_id;
	RBTreeImpl *impl = $Ref(&ptr->impl);
	u64 key_size = impl->key_size;
	u64 value_size = impl->value_size;

	// Base case: when we reach a NIL
	if (node == NIL) {
		// If this is the first NIL node reached, set the black count
		if (*black_count == 0) {
			*black_count = current_black_count; // Set the black count for the first path
		} else {
			// Check for black count consistency
			if (current_black_count != *black_count) {
				for (int i = 0; i < current_black_count; i++) {
					printf("%llu -> ", ids[i]);
				}
				printf("NIL (%d) (expected=%d)\n", current_black_count, *black_count);
			}
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
	RBTreeImpl *impl = $Ref(&ptr->impl);
	int black_count = 0;
	u64 ids[100];
	// Validate from the root and check if the root is black
	if (impl->root != NIL) {
		assert(IS_BLACK(impl, impl->root));
		rbtree_validate_node(ptr, impl->root, &black_count, 0, ids);
	}
}

void rbtree_node_depth(RBTreeImpl *impl, RBTreeNode *node, int *max_depth, int cur_depth) {
	if (cur_depth > *max_depth)
		*max_depth = cur_depth;
	if (node->right != NIL)
		rbtree_node_depth(impl, node->right, max_depth, cur_depth + 1);
	if (node->left != NIL)
		rbtree_node_depth(impl, node->left, max_depth, cur_depth + 1);
}

int rbtree_max_depth(const RBTree *ptr) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	int max_depth = 0;
	rbtree_node_depth(impl, impl->root, &max_depth, 1);
	return max_depth;
}

// Function to print a single node with its color
void rbtree_print_node(const RBTree *ptr, const RBTreeNode *node, int depth) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
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
	RBTreeImpl *impl = $Ref(&ptr->impl);

	printf("Red-Black Tree (root = %llu)\n", impl->root->node_id);
	printf("===================================\n"); // Separator for better clarity
	rbtree_print_node(ptr, impl->root, 0);
	printf("===================================\n"); // Separator for better clarity
}

#endif // TEST
