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
#define RED(node, key_size, value_size) *(bool *)(node->data + RED_OFFSET(key_size, value_size))

typedef struct RBTreeNode {
	FatPtr self;
	struct RBTreeNode *right;
	struct RBTreeNode *left;
	struct RBTreeNode *parent;
	// bool red;
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

void rbtree_iterator_cleanup(RBTreeIteratorNc *ptr) {
	if (ptr != NULL && !nil(ptr->impl)) {
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
	while (impl->cur || impl->stack_pointer > 0) {
		// Traverse left subtree
		if (impl->cur) {
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
void leftRotate(RBTree *ptr, RBTreeNode *x) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	RBTreeNode *y = x->right;
	x->right = y->left;
	if (y->left != NULL) {
		y->left->parent = x;
	}
	y->parent = x->parent;
	if (x->parent == NULL) {
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
void rightRotate(RBTree *ptr, RBTreeNode *x) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	RBTreeNode *y = x->left;
	x->left = y->right;
	if (y->right != NULL) {
		y->right->parent = x;
	}
	y->parent = x->parent;
	if (x->parent == NULL) {
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
	u64 key_size = impl->key_size;
	u64 value_size = impl->value_size;
	RBTreeNode *root = impl->root;

	// Using the RED macro to access the color of the root
	printf("root->red=%i\n", RED(root, key_size, value_size));

	while (k && k != root && k->parent && RED(k->parent, key_size, value_size)) {
		if (k->parent == k->parent->parent->left) {
			RBTreeNode *u = k->parent->parent->right; // uncle
			if (u && RED(u, key_size, value_size)) {
				// Change the colors of the parent and uncle to black
				RED(k->parent, key_size, value_size) = false;		 // Using the macro here
				RED(u, key_size, value_size) = false;				 // Using the macro here
				RED(k->parent->parent, key_size, value_size) = true; // Using the macro here
				k = k->parent->parent;
			} else {
				if (k == k->parent->right) {
					k = k->parent;
					leftRotate(ptr, k);
				}
				// Change the colors for the rotations
				RED(k->parent, key_size, value_size) = false;		 // Using the macro here
				RED(k->parent->parent, key_size, value_size) = true; // Using the macro here
				rightRotate(ptr, k->parent->parent);
			}
		} else {
			RBTreeNode *u = k->parent->parent->left; // uncle
			if (u && RED(u, key_size, value_size)) {
				// Change the colors of the parent and uncle to black
				RED(k->parent, key_size, value_size) = false;		 // Using the macro here
				RED(u, key_size, value_size) = false;				 // Using the macro here
				RED(k->parent->parent, key_size, value_size) = true; // Using the macro here
				k = k->parent->parent;
			} else {
				if (k == k->parent->left) {
					k = k->parent;
					rightRotate(ptr, k);
				}
				// Change the colors for the rotations
				RED(k->parent, key_size, value_size) = false;		 // Using the macro here
				RED(k->parent->parent, key_size, value_size) = true; // Using the macro here
				leftRotate(ptr, k->parent->parent);
			}
		}
	}
	// Set the root color to black
	RED(root, key_size, value_size) = false; // Using the macro here
}

void rbtree_delete_fixup(RBTree *ptr, RBTreeNode *x) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	u64 key_size = impl->key_size;
	u64 value_size = impl->value_size;
	RBTreeNode *root = impl->root;

	while (x != root && !RED(x, key_size, value_size)) {
		if (x == x->parent->left) {
			RBTreeNode *w = x->parent->right; // sibling

			if (RED(w, key_size, value_size)) {
				RED(w, key_size, value_size) = false;		 // Case 1: Sibling is red
				RED(x->parent, key_size, value_size) = true; // Parent becomes red
				leftRotate(ptr, x->parent);					 // Rotate around parent
				w = x->parent->right;						 // Update w to the new sibling
			}

			if ((!w->left || !RED(w->left, key_size, value_size)) &&
				(!w->right || !RED(w->right, key_size, value_size))) {
				RED(w, key_size, value_size) = true; // Case 2: Both children are black
				x = x->parent;						 // Move up the tree
			} else {
				if (!w->right || !RED(w->right, key_size, value_size)) {
					// Case 3: Left child is red, right child is black
					if (w->left)
						RED(w->left, key_size, value_size) = false; // Set left child to black
					RED(w, key_size, value_size) = true;			// Set sibling to red
					rightRotate(ptr, w);							// Rotate around w
					w = x->parent->right;							// Update w
				}

				// Case 4: Right child is red
				RED(w, key_size, value_size) =
					RED(x->parent, key_size, value_size);	  // Copy parent's color
				RED(x->parent, key_size, value_size) = false; // Parent becomes black
				if (w->right)
					RED(w->right, key_size, value_size) = false; // Make the right child black
				leftRotate(ptr, x->parent);						 // Rotate around parent
				x = root;										 // Break out of the loop
			}
		} else {
			RBTreeNode *w = x->parent->left; // sibling

			if (RED(w, key_size, value_size)) {
				RED(w, key_size, value_size) = false;		 // Case 1: Sibling is red
				RED(x->parent, key_size, value_size) = true; // Parent becomes red
				rightRotate(ptr, x->parent);				 // Rotate around parent
				w = x->parent->left;						 // Update w to the new sibling
			}

			if ((!w->right || !RED(w->right, key_size, value_size)) &&
				(!w->left || !RED(w->left, key_size, value_size))) {
				RED(w, key_size, value_size) = true; // Case 2: Both children are black
				x = x->parent;						 // Move up the tree
			} else {
				if (!w->left || !RED(w->left, key_size, value_size)) {
					// Case 3: Right child is red, left child is black
					if (w->right)
						RED(w->right, key_size, value_size) = false; // Set right child to black
					RED(w, key_size, value_size) = true;			 // Set sibling to red
					leftRotate(ptr, w);								 // Rotate around w
					w = x->parent->left;							 // Update w
				}

				// Case 4: Left child is red
				RED(w, key_size, value_size) =
					RED(x->parent, key_size, value_size);	  // Copy parent's color
				RED(x->parent, key_size, value_size) = false; // Parent becomes black
				if (w->left)
					RED(w->left, key_size, value_size) = false; // Make the left child black
				rightRotate(ptr, x->parent);					// Rotate around parent
				x = root;										// Break out of the loop
			}
		}
	}
	RED(x, key_size, value_size) = false; // Ensure the root is black
}

// Rbtree build function has the 'send' parameter. This parameter indicates whether or
// not the Rbtree can be send into another thread. If it can be sent into another
// thread, the globaly sync allocator is used for all functions.
int rbtree_build(RBTree *ptr, const u64 key_size, const u64 value_size,
				 int (*compare)(const void *, const void *), bool send) {
	// Obtain the chainguard based on the boolean 'send'. If this is a send, the
	// ChainGuard macro returns the global sync allocator's send guard and all memory
	// allocations will use the thread-safe global_sync allocator. Otherwise, the thread
	// local version will be used.
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
	impl->root = NULL;
	impl->size = 0;

	// return success
	return 0;
}

void rbtree_free_node(RBTreeNode *ptr) {
	if (ptr != NULL) {
		rbtree_free_node(ptr->right);
		rbtree_free_node(ptr->left);
		chain_free(&ptr->self);
	}
}

// cleanup function selects appropriate ChainGuard based on configuration and
// deallocates memory
void rbtree_cleanup(RBTreeNc *ptr) {
	// check non-initialized conditions
	if (ptr != NULL && !nil(ptr->impl)) {
		// obtain referent to internal RBTree structure
		RBTreeImpl *impl = $Ref(&ptr->impl);
		// set appropriate chainguard status
		ChainGuard _ = ChainSend(impl->send);

		rbtree_free_node(impl->root);

		// free RBTreeImpl structure
		chain_free(&ptr->impl);
	}
}

int rbtree_insert(RBTree *ptr, const void *key, const void *value) {
	if (ptr == NULL || nil(ptr->impl)) {
		errno = EINVAL;
		return -1;
	}
	RBTreeImpl *impl = $Ref(&ptr->impl);
	ChainGuard _ = ChainSend(impl->send);
	RBTreeNode *parent = NULL;
	RBTreeNode *check = impl->root;
	bool is_right = false;

	loop {
		RBTreeNode *node = NULL;
		if (check == NULL) {
			FatPtr self;
			u64 size = sizeof(RBTreeNode) + (impl->key_size + impl->value_size) * sizeof(char) +
					   KEY_PAD + VALUE_PAD(impl->key_size) + BOOLEAN_SIZE;
			if (chain_malloc(&self, size)) {
				return -1;
			}
			node = $Ref(&self);
			if (impl->root == NULL)
				impl->root = node;
			else if (is_right)
				parent->right = node;
			else
				parent->left = node;

			memcpy((char *)node->data + KEY_PAD, key, impl->key_size);
			memcpy(node->data + KEY_PAD + VALUE_PAD(impl->key_size) + impl->key_size, value,
				   impl->value_size);
			node->data[KEY_PAD + VALUE_PAD(impl->key_size) + impl->key_size + impl->value_size] =
				true;
			node->self = self;
			node->right = NULL;
			node->left = NULL;
			node->parent = parent;
			impl->size++;
			rbtree_fix_up(ptr, node);
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
	bool wasBlack = false;			 // To track if the deleted node is black

	// Find the node to delete
	while (check != NULL) {
		int v = impl->compare(check->data + KEY_PAD, key);
		if (v == 0) {
			nodeToDelete = check; // Found the node to delete
			break;
		} else if (v < 0) {
			check = check->right; // Go left
		} else {
			check = check->left; // Go right
		}
	}

	if (nodeToDelete == NULL) {
		return -1; // Node not found
	}

	// Store the color of the node to delete
	wasBlack = !RED(nodeToDelete, key_size, value_size);

	// Case 1: Node has two children
	if (nodeToDelete->left != NULL && nodeToDelete->right != NULL) {
		// Find the in-order successor (smallest node in the right subtree)
		RBTreeNode *successor = nodeToDelete->right;
		while (successor->left != NULL) {
			successor = successor->left;
		}

		// Copy the successor's data to the node to delete
		memcpy(nodeToDelete->data + KEY_PAD, successor->data + KEY_PAD,
			   (impl->key_size + impl->value_size + VALUE_PAD(impl->key_size)) + BOOLEAN_SIZE);
		nodeToDelete->self = successor->self;

		// Now we need to delete the successor
		nodeToDelete = successor; // Update to the successor nodeAA
	}

	// Case 2: Node has at most one child
	child = (nodeToDelete->left != NULL) ? nodeToDelete->left : nodeToDelete->right;

	// Remove nodeToDelete from the tree
	if (child != NULL) {
		child->parent = nodeToDelete->parent; // Update child’s parent
	}

	if (nodeToDelete->parent == NULL) {
		// Node is root
		impl->root = child; // Update the root if necessary
	} else {
		// Update the parent's child pointer
		if (nodeToDelete == nodeToDelete->parent->left) {
			nodeToDelete->parent->left = child; // Update left child
		} else {
			nodeToDelete->parent->right = child; // Update right child
		}
	}

	// Free the node
	chain_free(&nodeToDelete->self);

	// If the deleted node was black, we need to fix up the tree
	if (wasBlack) {
		rbtree_delete_fixup(ptr, child); // Call fix-up function
	}
	impl->size--;
	return 0; // Successful deletion
}

const void *rbtree_get(const RBTree *ptr, const void *key) {
	if (ptr == NULL || nil(ptr->impl)) {
		errno = EINVAL;
		return NULL;
	}
	RBTreeImpl *impl = $Ref(&ptr->impl);

	RBTreeNode *check = impl->root;
	loop {
		if (check == NULL)
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