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
#include <base/resources.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <util/rbtree.h>

typedef struct RBTreeNode {
	FatPtr right;
	FatPtr left;
	FatPtr parent;
	bool red;
	char data[];
} RBTreeNode;

typedef struct RBTreeImpl {
	u64 key_size;
	u64 value_size;
	int (*compare)(const void *, const void *);
	FatPtr root;
	bool send;
} RBTreeImpl;

void rbtree_iterator_cleanup(RBTreeIteratorNc *ptr) {
}

void *rbtree_iterator_next(RBTreeIterator *ptr) {
	return NULL;
}

FatPtr create_fatptr_from_node(RBTreeNode *node) {
	FatPtr fp;
	fp.data = node; // set node
	return fp;
}

void rbtree_rotate_left(RBTreeImpl *impl, FatPtr *node) {
	RBTreeNode *target = $Ref(node);							   // The node being rotated
	RBTreeNode *right_child = $Ref(&target->right);				   // The right child of the target
	RBTreeNode *right_child_left_child = $Ref(&right_child->left); // Left child of right_child

	// Move the left child of right_child to be the new right child of the target
	target->right = right_child->left;

	// If the right child's left child is not null, update its parent to point to the target
	if (!nil(right_child->left)) {
		right_child_left_child->parent = *node;
	}

	// Update the parent of the right child to be the parent of the target
	right_child->parent = target->parent;

	// If the target was the root of the tree, update the root to point to right_child
	if (nil(target->parent)) {
		impl->root = create_fatptr_from_node(right_child); // Convert to FatPtr
	} else {
		// Otherwise, update the parent's child pointer to point to right_child
		RBTreeNode *parent = $Ref(&target->parent);
		if (node == &parent->left) {
			parent->left = create_fatptr_from_node(right_child); // Convert to FatPtr
		} else {
			parent->right = create_fatptr_from_node(right_child); // Convert to FatPtr
		}
	}

	// Set target as the left child of right_child
	right_child->left = *node;

	// Update the parent of target to point to right_child
	target->parent = create_fatptr_from_node(right_child); // Convert to FatPtr
}

void rbtree_rotate_right(RBTreeImpl *impl, FatPtr *node) {
	RBTreeNode *target = $Ref(node);							   // The node being rotated
	RBTreeNode *left_child = $Ref(&target->left);				   // The left child of the target
	RBTreeNode *left_child_right_child = $Ref(&left_child->right); // Right child of left_child

	// Move the right child of left_child to be the new left child of the target
	target->left = left_child->right;

	// If the left child's right child is not null, update its parent to point to the target
	if (!nil(left_child->right)) {
		left_child_right_child->parent = *node;
	}

	// Update the parent of the left child to be the parent of the target
	left_child->parent = target->parent;

	// If the target was the root of the tree, update the root to point to left_child
	if (nil(target->parent)) {
		impl->root = create_fatptr_from_node(left_child); // Convert to FatPtr
	} else {
		// Otherwise, update the parent's child pointer to point to left_child
		RBTreeNode *parent = $Ref(&target->parent);
		if (node == &parent->left) {
			parent->left = create_fatptr_from_node(left_child); // Convert to FatPtr
		} else {
			parent->right = create_fatptr_from_node(left_child); // Convert to FatPtr
		}
	}

	// Set target as the right child of left_child
	left_child->right = *node;

	// Update the parent of target to point to left_child
	target->parent = create_fatptr_from_node(left_child); // Convert to FatPtr
}

void rbtree_fix_up(RBTreeImpl *impl, FatPtr *node) {
	printf("1\n");
	while (true) {
		RBTreeNode *target = $Ref(node);

		// If the node is the root, set it to black and return
		if (nil(target->parent)) {
			target->red = false; // Root must be black
			return;
		}

		RBTreeNode *parent = $Ref(&target->parent);

		// If the parent is black, no fix is needed
		if (!parent->red) {
			return;
		}

		// Get the grandparent
		RBTreeNode *grandparent = $Ref(&parent->parent);
		FatPtr uncle;

		// Determine if the parent is the left or right child of the grandparent
		if (parent == $Ref(&grandparent->left)) {
			// Parent is the left child, so the uncle is the right child
			uncle = grandparent->right; // Right child of grandparent
		} else {
			// Parent is the right child, so the uncle is the left child
			uncle = grandparent->left; // Left child of grandparent
		}
		RBTreeNode *uncle_node = $Ref(&uncle);

		// Case 1: Parent and Uncle are both red (Recoloring)
		if (!nil(uncle) && uncle_node->red) {
			parent->red = false;	 // Recolor parent to black
			uncle_node->red = false; // Recolor uncle to black
			grandparent->red = true; // Recolor grandparent to red

			// Move up to the grandparent and repeat
			node = &parent->parent;
			continue;
		}

		// Case 2: Parent is red and Uncle is black or nil (Rotations)
		if (parent == $Ref(&grandparent->left)) {
			if (target == $Ref(&parent->right)) {
				// Case 2a: Target is a right child -> Rotate left on parent
				rbtree_rotate_left(impl, &target->parent);
				target = $Ref(&target->left); // Update target after rotation
			}
			// Case 2b: Rotate right on grandparent and recolor
			rbtree_rotate_right(impl, &parent->parent);
			parent->red = false;	 // Recolor parent to black
			grandparent->red = true; // Recolor grandparent to red
		} else {
			if (target == $Ref(&parent->left)) {
				// Case 2a (mirror): Target is a left child -> Rotate right on parent
				rbtree_rotate_right(impl, &target->parent);
				target = $Ref(&target->right); // Update target after rotation
			}
			// Case 2b (mirror): Rotate left on grandparent and recolor
			rbtree_rotate_left(impl, &parent->parent);
			parent->red = false;	 // Recolor parent to black
			grandparent->red = true; // Recolor grandparent to red
		}
		break; // Fixup complete
	}

	// Ensure the root is always black
	RBTreeNode *root = $Ref(&impl->root);
	root->red = false;
}

/*
Compiles:
void rbtree_rotate_left(RBTreeImpl *impl, FatPtr *node) {
	RBTreeNode *target = $Ref(node);
	RBTreeNode *right_child = $Ref(&target->right);
	RBTreeNode *right_child_left_child = $Ref(&right_child->left);

	target->right = right_child->left;

	if (!nil(right_child->left)) {
		// If the right child has a left child, update its parent to point to the target
		right_child_left_child->parent =
			*node; // Set parent of the left child of the right child to the target
	}

	RBTreeNode *parent = $Ref(&target->parent);
	if (!nil(target->parent)) {
		if (node == &parent->left) {
			parent->left = target->right; // Set right child as the new left child of the parent
		} else {
			parent->right = target->right;
		}
	}

	right_child->parent = target->parent;

	if (nil(target->parent)) {
		impl->root = *node; // Update the root of the tree
	} else {
		// If the target node was not the root, update the parent's child pointer
		if (node == &parent->left) {
			parent->left = right_child->left; // Set the new root as the parent's left child
		} else {
			parent->right = right_child->left; // Set the new root as the parent's right child
		}
	}

	right_child->left = *node;
	target->parent = target->right;
}
*/

/*
	RBTreeNode *x = $Ref(node); // Current node

	// Dereference y to access its fields
	RBTreeNode *tmp_y = $Ref(y); // Temporary variable to hold y's node

	// Perform the rotation
	x->right = tmp_y->left; // Turn y's left subtree into x's right subtree
	if (!nil(tmp_y->left)) {
		// Correctly access the parent of tmp_y->left
		RBTreeNode *left_child = $Ref(&tmp_y->left);
		left_child->parent = *node; // If y's left child is not null, set its parent to x
	}

	tmp_y->parent = x->parent; // Link x's parent as y's parent
	if (nil(x->parent)) {
		// x is root
		impl->root = *y; // y becomes the root
	} else if (node == $Ref(&x->parent)->left) {
		$Ref(x->parent)->left = *y; // Link y as left child of x's parent
	} else {
		$Ref(x->parent)->right = *y; // Link y as right child of x's parent
	}

	tmp_y->left = *node; // Put x on y's left
	x->parent = y;		 // Link y as parent of x
}
*/

/*
// Rotate the subtree right.
void rbtree_rotate_right(RBTreeImpl *impl, FatPtr *node) {
	RBTreeNode *y = $Ref(node);
	FatPtr *x = &y->left;

	// Perform rotation.
	y->left = x->right;
	if (!nil(x->right)) {
		x->right->parent = node;
	}
	x->parent = y->parent;

	if (nil(y->parent)) {
		impl->root = x; // x becomes the root.
	} else if (node == y->parent->right) {
		y->parent->right = x;
	} else {
		y->parent->left = x;
	}

	x->right = node;
	y->parent = x;
}
*/

int rbtree_rebalance(RBTree *ptr) {
	return 0;
}

// Rbtree build function has the 'send' parameter. This parameter indicates whether or not
// the Rbtree can be send into another thread. If it can be sent into another thread,
// the globaly sync allocator is used for all functions.
int rbtree_build(RBTree *ptr, const u64 key_size, const u64 value_size,
				 int (*compare)(const void *, const void *), bool send) {
	// Obtain the chainguard based on the boolean 'send'. If this is a send, the ChainGuard
	// macro returns the global sync allocator's send guard and all memory allocations will
	// use the thread-safe global_sync allocator. Otherwise, the thread local version will
	// be used.
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
	impl->root = null;

	// return success
	return 0;
}

void rbtree_free_node(FatPtr *ptr) {
	if (ptr != NULL && !nil(*ptr)) {
		RBTreeNode *node = $Ref(ptr);
		if (!nil(node->right))
			rbtree_free_node(&node->right);
		if (!nil(node->left))
			rbtree_free_node(&node->left);
		chain_free(ptr);
	}
}

// cleanup function selects appropriate ChainGuard based on configuration and deallocates memory
void rbtree_cleanup(RBTreeNc *ptr) {
	// check non-initialized conditions
	if (ptr != NULL && !nil(ptr->impl)) {
		// obtain referent to internal RBTree structure
		RBTreeImpl *impl = $Ref(&ptr->impl);
		// set appropriate chainguard status
		ChainGuard _ = ChainSend(impl->send);

		rbtree_free_node(&impl->root);

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

	FatPtr parent = null;
	FatPtr *check = &impl->root;

	loop {

		if (nil(*check)) {
			u64 size = sizeof(RBTreeNode) + (impl->key_size + impl->value_size) * sizeof(char);
			if (chain_malloc(check, size)) {
				return -1;
			}
			RBTreeNode *node = $Ref(check);
			memcpy((char *)node->data, key, impl->key_size);
			memcpy(node->data + impl->key_size, value, impl->value_size);
			node->red = true;
			node->right = null;
			node->left = null;
			node->parent = parent;
			break;
		}
		parent = *check;
		RBTreeNode *cur = $Ref(check);
		int v = impl->compare(cur->data, key);
		if (v == 0) {
			return -1;
		} else if (v > 0) {
			check = &cur->right;
		} else {
			check = &cur->left;
		}
	}

	printf("calling fix up\n");
	// rbtree_fix_up(impl, check);
	return 0;
}

int rbtree_delete(RBTree *ptr, const void *key) {
	return 0;
}

const void *rbtree_get(const RBTree *ptr, const void *key) {
	if (ptr == NULL || nil(ptr->impl)) {
		errno = EINVAL;
		return NULL;
	}
	RBTreeImpl *impl = $Ref(&ptr->impl);
	ChainGuard _ = ChainSend(impl->send);

	FatPtr *check = &impl->root;

	loop {

		if (nil(*check)) {
			return NULL;
		}
		RBTreeNode *cur = $Ref(check);
		int v = impl->compare(cur->data, key);
		if (v == 0) {
			return cur->data + impl->key_size;
		} else if (v > 0) {
			check = &cur->right;
		} else {
			check = &cur->left;
		}
	}

	return NULL;
}

int rbtree_size(const RBTree *ptr) {
	return 0;
}

int rbtree_iterator(const RBTree *ptr, RBTreeIterator *iter) {
	return 0;
}
