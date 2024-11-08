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

#include <base/macro_util.h>
#include <base/print_util.h>
#include <base/slabs.h>
#include <core/orbtree.h>

typedef struct OrbTreeNodeImpl {
	Ptr parent;
	Ptr right;
	Ptr left;
	unsigned int right_subtree_height_color;
	unsigned int left_subtree_height;
} OrbTreeNodeImpl;

typedef struct OrbTreeImpl {
	const SlabAllocator *sa;
	Ptr root;
	byte padding[28];
} OrbTreeImpl;

typedef struct OrbTreeCtx {
	const SlabAllocator *sa;
	OrbTreeImpl *tree;
	unsigned int offset;
} OrbTreeCtx;

_Thread_local OrbTreeCtx orbtree_tl_ctx = {};

void __attribute__((constructor)) __orbtree_check_sizes() {
	if (sizeof(OrbTreeNodeImpl) != sizeof(OrbTreeNode))
		panic("sizeof(OrbTreeNodeImpl) (%i) != sizeof(OrbTreeNode) (%i)",
			  sizeof(OrbTreeNodeImpl), sizeof(OrbTreeNode));
	if (sizeof(OrbTreeImpl) != sizeof(OrbTree))
		panic("sizeof(OrbTreeImpl) (%i) != sizeof(OrbTree) (%i)",
			  sizeof(OrbTreeImpl), sizeof(OrbTree));
}

void *orbtree_value(Ptr ptr) {
	return slab_get(orbtree_tl_ctx.sa, ptr);
}

OrbTreeNodeImpl *orbtree_node(Ptr ptr) {
	byte *slab = slab_get(orbtree_tl_ctx.sa, ptr);
	if (slab == NULL) return NULL;
	return (OrbTreeNodeImpl *)(slab + orbtree_tl_ctx.offset);
}

#define RED_NODE 0x80000000
#define IS_RED(k)                                                    \
	({                                                               \
		OrbTreeNodeImpl *_impl__ = orbtree_node(k);                  \
		int _ret__;                                                  \
		if (_impl__ == NULL)                                         \
			_ret__ = 0;                                              \
		else                                                         \
			_ret__ = _impl__->right_subtree_height_color & RED_NODE; \
		_ret__;                                                      \
	})
#define IS_BLACK(k) !IS_RED(k)
#define SET_BLACK(k)                                      \
	({                                                    \
		OrbTreeNodeImpl *_impl__ = orbtree_node(k);       \
		_impl__->right_subtree_height_color &= ~RED_NODE; \
	})
#define SET_RED(k)                                       \
	({                                                   \
		OrbTreeNodeImpl *_impl__ = orbtree_node(k);      \
		_impl__->right_subtree_height_color |= RED_NODE; \
	})

void orbtree_rotate_right(Ptr x_ptr) {
	OrbTreeNodeImpl *x = orbtree_node(x_ptr);
	Ptr y_ptr = x->left;
	OrbTreeNodeImpl *y = orbtree_node(y_ptr);

	// Move subtree
	x->left = y->right;

	if (y->right != null) {
		OrbTreeNodeImpl *yright = orbtree_node(y->right);
		yright->parent = x_ptr;
	}

	// Update y's parent to x's parent
	y->parent = x->parent;

	// If x was the root, now y becomes the root
	OrbTreeNodeImpl *xparent = orbtree_node(x->parent);
	if (x->parent == null) {
		orbtree_tl_ctx.tree->root = y_ptr;
	} else if (x_ptr == xparent->right) {
		xparent->right = y_ptr;
	} else {
		xparent->left = y_ptr;
	}

	// Place x as y's left child
	y->right = x_ptr;
	x->parent = y_ptr;
}

void orbtree_rotate_left(Ptr x_ptr) {
	OrbTreeNodeImpl *x = orbtree_node(x_ptr);
	Ptr y_ptr = x->right;
	OrbTreeNodeImpl *y = orbtree_node(y_ptr);

	// Move subtree
	x->right = y->left;

	if (y->left != null) {
		OrbTreeNodeImpl *yleft = orbtree_node(y->left);
		yleft->parent = x_ptr;
	}

	// Update y's parent to x's parent
	y->parent = x->parent;

	// If x was the root, now y becomes the root
	OrbTreeNodeImpl *xparent = orbtree_node(x->parent);
	if (x->parent == null) {
		orbtree_tl_ctx.tree->root = y_ptr;
	} else if (x_ptr == xparent->left) {
		xparent->left = y_ptr;
	} else {
		xparent->right = y_ptr;
	}

	// Place x as y's left child
	y->left = x_ptr;
	x->parent = y_ptr;
}

int orbtree_put_fixup(Ptr k_ptr) {
	OrbTreeNodeImpl *k = orbtree_node(k_ptr);
	while (k_ptr != orbtree_tl_ctx.tree->root && IS_RED(k->parent)) {
		k = orbtree_node(k_ptr);
		OrbTreeNodeImpl *parent = orbtree_node(k->parent);
		OrbTreeNodeImpl *gparent = orbtree_node(parent->parent);

		if (k->parent == gparent->left) {
			// Case 1: Uncle is on the right
			OrbTreeNodeImpl *u = orbtree_node(gparent->right);
			Ptr u_ptr = gparent->right;
			if (IS_RED(u_ptr)) {
				// Case 1a: Uncle is red
				// Recolor the parent and uncle to black
				SET_BLACK(k->parent);
				SET_BLACK(u_ptr);
				// Recolor the grandparent to red
				SET_RED(parent->parent);

				// Move up the tree
				k_ptr = parent->parent;
				k = orbtree_node(k_ptr);
			} else {
				// Case 1b: Uncle is black
				if (k_ptr == parent->right) {
					// Case 1b1: Node is a right child
					// Rotate left to make the node the left child
					k_ptr = k->parent;
					k = orbtree_node(k_ptr);
					orbtree_rotate_left(k_ptr);
				}
				// Recolor and rotate
				OrbTreeNodeImpl *kparent = orbtree_node(k->parent);
				SET_BLACK(k->parent);
				SET_RED(kparent->parent);

				orbtree_rotate_right(kparent->parent);
			}
		} else {
			// Case 2: Uncle is on the left
			OrbTreeNodeImpl *u = orbtree_node(gparent->left);
			Ptr u_ptr = gparent->left;
			if (IS_RED(u_ptr)) {
				// Case 2a: Uncle is red
				// Recolor the parent and uncle to black
				SET_BLACK(k->parent);
				SET_BLACK(u_ptr);
				// Recolor the grandparent to red
				SET_RED(parent->parent);

				// Move up the tree
				k_ptr = parent->parent;
				k = orbtree_node(k_ptr);
			} else {
				// Case 2b: Uncle is black
				if (k_ptr == parent->left) {
					// Case 2b1: Node is a left child
					// Rotate right to make the node the right child
					k_ptr = k->parent;
					k = orbtree_node(k_ptr);
					orbtree_rotate_right(k_ptr);
				}
				// Recolor and rotate
				SET_BLACK(k->parent);
				OrbTreeNodeImpl *kparent = orbtree_node(k->parent);
				SET_RED(kparent->parent);
				orbtree_rotate_left(kparent->parent);
			}
		}
	}
	// Ensure the root is always black
	SET_BLACK(orbtree_tl_ctx.tree->root);
	return 0;
}

int orbtree_insert(OrbTreeImpl *impl, OrbTreeNodePair *pair, Ptr ptr) {
	if (pair->parent == null) {
		impl->root = pair->self;
		OrbTreeNodeImpl *self = (OrbTreeNodeImpl *)orbtree_node(pair->self);
		self->right = null;
		self->left = null;
		self->parent = null;
		self->right_subtree_height_color = 0;
		self->left_subtree_height = 0;
	} else {
		OrbTreeNodeImpl *self = (OrbTreeNodeImpl *)orbtree_node(pair->self);
		if (self == NULL) {
			OrbTreeNodeImpl *parent =
				(OrbTreeNodeImpl *)orbtree_node(pair->parent);
			if (pair->is_right)
				parent->right = ptr;
			else
				parent->left = ptr;
			self = (OrbTreeNodeImpl *)orbtree_node(ptr);
			self->parent = pair->parent;
			self->right = null;
			self->left = null;
			self->right_subtree_height_color = RED_NODE;
			self->left_subtree_height = 0;
		} else {
		}
	}
	return 0;
}

void *orbtree_node_right(const OrbTreeNode *node) {
	const OrbTreeNodeImpl *impl = (const OrbTreeNodeImpl *)node;
	if (impl->right == null) return NULL;
	return orbtree_value(impl->right);
}

void *orbtree_node_left(const OrbTreeNode *node) {
	const OrbTreeNodeImpl *impl = (const OrbTreeNodeImpl *)node;
	if (impl->left == null) return NULL;
	return orbtree_value(impl->left);
}

void *orbtree_node_parent(const OrbTreeNode *node) {
	const OrbTreeNodeImpl *impl = (const OrbTreeNodeImpl *)node;
	if (impl->parent == null) return NULL;
	return orbtree_value(impl->parent);
}

bool orbtree_node_is_red(const OrbTreeNode *node) {
	const OrbTreeNodeImpl *impl = (const OrbTreeNodeImpl *)node;
	return (impl->right_subtree_height_color & RED_NODE) != 0;
}

Ptr orbtree_node_ptr(const OrbTreeNode *node, bool is_right) {
	if (node == NULL) return null;
	const OrbTreeNodeImpl *impl = (const OrbTreeNodeImpl *)node;
	if (impl->parent == null) {
		return orbtree_tl_ctx.tree->root;
	} else {
		OrbTreeNodeImpl *node = (OrbTreeNodeImpl *)orbtree_node(impl->parent);
		if (is_right)
			return node->right;
		else
			return node->left;
	}
}

int orbtree_init(OrbTree *tree, const SlabAllocator *sa) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	impl->sa = sa;
	impl->root = null;
	return 0;
}

void orbtree_set_tl_context(OrbTreeImpl *impl,
							const OrbTreeNodeWrapper *value) {
	orbtree_tl_ctx.sa = impl->sa;
	orbtree_tl_ctx.tree = impl;
	orbtree_tl_ctx.offset = value->offset_of_orbtree_node;
}

void *orbtree_get(const OrbTree *tree, const OrbTreeNodeWrapper *value,
				  OrbTreeSearch search, unsigned int offset) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	if (impl->root == null) return NULL;
	orbtree_set_tl_context(impl, value);
	OrbTreeNode *root = (OrbTreeNode *)orbtree_node(impl->root);
	OrbTreeNode *target = (OrbTreeNode *)orbtree_node(value->ptr);
	OrbTreeNodePair retval = {};
	search(root, target, &retval);

	if (retval.self != null)
		return orbtree_value(retval.self);
	else
		return NULL;
}

void *orbtree_put(OrbTree *tree, const OrbTreeNodeWrapper *value,
				  OrbTreeSearch search) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	orbtree_set_tl_context(impl, value);

	OrbTreeNode *target = (OrbTreeNode *)orbtree_node(value->ptr);

	OrbTreeNodePair pair = {.parent = null, .self = value->ptr};
	if (impl->root != null) {
		OrbTreeNode *root = (OrbTreeNode *)orbtree_node(impl->root);
		search(root, target, &pair);
	}

	orbtree_insert(impl, &pair, value->ptr);
	orbtree_put_fixup(value->ptr);

	return NULL;
}

void *orbtree_remove(OrbTree *tree, const OrbTreeNodeWrapper *value,
					 OrbTreeSearch search) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	orbtree_set_tl_context(impl, value);
	return NULL;
}

Ptr orbtree_root(const OrbTree *tree) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	return impl->root;
}
