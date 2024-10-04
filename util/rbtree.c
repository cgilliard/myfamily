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

/*
// Utility function to perform left rotation
	void leftRotate(Node* x)
	{
		Node* y = x->right;
		x->right = y->left;
		if (y->left != NIL) {
			y->left->parent = x;
		}
		y->parent = x->parent;
		if (x->parent == nullptr) {
			root = y;
		}
		else if (x == x->parent->left) {
			x->parent->left = y;
		}
		else {
			x->parent->right = y;
		}
		y->left = x;
		x->parent = y;
	}
*/

/*
void rbtree_rotate_left(RBTree *ptr, FatPtr *node) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	printf("rotate left\n");
	RBTreeNode *target = $Ref(node);
	FatPtr y = target->right;
	RBTreeNode *ynode = $Ref(&y);
	target->right = ynode->left;
	FatPtr yleft = null;
	if (ynode && !nil(ynode->left)) {
		RBTreeNode *ynode_left = $Ref(&ynode->left);
		ynode_left->parent = *node;
	}
	ynode->parent = target->parent;

	if (nil(target->parent)) {
		printf("!!!!!!parent target null\n");
		impl->root = y;
	} else {
		RBTreeNode *parent = $Ref(&target->parent);
		RBTreeNode *parent_left = $Ref(&parent->left);
		if (target == parent_left) {
			printf("==parent_left\n");
			parent->left = y;
		} else {
			printf("==parent_right\n");
			parent->right = y;
		}
	}
	ynode->left = *node;
	target->parent = y;
	printf("complete!!!!!!\n");
}
*/

void rbtree_rotate_left(RBTree *ptr, FatPtr *node) {
	return;
	RBTreeImpl *impl = $Ref(&ptr->impl);
	printf("rotate left\n");

	// Get the target node and its right child (y)
	RBTreeNode *target = $Ref(node); // target is of type RBTreeNode*
	FatPtr y = target->right;		 // y is of type FatPtr
	RBTreeNode *ynode = $Ref(&y);	 // ynode is of type RBTreeNode*

	// Move y's left subtree into target's right subtree
	target->right = ynode->left;
	if (!nil(ynode->left)) {
		RBTreeNode *yleft = $Ref(&ynode->left);
		yleft->parent = *node; // Correctly set the parent of left child to target.
	}

	assert(target != ynode && "Target and Ynode cannot be the same!");
	assert(&target->right != node && "Target's right child cannot be itself!");

	// Temporarily store parent and sibling
	FatPtr tempParent = target->parent; // Store target's parent temporarily
	FatPtr tempLeft = ynode->left;		// Store y's left child temporarily

	// Set y's parent to target's parent
	ynode->parent = tempParent;

	// Update the root if target was the root
	if (nil(tempParent)) {
		printf("!!!!!!parent target null\n");
		impl->root = y; // `y` becomes the new root.
	} else {
		// Update the correct child reference of target's parent
		FatPtr *parentPtr = &tempParent; // Use a pointer to FatPtr
		RBTreeNode *parent = $Ref(parentPtr);
		if (target == $Ref(&parent->left)) {
			printf("==parent_left\n");
			parent->left = y;
		} else {
			printf("==parent_right\n");
			parent->right = y;
		}
	}

	// Make target the left child of y
	ynode->left = *node; // Set the left of y to target

	// Update the parent of target to be y
	target->parent = y; // ynode; // Correctly set target's parent to y

	// Update assertions to check pointers directly
	// assert((FatPtr)ynode->parent != (FatPtr)target && "Ynode's parent cannot be Target!");
	// assert(tempLeft != (FatPtr)node && "Ynode's left child cannot be Node!");

	printf("complete!!!!!!\n");
}

/*
void rbtree_rotate_left(RBTree *ptr, FatPtr *node) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	printf("rotate left\n");

	// Get the target node and its right child (y)
	RBTreeNode *target = $Ref(node);
	FatPtr y = target->right;
	RBTreeNode *ynode = $Ref(&y);

	// Move y's left subtree into target's right subtree
	target->right = ynode->left;
	if (!nil(ynode->left)) {
		RBTreeNode *yleft = $Ref(&ynode->left);
		yleft->parent = *node;
	}

	assert(target != ynode && "Target and Ynode cannot be the same!");
	assert(&target->right != node && "Target's right child cannot be itself!");

	// Set y's parent to target's parent
	ynode->parent = target->parent;

	// Update the root if target was the root
	if (nil(target->parent)) {
		printf("!!!!!!parent target null\n");
		impl->root = y;
	} else {
		// Update the correct child reference of target's parent
		RBTreeNode *parent = $Ref(&target->parent);
		if (target == $Ref(&parent->left)) {
			printf("==parent_left\n");
			parent->left = y;
		} else {
			printf("==parent_right\n");
			parent->right = y;
		}
	}

	// Make target the left child of y
	ynode->left = *node;
	target->parent = y;

	assert(&ynode->parent != node && "Ynode's parent cannot be Target!");

	printf("complete!!!!!!\n");
}
*/

/*
void rbtree_rotate_right(RBTree *ptr, FatPtr *node) {
	printf("rotate right\n");
}
*/

void rbtree_rotate_right(RBTree *ptr, FatPtr *node) {
	return;
	RBTreeImpl *impl = $Ref(&ptr->impl);
	printf("rotate right\n");

	// Get the target node and its left child (y)
	RBTreeNode *target = $Ref(node);
	FatPtr y = target->left;
	RBTreeNode *ynode = $Ref(&y);

	// Move y's right subtree into target's left subtree
	target->left = ynode->right;
	if (!nil(ynode->right)) {
		RBTreeNode *yright = $Ref(&ynode->right);
		yright->parent = *node;
	}

	// Set y's parent to target's parent
	ynode->parent = target->parent;

	// Update the root if target was the root
	if (nil(target->parent)) {
		impl->root = y;
	} else {
		// Update the correct child reference of target's parent
		RBTreeNode *parent = $Ref(&target->parent);
		if (target == $Ref(&parent->left)) {
			parent->left = y;
		} else {
			parent->right = y;
		}
	}

	// Make target the right child of y
	ynode->right = *node;
	target->parent = y;

	printf("complete right rotation\n");
}

/*
void fixInsert(Node* k)
	{
		while (k != root && k->parent->color == "RED") {
			if (k->parent == k->parent->parent->left) {
				Node* u = k->parent->parent->right; // uncle
				if (u->color == "RED") {
					k->parent->color = "BLACK";
					u->color = "BLACK";
					k->parent->parent->color = "RED";
					k = k->parent->parent;
				}
				else {
					if (k == k->parent->right) {
						k = k->parent;
						leftRotate(k);
					}
					k->parent->color = "BLACK";
					k->parent->parent->color = "RED";
					rightRotate(k->parent->parent);
				}
			}
			else {
				Node* u = k->parent->parent->left; // uncle
				if (u->color == "RED") {
					k->parent->color = "BLACK";
					u->color = "BLACK";
					k->parent->parent->color = "RED";
					k = k->parent->parent;
				}
				else {
					if (k == k->parent->left) {
						k = k->parent;
						rightRotate(k);
					}
					k->parent->color = "BLACK";
					k->parent->parent->color = "RED";
					leftRotate(k->parent->parent);
				}
			}
		}
		root->color = "BLACK";
	}
*/

void rbtree_fix_up(RBTree *ptr, FatPtr *fptr) {
	RBTreeImpl *impl = $Ref(&ptr->impl);
	RBTreeNode *root = $Ref(&impl->root);
	loop {
		RBTreeNode *k = $Ref(fptr);
		if (k == root)
			break;
		RBTreeNode *parent = $Ref(&k->parent);
		if (!parent->red)
			break;
		RBTreeNode *gp = $Ref(&parent->parent);
		if (gp == NULL)
			break;

		if (!nil(gp->left) && $Ref(&gp->left) == parent) {
			printf("===============left uncle\n");
			FatPtr ufat = gp->right; // uncle
			RBTreeNode *u = $Ref(&ufat);
			if (u != NULL && u->red) {
				parent->red = false;
				u->red = false;
				gp->red = true;
				fptr = &parent->parent;
			} else {
				printf("else le\n");
				if (k == $Ref(&parent->right)) {
					fptr = &k->parent;
					rbtree_rotate_left(ptr, fptr);
				}
				parent->red = false;
				gp->red = true;
				rbtree_rotate_right(ptr, &parent->parent);
			}
		} else {
			printf("===============right uncle\n");
			FatPtr ufat = gp->left; // uncle
			RBTreeNode *u = $Ref(&ufat);
			if (u != NULL && u->red) {
				parent->red = false;
				u->red = false;
				gp->red = true;
				fptr = &parent->parent;
			} else {
				if (k == $Ref(&parent->left)) {
					fptr = &k->parent;
					rbtree_rotate_right(ptr, fptr);
				}
				parent->red = false;
				gp->red = true;
				rbtree_rotate_left(ptr, &parent->parent);
			}
		}

		printf("loop continues\n");
	}
	root->red = false;
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
		printf("free %p %p\n", ptr, ptr->data);
		chain_free(ptr);
	}
}

// cleanup function selects appropriate ChainGuard based on configuration and
// deallocates memory
void rbtree_cleanup(RBTreeNc *ptr) {
	printf("rbtree cleanup\n");
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

	printf("insert\n");
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
			printf("go right\n");
			check = &cur->right;
		} else {
			printf("go left\n");
			check = &cur->left;
		}
	}

	printf("calling fix up\n");
	rbtree_fix_up(ptr, check);
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

	int counter = 0;
	loop {
		if (nil(*check)) {
			return NULL;
		}
		RBTreeNode *cur = $Ref(check);
		int v = impl->compare(cur->data, key);
		if (v == 0) {
			printf("found at %p\n", check->data);
			return cur->data + impl->key_size;
		} else if (v > 0) {
			printf("check=%p,cur->right=%p\n", check, &cur->right);
			check = &cur->right;
		} else {
			printf("check=%p,cur->left=%p\n", check, &cur->left);
			check = &cur->left;
		}
		counter++;
		if (counter == 10)
			panic("too many loops");
	}

	return NULL;
}

int rbtree_size(const RBTree *ptr) {
	return 0;
}

int rbtree_iterator(const RBTree *ptr, RBTreeIterator *iter) {
	return 0;
}
