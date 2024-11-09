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

#include <base/lock.h>
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
	Lock lock;
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
#define RIGHT_HEIGHT(n) (n->right_subtree_height_color & ~RED_NODE)
#define LEFT_HEIGHT(n) (n->left_subtree_height)
#define SET_RIGHT_HEIGHT(n, v)                                \
	({                                                        \
		(n->right_subtree_height_color =                      \
			 v | (n->right_subtree_height_color & RED_NODE)); \
	})
#define SET_LEFT_HEIGHT(n, v) ({ (n->left_subtree_height = v); })

void orbtree_update_heights(Ptr node, bool is_right, bool insert) {
	OrbTreeNodeImpl *last = orbtree_node(node);
	if (last == NULL) return;
	if (insert) {
		if (is_right)
			last->right_subtree_height_color++;

		else
			last->left_subtree_height++;
	}
	OrbTreeNodeImpl *parent;
	while (last->parent != null) {
		parent = orbtree_node(last->parent);
		if (orbtree_node(parent->right) == last) {
			unsigned int h = 1 + RIGHT_HEIGHT(last) + LEFT_HEIGHT(last);
			SET_RIGHT_HEIGHT(parent, h);
		} else {
			unsigned int h = 1 + RIGHT_HEIGHT(last) + LEFT_HEIGHT(last);
			SET_LEFT_HEIGHT(parent, h);
		}
		last = parent;
	}
}

void orbtree_rotate_right(Ptr x_ptr) {
	OrbTreeNodeImpl *x = orbtree_node(x_ptr);
	Ptr y_ptr = x->left;
	OrbTreeNodeImpl *y = orbtree_node(y_ptr);

	unsigned int y_right_height = RIGHT_HEIGHT(y);

	// Move subtree
	x->left = y->right;
	SET_LEFT_HEIGHT(x, y_right_height);

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
	unsigned int h = 1 + RIGHT_HEIGHT(x) + LEFT_HEIGHT(x);
	SET_RIGHT_HEIGHT(y, h);
	x->parent = y_ptr;
}

void orbtree_rotate_left(Ptr x_ptr) {
	OrbTreeNodeImpl *x = orbtree_node(x_ptr);
	Ptr y_ptr = x->right;
	OrbTreeNodeImpl *y = orbtree_node(y_ptr);

	unsigned int y_left_height = LEFT_HEIGHT(y);

	// Move subtree
	x->right = y->left;
	SET_RIGHT_HEIGHT(x, y_left_height);

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
	unsigned int h = 1 + RIGHT_HEIGHT(x) + LEFT_HEIGHT(x);
	SET_LEFT_HEIGHT(y, h);
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

void orbtree_init_node(OrbTreeNodeImpl *node, bool red, Ptr parent) {
	node->right = null;
	node->left = null;
	node->parent = parent;
	if (red)
		node->right_subtree_height_color = RED_NODE;
	else
		node->right_subtree_height_color = 0;
	node->left_subtree_height = 0;
}

void orbtree_insert_transplant(OrbTreeImpl *impl, const OrbTreeNodeImpl *prev,
							   OrbTreeNodeImpl *next, Ptr ptr, bool is_right) {
	next->parent = prev->parent;
	next->right = prev->right;
	next->left = prev->left;
	next->right_subtree_height_color = prev->right_subtree_height_color;
	next->left_subtree_height = prev->left_subtree_height;

	if (next->parent != null) {
		OrbTreeNodeImpl *parent = orbtree_node(next->parent);
		if (is_right) {
			parent->right = ptr;
		} else
			parent->left = ptr;
	}
	if (next->left != null) {
		OrbTreeNodeImpl *left = orbtree_node(next->left);
		left->parent = ptr;
	}
	if (next->right != null) {
		OrbTreeNodeImpl *right = orbtree_node(next->right);
		right->parent = ptr;
	}
}

Ptr orbtree_insert(OrbTreeImpl *impl, OrbTreeNodePair *pair, Ptr ptr) {
	if (pair->parent == null) {
		impl->root = pair->self;
		OrbTreeNodeImpl *self = (OrbTreeNodeImpl *)orbtree_node(pair->self);
		orbtree_init_node(self, false, null);
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
			orbtree_init_node(self, true, pair->parent);
		} else {
			OrbTreeNodeImpl *next = orbtree_value(ptr) + orbtree_tl_ctx.offset;
			Ptr ret =
				orbtree_node_ptr((const OrbTreeNode *)self, pair->is_right);
			orbtree_insert_transplant(impl, self, next, ptr, pair->is_right);
			return ret;
		}
	}
	return null;
}

void orbtree_remove_transplant(Ptr dst, Ptr src) {
	OrbTreeNodeImpl *dst_node = orbtree_node(dst);
	OrbTreeNodeImpl *dst_parent = orbtree_node(dst_node->parent);

	if (dst_node->parent == null)
		orbtree_tl_ctx.tree->root = src;
	else if (dst == dst_parent->left) {
		dst_parent->left = src;
		if (src == null) SET_LEFT_HEIGHT(dst_parent, 0);

	} else {
		dst_parent->right = src;
		if (src == null) SET_RIGHT_HEIGHT(dst_parent, 0);
	}
	if (src != null) {
		OrbTreeNodeImpl *src_node = orbtree_node(src);
		src_node->parent = dst_node->parent;

		OrbTreeNodeImpl *src_node_p = orbtree_node(src_node->parent);
		if (src_node_p) {
			unsigned int h = 1 + RIGHT_HEIGHT(src_node) + LEFT_HEIGHT(src_node);
			if (src_node_p->right == src)
				SET_RIGHT_HEIGHT(src_node_p, h);
			else
				SET_LEFT_HEIGHT(src_node_p, h);
		}
	}

	orbtree_update_heights(dst_node->parent, false, false);
}

Ptr orbtree_find_successor(Ptr x_ptr) {
	OrbTreeNodeImpl *x = orbtree_node(x_ptr);
	Ptr successor_ptr = x->right;
	OrbTreeNodeImpl *successor = orbtree_node(x->right);
	while (successor && successor->left != null) {
		successor_ptr = successor->left;
		successor = orbtree_node(successor->left);
	}
	return successor_ptr;
}

// set child's color to parent's
void orbtree_set_color_based_on_parent(Ptr child, Ptr parent) {
	if (child != null) {
		if (IS_RED(parent)) {
			SET_RED(child);
		} else {
			SET_BLACK(child);
		}
	}
}

void orbtree_remove_fixup(Ptr p_ptr, Ptr w_ptr, Ptr x_ptr) {
	while (x_ptr != orbtree_tl_ctx.tree->root && IS_BLACK(x_ptr)) {
		OrbTreeNodeImpl *parent_node = orbtree_node(p_ptr);
		OrbTreeNodeImpl *w_node = orbtree_node(w_ptr);
		OrbTreeNodeImpl *x_node = orbtree_node(x_ptr);
		if (w_ptr == parent_node->right) {
			// Case 1: Sibling is
			// red
			if (IS_RED(w_ptr)) {
				SET_BLACK(w_ptr);
				SET_RED(p_ptr);
				orbtree_rotate_left(p_ptr);
				w_ptr = parent_node->right;
				w_node = orbtree_node(w_ptr);
			}

			// Case 2: Sibling's
			// children are both
			// black
			if (IS_BLACK(w_node->left) && IS_BLACK(w_node->right)) {
				SET_RED(w_ptr);
				x_ptr = p_ptr;
				x_node = orbtree_node(x_ptr);
				p_ptr = parent_node->parent;
				parent_node = orbtree_node(p_ptr);
				OrbTreeNodeImpl *x_parent = orbtree_node(x_node->parent);
				if (x_parent == NULL) {
					w_ptr = null;
					w_node = NULL;
				} else if (x_ptr == x_parent->left) {
					w_ptr = x_parent->right;
					w_node = orbtree_node(w_ptr);
				} else {
					w_ptr = x_parent->left;
					w_node = orbtree_node(w_ptr);
				}
			} else {
				// Case 3: Sibling's
				// right child is
				// black, left child
				// is red
				if (IS_BLACK(w_node->right)) {
					SET_BLACK(w_node->left);
					SET_RED(w_ptr);
					orbtree_rotate_right(w_ptr);
					w_ptr = parent_node->right;
					w_node = orbtree_node(w_ptr);
				}

				// Case 4: Sibling's
				// right child is
				// red
				orbtree_set_color_based_on_parent(w_ptr, p_ptr);
				SET_BLACK(p_ptr);
				SET_BLACK(w_node->right);
				orbtree_rotate_left(p_ptr);
				x_ptr = orbtree_tl_ctx.tree->root;
				x_node = orbtree_node(x_ptr);
			}
		} else {
			// Case 1: Sibling is
			// red
			if (IS_RED(w_ptr)) {
				SET_BLACK(w_ptr);
				SET_RED(p_ptr);
				orbtree_rotate_right(p_ptr);
				w_ptr = parent_node->left;
				w_node = orbtree_node(w_ptr);
			}

			// Case 2: Sibling's
			// children are both
			// black
			if (IS_BLACK(w_node->right) && IS_BLACK(w_node->left)) {
				SET_RED(w_ptr);
				x_ptr = p_ptr;
				x_node = orbtree_node(x_ptr);
				p_ptr = parent_node->parent;
				parent_node = orbtree_node(p_ptr);
				OrbTreeNodeImpl *x_parent = orbtree_node(x_node->parent);
				if (x_parent == NULL) {
					w_ptr = null;
					w_node = NULL;
				} else if (x_ptr == x_parent->left) {
					w_ptr = x_parent->right;
					w_node = orbtree_node(w_ptr);
				} else {
					w_ptr = x_parent->left;
					w_node = orbtree_node(w_ptr);
				}
			} else {
				// Case 3: Sibling's
				// right child is
				// black, left child
				// is red
				if (IS_BLACK(w_node->left)) {
					SET_BLACK(w_node->right);
					SET_RED(w_ptr);
					orbtree_rotate_left(w_ptr);
					w_ptr = parent_node->left;
					w_node = orbtree_node(w_ptr);
				}

				// Case 4: Sibling's
				// right child is
				// red
				orbtree_set_color_based_on_parent(w_ptr, p_ptr);
				SET_BLACK(p_ptr);
				SET_BLACK(w_node->left);
				orbtree_rotate_right(p_ptr);
				x_ptr = orbtree_tl_ctx.tree->root;
				x_node = orbtree_node(x_ptr);
			}
		}
	}

	// Ensure x is black at the end
	// of fixup
	SET_BLACK(x_ptr);
}

void orbtree_remove_impl(Ptr ptr, bool is_right) {
	bool do_fixup = IS_BLACK(ptr);
	OrbTreeNodeImpl *node_to_delete = orbtree_node(ptr);

	Ptr x_ptr = null, w_ptr = null, p_ptr = null;
	OrbTreeNodeImpl *x = NULL, *w = NULL, *p = NULL;

	if (node_to_delete->left == null) {
		x_ptr = node_to_delete->right;
		x = orbtree_node(x_ptr);
		orbtree_remove_transplant(ptr, node_to_delete->right);
		OrbTreeNodeImpl *node_to_delete_parent =
			orbtree_node(node_to_delete->parent);
		if (node_to_delete->parent != null) {
			if (node_to_delete_parent->left == null) {
				w_ptr = node_to_delete_parent->right;
				w = orbtree_node(w_ptr);
			} else if (node_to_delete_parent) {
				w_ptr = node_to_delete_parent->left;
				w = orbtree_node(w_ptr);
			}
		}
		if (x_ptr != null) {
			x = orbtree_node(x_ptr);
			p_ptr = x->parent;
			p = orbtree_node(p_ptr);
		} else if (w_ptr != null) {
			p_ptr = w->parent;
			p = orbtree_node(p_ptr);
		}
	} else if (node_to_delete->right == null) {
		x_ptr = node_to_delete->left;
		x = orbtree_node(x_ptr);
		orbtree_remove_transplant(ptr, node_to_delete->left);
		OrbTreeNodeImpl *node_to_delete_parent =
			orbtree_node(node_to_delete->parent);
		if (node_to_delete_parent) {
			w_ptr = node_to_delete_parent->left;
			w = orbtree_node(w_ptr);
		}
		p_ptr = x->parent;
		p = orbtree_node(p_ptr);
	} else {
		Ptr successor_ptr = orbtree_find_successor(ptr);
		OrbTreeNodeImpl *successor = orbtree_node(successor_ptr);
		do_fixup = IS_BLACK(successor_ptr);

		x_ptr = successor->right;
		OrbTreeNodeImpl *successor_parent = orbtree_node(successor->parent);
		w_ptr = successor_parent->right;
		x = orbtree_node(x_ptr);
		w = orbtree_node(w_ptr);

		if (w == NULL) {
			p_ptr = null;
			p = NULL;
		} else if (w->parent == ptr) {
			w_ptr = node_to_delete->left;
			w = orbtree_node(w_ptr);
			p_ptr = successor_ptr;
			p = successor;
		} else {
			p_ptr = w->parent;
			p = orbtree_node(p_ptr);
		}

		if (successor->parent != ptr) {
			orbtree_remove_transplant(successor_ptr, successor->right);
			successor->right = node_to_delete->right;
			SET_RIGHT_HEIGHT(successor, 0);
			OrbTreeNodeImpl *successor_right = orbtree_node(successor->right);
			if (successor_right) {
				unsigned int h = 1 + RIGHT_HEIGHT(successor_right) +
								 LEFT_HEIGHT(successor_right);
				SET_RIGHT_HEIGHT(successor, h);
				successor_right->parent = successor_ptr;
			}
		}

		orbtree_remove_transplant(ptr, successor_ptr);
		successor->left = node_to_delete->left;

		OrbTreeNodeImpl *successor_left = orbtree_node(successor->left);
		SET_LEFT_HEIGHT(successor, 0);
		if (node_to_delete->left != null) {
			OrbTreeNodeImpl *ntdl = orbtree_node(node_to_delete->left);
			unsigned int h = 1 + LEFT_HEIGHT(ntdl) + RIGHT_HEIGHT(ntdl);
			SET_LEFT_HEIGHT(successor, h);
		}
		successor_left->parent = successor_ptr;
		orbtree_set_color_based_on_parent(successor_ptr, ptr);

		orbtree_update_heights(successor_ptr, false, false);
	}

	if (do_fixup) {
		if (w_ptr != null && p_ptr != null) {
			orbtree_remove_fixup(p_ptr, w_ptr, x_ptr);
		} else {
			if (orbtree_tl_ctx.tree->root) SET_BLACK(orbtree_tl_ctx.tree->root);
		}
	}
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

unsigned int orbtree_node_right_subtree_height(const OrbTreeNode *node) {
	if (node == NULL) return 0;
	return RIGHT_HEIGHT(((OrbTreeNodeImpl *)node));
}

unsigned int orbtree_node_left_subtree_height(const OrbTreeNode *node) {
	if (node == NULL) return 0;
	return LEFT_HEIGHT(((OrbTreeNodeImpl *)node));
}

bool orbtree_node_is_red(const OrbTreeNode *node) {
	const OrbTreeNodeImpl *impl = (const OrbTreeNodeImpl *)node;
	return (impl->right_subtree_height_color & RED_NODE) != 0;
}

Ptr orbtree_root(const OrbTree *tree) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	return impl->root;
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

void orbtree_set_tl_context(OrbTreeImpl *impl,
							const OrbTreeNodeWrapper *value) {
	orbtree_tl_ctx.sa = impl->sa;
	orbtree_tl_ctx.tree = impl;
	orbtree_tl_ctx.offset = value->offsetof;
}

// use subtree heights to navigate to correct offset
Ptr orbtree_adjust_offset(Ptr ret, unsigned int offset) {
	while (offset && ret) {
		OrbTreeNodeImpl *cur = orbtree_node(ret);
		unsigned int rh = RIGHT_HEIGHT(cur);
		if (rh < offset) {
			offset -= rh + 1;
			while (ret) {
				OrbTreeNodeImpl *n = orbtree_node(ret);
				if (n->parent) {
					OrbTreeNodeImpl *parent = orbtree_node(n->parent);
					if (parent->left == ret) {
						ret = n->parent;
						break;
					}
				}
				ret = n->parent;
			}
		} else {
			offset -= 1;
			ret = cur->right;
			cur = orbtree_node(ret);
			while (cur->left) {
				unsigned int lh = LEFT_HEIGHT(cur);
				if (lh <= offset) {
					offset -= lh;
					break;
				}
				ret = cur->left;
				cur = orbtree_node(cur->left);
			}
		}
	}
	return ret;
}

int orbtree_init(OrbTree *tree, const SlabAllocator *sa) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	impl->sa = sa;
	impl->root = null;
	impl->lock = INIT_LOCK;
	return 0;
}

Ptr orbtree_get(const OrbTree *tree, const OrbTreeNodeWrapper *value,
				OrbTreeSearch search, unsigned int offset) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	OrbTreeNode *target = (OrbTreeNode *)orbtree_node(value->ptr);
	OrbTreeNodePair retval = {};
	orbtree_set_tl_context(impl, value);
	Ptr ret = null;

	lockr(&impl->lock);
	if (impl->root != null) {
		OrbTreeNode *root = (OrbTreeNode *)orbtree_node(impl->root);
		search(root, target, &retval);
		ret = orbtree_adjust_offset(retval.self, offset);
	}
	unlock(&impl->lock);

	return ret;
}

Ptr orbtree_put(OrbTree *tree, const OrbTreeNodeWrapper *value,
				OrbTreeSearch search) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	orbtree_set_tl_context(impl, value);
	OrbTreeNode *target = (OrbTreeNode *)orbtree_node(value->ptr);
	OrbTreeNodePair pair = {.parent = null, .self = value->ptr};

	lockw(&impl->lock);
	if (impl->root != null) {
		OrbTreeNode *root = (OrbTreeNode *)orbtree_node(impl->root);
		search(root, target, &pair);
	}

	Ptr ret = orbtree_insert(impl, &pair, value->ptr);
	if (ret == null) {
		orbtree_update_heights(pair.parent, pair.is_right, true);
		orbtree_put_fixup(value->ptr);
	}
	unlock(&impl->lock);

	return ret;
}

Ptr orbtree_remove(OrbTree *tree, const OrbTreeNodeWrapper *value,
				   OrbTreeSearch search) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	orbtree_set_tl_context(impl, value);
	OrbTreeNode *target = (OrbTreeNode *)orbtree_node(value->ptr);
	OrbTreeNodePair pair = {.parent = null, .self = value->ptr};

	lockw(&impl->lock);
	if (impl->root != null) {
		OrbTreeNode *root = (OrbTreeNode *)orbtree_node(impl->root);
		search(root, target, &pair);
	}
	if (pair.self != null) orbtree_remove_impl(pair.self, pair.is_right);
	unlock(&impl->lock);

	return pair.self;
}
