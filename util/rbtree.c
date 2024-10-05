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
#include <base/resources.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <util/rbtree.h>

#define BOOLEAN_SIZE 1
#define KEY_PAD 0
#define VALUE_PAD(key_size) (16 - (key_size % 16))
#define RED_OFFSET(key_size, value_size) (KEY_PAD + key_size + value_size + VALUE_PAD(key_size))
#define SET_RED(impl, node)                                                                        \
	({                                                                                             \
		assert(node);                                                                              \
		assert(node != NIL);                                                                       \
		*(bool *)(node->data + RED_OFFSET(impl->key_size, impl->value_size)) = true;               \
	})
#define SET_BLACK(impl, node)                                                                      \
	({                                                                                             \
		if (node && node != NIL) {                                                                 \
			*(bool *)(node->data + RED_OFFSET(impl->key_size, impl->value_size)) = false;          \
			printf("setting node %llu black\n", fat_ptr_id(&node->self));                          \
		}                                                                                          \
	})

#define IS_RED(impl, node)                                                                         \
	({                                                                                             \
		bool ret = false;                                                                          \
		if (node && node != NIL && node->data &&                                                   \
			*(bool *)(node->data + RED_OFFSET(impl->key_size, impl->value_size)))                  \
			ret = true;                                                                            \
		ret;                                                                                       \
	})
#define IS_BLACK(impl, node)                                                                       \
	({                                                                                             \
		bool ret = true;                                                                           \
		if (node && node != NIL && node->data &&                                                   \
			*(bool *)(node->data + RED_OFFSET(impl->key_size, impl->value_size)))                  \
			ret = false;                                                                           \
		ret;                                                                                       \
	})

typedef struct RBTreeNode {
	FatPtr self;
	struct RBTreeNode *right;
	struct RBTreeNode *left;
	struct RBTreeNode *parent;
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

static RBTreeNode NIL_DEFN = {.self = null};
static RBTreeNode *NIL = &NIL_DEFN;
static RBTreeNode NONE_DEFN = {.self = null};
static RBTreeNode *NONE = &NONE_DEFN;

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
	if (!impl->cur && impl->stack_pointer == 0) {
		return false; // No more nodes to traverse
	}

	// Traverse the tree
	while (impl->cur != NIL || impl->stack_pointer > 0) {
		// Traverse left subtree
		if (impl->cur != NIL) {
			if (impl->stack_pointer < 128) {
				// Push the current node pointer onto the stack
				impl->stack[impl->stack_pointer++] = impl->cur;
			} else {
				panic("Iterator stack overflow");
			}

			// Move to the left child
			impl->cur = impl->cur->left;
		} else {
			// Pop the top node from the stack
			impl->cur = impl->stack[--impl->stack_pointer];

			// Store the current node's data to return
			void *ret = impl->cur->data;

			// Move to the right child after visiting this node
			impl->cur = impl->cur->right;

			kv->key = ret + KEY_PAD;
			kv->value = ret + KEY_PAD + VALUE_PAD(impl->key_size) + impl->key_size;
			return true; // Return the node's data
		}
	}

	return false; // Traversal complete
}

// Utility function to perform left rotation
void leftRotate(RBTreeImpl *impl, RBTreeNode *x) {
	RBTreeNode *y = x->right;
	x->right = y->left;
	y->left->parent = x;
	y->parent = x->parent;
	if (x->parent == NONE) {
		impl->root = y;
	} else if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}
	y->left = x;
	x->parent = y;
}

// Utility function to perform right rotation
void rightRotate(RBTreeImpl *impl, RBTreeNode *x) {
	RBTreeNode *y = x->left;
	x->left = y->right;
	y->right->parent = x;
	y->parent = x->parent;
	if (x->parent == NONE) {
		impl->root = y;
	} else if (x == x->parent->right) {
		x->parent->right = y;
	} else {
		x->parent->left = y;
	}
	y->right = x;
	x->parent = y;
}

void rbtree_fix_up(RBTree *ptr, RBTreeNode *k) {
	RBTreeImpl *impl = $Ref(&ptr->impl);

	while (k && k != impl->root && IS_RED(impl, k->parent)) {

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
					leftRotate(impl, k);
				}
				SET_BLACK(impl, k->parent);
				SET_RED(impl, k->parent->parent);
				rightRotate(impl, k->parent->parent);
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
					rightRotate(impl, k);
				}
				SET_BLACK(impl, k->parent);
				SET_RED(impl, k->parent->parent);
				leftRotate(impl, k->parent->parent);
			}
		}
	}
	SET_BLACK(impl, impl->root);
}

int rbtree_build(RBTree *ptr, const u64 key_size, const u64 value_size,
				 int (*compare)(const void *, const void *), bool send) {
	ChainGuard _ = ChainSend(send);

	// validate input
	if (ptr == NULL || key_size == 0 || value_size == 0 || compare == NULL) {
		errno = EINVAL;
		return -1;
	}

	// try to allocate required size for RBTreeImpl
	if (chain_malloc(&ptr->impl, sizeof(RBTreeImpl))) {
		// set the fatptr to null so it's not cleaned up in cleanup function
		ptr->impl = null;
		return -1;
	}

	// Obtain the reference to the fat pointers data using the $Ref macro
	RBTreeImpl *impl = $Ref(&ptr->impl);

	// initialize values of the RBTree structure
	impl->send = send;
	impl->key_size = key_size;
	impl->value_size = value_size;
	impl->compare = compare;
	impl->root = NIL;
	impl->size = 0;

	// return success
	return 0;
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

int rbtree_insert_node(RBTree *ptr, RBTreeNode *parent, bool is_right, const void *key,
					   const void *value) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	ChainGuard _ = ChainSend(impl->send);
	RBTreeNode *node;

	FatPtr self;
	u64 size = sizeof(RBTreeNode) + (impl->key_size + impl->value_size) * sizeof(char) + KEY_PAD +
			   VALUE_PAD(impl->key_size) + BOOLEAN_SIZE;
	if (chain_malloc(&self, size)) {
		return -1;
	}
	node = $Ref(&self);
	if (impl->root == NIL)
		impl->root = node;
	else if (is_right)
		parent->right = node;
	else
		parent->left = node;

	memcpy((char *)node->data + KEY_PAD, key, impl->key_size);
	memcpy(node->data + KEY_PAD + VALUE_PAD(impl->key_size) + impl->key_size, value,
		   impl->value_size);
	node->data[KEY_PAD + VALUE_PAD(impl->key_size) + impl->key_size + impl->value_size] = true;
	node->self = self;
	node->right = NIL;
	node->left = NIL;
	node->parent = parent;
	impl->size++;
	rbtree_fix_up(ptr, node);

	return 0;
}

int rbtree_insert(RBTree *ptr, const void *key, const void *value) {
	if (ptr == NULL || nil(ptr->impl)) {
		errno = EINVAL;
		return -1;
	}
	RBTreeImpl *impl = $Ref(&ptr->impl);
	RBTreeNode *parent = NONE;
	RBTreeNode *check = impl->root;
	bool is_right = false;

	loop {
		if (check == NIL) {
			if (rbtree_insert_node(ptr, parent, is_right, key, value))
				return -1;
			break;
		}
		parent = check;
		int v = impl->compare(check->data + KEY_PAD, key);
		if (v == 0) {
			return -1;
		} else if (v < 0) {
			is_right = true;
			check = check->right;
		} else {
			is_right = false;
			check = check->left;
		}
	}

	return 0;
}

void rbtree_transplant(RBTreeImpl *impl, RBTreeNode *dst, RBTreeNode *src) {
	if (dst->parent == NONE)
		impl->root = src;
	else if (dst == dst->parent->left)
		dst->parent->left = src;
	else
		dst->parent->right = src;
	if (src)
		src->parent = dst->parent;
}

void rbtree_delete_fixup(RBTree *ptr, RBTreeNode *x) {
	if (x != NULL)
		printf("delete fixup node %llu\n", fat_ptr_id(&x->self));
	RBTreeImpl *impl = $Ref(&ptr->impl);
	RBTreeNode *w;

	while (x != impl->root && IS_BLACK(impl, x)) {
		printf("loop\n");
		if (x == NULL || x->parent == NULL) {
			break;
		}
		if (x == x->parent->left) {
			w = x->parent->right;

			// If w is NULL, treat it as black and handle accordingly
			if (w == NULL) {
				x = x->parent; // Move up the tree
			} else {
				// type 1
				if (IS_RED(impl, w)) {
					SET_BLACK(impl, w);
					SET_RED(impl, x->parent);
					leftRotate(impl, x->parent);
					w = x->parent->right; // Update w after rotation
				}

				// type 2
				if (IS_BLACK(impl, w->left) && IS_BLACK(impl, w->right)) {
					SET_RED(impl, w);
					x = x->parent; // Move up the tree
				} else {
					// type 3
					if (IS_BLACK(impl, w->right)) {
						SET_BLACK(impl, w->left);
						SET_RED(impl, w);
						rightRotate(impl, w);
						w = x->parent->right; // Update w after rotation
					}
					// type 4
					if (IS_BLACK(impl, w)) {
						SET_BLACK(impl, x->parent);
					} else {
						SET_RED(impl, x->parent);
					}
					SET_BLACK(impl, w->right);
					leftRotate(impl, x->parent);
					x = impl->root; // Reset x to root
				}
			}
		} else {
			w = x->parent->left;

			// If w is NULL, treat it as black and handle accordingly
			if (w == NULL) {
				x = x->parent; // Move up the tree
			} else {
				// type 1
				if (IS_RED(impl, w)) {
					SET_BLACK(impl, w);
					SET_RED(impl, x->parent);
					rightRotate(impl, x);
					w = x->parent->left; // Update w after rotation
				}

				// type 2
				if (IS_BLACK(impl, w->right) && IS_BLACK(impl, w->left)) {
					SET_RED(impl, w);
					x = x->parent; // Move up the tree
				} else {
					// type 3
					if (IS_BLACK(impl, w->left)) {
						SET_BLACK(impl, w->right);
						SET_RED(impl, w);
						leftRotate(impl, w);
						w = x->parent->left; // Update w after rotation
					}
					// type 4
					if (IS_BLACK(impl, w)) {
						SET_BLACK(impl, x->parent);
					} else {
						SET_RED(impl, x->parent);
					}
					SET_BLACK(impl, w->left);
					rightRotate(impl, x->parent);
					x = impl->root; // Reset x to root
				}
			}
		}
	}

	printf("set x black\n");
	if (x)
		SET_BLACK(impl, x); // Finally, ensure x is black
}

/*
transplant(self, u, v)
: if u.p
== None : self.root = v elif u == u.p.left : u.p.left = v else : u.p.right = v v.p =
u.p
*/

int rbtree_delete(RBTree *ptr, const void *key) {
	if (ptr == NULL || nil(ptr->impl)) {
		errno = EINVAL;
		return -1;
	}

	RBTreeImpl *impl = $Ref(&ptr->impl);
	u64 key_size = impl->key_size;
	u64 value_size = impl->value_size;
	ChainGuard _ = ChainSend(impl->send);

	RBTreeNode *check = impl->root;
	RBTreeNode *nodeToDelete = NULL; // Node to be deleted
	RBTreeNode *child = NULL;		 // Child to replace the node
	RBTreeNode *successor = NULL;
	RBTreeNode *x = NULL;
	bool wasBlack = false; // To track if the deleted node is black

	// Find the node to delete
	while (check != NULL) {
		int v = impl->compare(check->data + KEY_PAD, key);
		if (v == 0) {
			nodeToDelete = check; // Found the node to delete
			break;
		} else if (v < 0) {
			check = check->right;
		} else {
			check = check->left;
		}
	}

	if (nodeToDelete == NULL) {
		return -1; // Node not found
	}

	// Store the color of the node to delete
	wasBlack = IS_BLACK(impl, nodeToDelete);

	if (nodeToDelete->left == NULL) {
		printf("1\n");
		x = nodeToDelete->left;
		rbtree_transplant(impl, nodeToDelete, nodeToDelete->right);
	} else if (nodeToDelete->right == NULL) {
		printf("2\n");
		x = nodeToDelete->right;
		rbtree_transplant(impl, nodeToDelete, nodeToDelete->left);
	} else {
		printf("3\n");
		// Find the in-order successor (smallest node in the right subtree)
		RBTreeNode *successor = nodeToDelete->right;
		while (successor->left != NULL) {
			successor = successor->left;
		}
		x = successor->right;
		rbtree_transplant(impl, nodeToDelete, successor);
	}

	if (wasBlack)
		rbtree_delete_fixup(ptr, x); // Call fix-up function

	// Free the node which has been transplanted
	chain_free(&nodeToDelete->self);

	impl->size--;

	// Successful deletion
	return 0;
}

const void *rbtree_get(const RBTree *ptr, const void *key) {
	if (ptr == NULL || nil(ptr->impl)) {
		errno = EINVAL;
		return NULL;
	}
	RBTreeImpl *impl = $Ref(&ptr->impl);

	RBTreeNode *check = impl->root;
	loop {
		if (check == NIL)
			return NULL;
		int v = impl->compare(check->data + KEY_PAD, key);
		if (v == 0) {
			return check->data + KEY_PAD + impl->key_size + VALUE_PAD(impl->key_size);
		} else if (v < 0) {
			check = check->right;
		} else {
			check = check->left;
		}
	}

	return NULL;
}

i64 rbtree_size(const RBTree *ptr) {
	if (ptr == NULL || nil(ptr->impl)) {
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

bool rbtree_validate_node(const RBTree *ptr, const RBTreeNode *node, int *black_count,
						  int current_black_count) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	u64 key_size = impl->key_size;
	u64 value_size = impl->value_size;

	// Base case: when we reach a NIL node's pointer to actual NULL
	if (node == NULL) {
		// If this is the first NIL node reached, set the black count
		if (*black_count == 0) {
			*black_count = current_black_count; // Set the black count for the first path
		} else {
			// Check for black count consistency
			if (current_black_count != *black_count) {
				printf("NIL node reached: current_black_count = %d, expected_black_count = %d\n",
					   current_black_count, *black_count);
				return false; // If counts do not match, return false
			}
		}
		return true; // Return true for NIL nodes
	}

	// Increment black count if the current node is black
	if (IS_BLACK(impl, node)) {
#ifdef TEST
		if (node == NIL)
			;
		//			printf("======================NIL\n");
		else
//			printf("======================black node %llu\n", fat_ptr_id(&node->self));
#endif // TEST
			current_black_count++;
	} else {
#ifdef TEST
//		printf("======================red node %llu\n", fat_ptr_id(&node->self));
#endif // TEST
	   //  Check if the node is red
	   //  If the parent is red, return false (Red property violation)
		if (node->parent != NONE && IS_RED(impl, node->parent)) {
			printf("Red property violation at node with key\n");
			return false;
		}
	}

	// Recursive calls for left and right children
	// printf("go left\n");
	bool left_valid = rbtree_validate_node(ptr, node->left, black_count, current_black_count);
	// printf("go right\n");
	bool right_valid = rbtree_validate_node(ptr, node->right, black_count, current_black_count);

	return left_valid && right_valid;
}

bool rbtree_validate(const RBTree *ptr) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	int black_count = 0;
	// Validate from the root and check if the root is black
	if (impl->root != NULL) {
		if (IS_BLACK(impl, impl->root)) {
			bool ret = rbtree_validate_node(ptr, impl->root, &black_count, 0);
			return ret;
		}
#ifdef TEST
//		printf("======================red node %llu\n", fat_ptr_id(&impl->root->self));
#endif // TEST
		printf("root is not black\n");
		return false; // Root should be black
	}
	return true;
}
