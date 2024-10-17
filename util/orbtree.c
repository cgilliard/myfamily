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
#include <base/panic.h>
#include <base/resources.h>
#include <stdio.h>
#include <util/orbtree.h>

#define NODES_PER_CHUNK 100
#define CHUNKS_PER_RESIZE 10

u64 orbtree_next_node_id = 10;

#define RED_MASK 0x80000000ULL

#define SET_RED(impl, id)                                                                          \
	({                                                                                             \
		ORBTreeNode *_orb_node__ = orbtree_node(impl, id);                                         \
		_orb_node__->color |= RED_MASK;                                                            \
	})
#define SET_BLACK(impl, id)                                                                        \
	({                                                                                             \
		ORBTreeNode *_orb_node__ = orbtree_node(impl, id);                                         \
		_orb_node__->color &= ~RED_MASK;                                                           \
	})
#define IS_RED(impl, id)                                                                           \
	({                                                                                             \
		ORBTreeNode *_orb_node__ = orbtree_node(impl, id);                                         \
		_orb_node__ != NULL && (_orb_node__->color & RED_MASK) != 0;                               \
	})
#define IS_BLACK(impl, id)                                                                         \
	({                                                                                             \
		ORBTreeNode *_orb_node__ = orbtree_node(impl, id);                                         \
		_orb_node__ == NULL || (_orb_node__->color & RED_MASK) == 0;                               \
	})

typedef struct ORBTreeNode {
	u32 color;
	u32 right;
	u32 left;
	u32 parent;
	u32 right_subtree_size;
	u32 left_subtree_size;
#ifdef TEST
	u64 node_id;
#endif // TEST
	char data[];
} ORBTreeNode;

// Data structure used for searching ORBTrees.
typedef struct ORBTreeNodePair {
	u32 parent;
	u32 self;
	bool is_right;
} ORBTreeNodePair;

static u32 NIL = (UINT32_MAX - 2);

// The internal ORBTreeImpl storage data structure
typedef struct ORBTreeImpl {
	u64 value_size; // size of the values

	// a comparion function (like qsort)
	int (*compare)(const void *, const void *);
	u32 root;	  // offset to the root node.
	u32 size;	  // current number of slabs allocated for the tree.
	u32 capacity; // current capacity in terms of number of slabs for the tree.
	u32 elements; // number of elements in the tree.
	u32 free_list_head;
	u8 **data_chunks;
	u32 cur_chunks;
	u32 *free_list;
} ORBTreeImpl;

void orbtree_print_impl(ORBTreeImpl *impl);
void orbtree_validate_impl(const ORBTreeImpl *impl);

void orbtree_iterator_cleanup(ORBTreeIteratorNc *ptr) {
}

bool orbtree_iterator_next(ORBTreeIterator *ptr, ORBTreeTray *value) {
	return false;
}

void orbtree_cleanup(ORBTreeNc *ptr) {
	if (ptr->impl) {
		ORBTreeImpl *impl = ptr->impl;
		if (impl->data_chunks) {
			for (u32 i = 0; i < impl->capacity; i += NODES_PER_CHUNK) {
				myfree(impl->data_chunks[i / NODES_PER_CHUNK]);
			}
			myfree(impl->data_chunks);
			impl->data_chunks = NULL;
		}

		if (impl->free_list) {
			myfree(impl->free_list);
			impl->free_list = NULL;
		}
		myfree(ptr->impl);
		ptr->impl = NULL;
	}
}

int orbtree_create(ORBTree *ptr, const u64 value_size, int (*compare)(const void *, const void *)) {
	// validate input
	if (ptr == NULL || value_size == 0 || compare == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	ptr->impl = mymalloc(sizeof(ORBTreeImpl));
	if (ptr->impl == NULL) {
		return -1;
	}
	ORBTreeImpl *impl = ptr->impl;
	impl->value_size = value_size;
	impl->compare = compare;
	impl->root = NIL;
	impl->size = 0;
	impl->capacity = 0;
	impl->free_list_head = UINT32_MAX;
	impl->data_chunks = NULL;
	impl->cur_chunks = 0;
	impl->elements = 0;
	impl->free_list = NULL;

	return 0;
}

ORBTreeNode *orbtree_node(const ORBTreeImpl *impl, u32 node) {
	if (node == NIL) {
		return NULL;
	}
	u64 index = node / NODES_PER_CHUNK;
	u64 offset = node % NODES_PER_CHUNK;
	u64 offset_sum = (offset * (sizeof(ORBTreeNode) + impl->value_size));
	return (ORBTreeNode *)(impl->data_chunks[index] + offset_sum);
}

void *orbtree_value(const ORBTreeImpl *impl, u32 node) {
	return orbtree_node(impl, node)->data;
}

// internal search function used by get/insert/delete.
void orbtree_search(const ORBTreeImpl *impl, const void *value, ORBTreeNodePair *nodes) {
	nodes->parent = NIL;
	nodes->self = impl->root;

	int i = 0;
	while (nodes->self != NIL) {
		nodes->parent = nodes->self;
		int v = impl->compare(orbtree_value(impl, nodes->self), value);
		if (v == 0) {
			break;
		} else if (v < 0) {
			ORBTreeNode *self = orbtree_node(impl, nodes->self);
			nodes->self = self->right;
			nodes->is_right = true;
		} else {
			ORBTreeNode *self = orbtree_node(impl, nodes->self);
			nodes->self = self->left;
			nodes->is_right = false;
		}
	}
}

void orbtree_left_rotate(ORBTreeImpl *impl, u32 x_id) {
	ORBTreeNode *x = orbtree_node(impl, x_id);
	u32 y_id = x->right;
	ORBTreeNode *y = orbtree_node(impl, y_id);

	// Move y's left subtree to x's right subtree
	x->right = y->left;
	if (y->left != NIL) {
		ORBTreeNode *yleft = orbtree_node(impl, y->left);
		yleft->parent = x_id;
	}

	// Update y's parent to x's parent
	y->parent = x->parent;

	// If x was the root, now y becomes the root
	ORBTreeNode *xparent = orbtree_node(impl, x->parent);
	if (x->parent == NIL) {
		impl->root = y_id;
	} else if (x_id == xparent->left) {
		xparent->left = y_id;
	} else {
		xparent->right = y_id;
	}

	// Place x as y's left child
	y->left = x_id;
	x->parent = y_id;
}

void orbtree_right_rotate(ORBTreeImpl *impl, u32 x_id) {
	ORBTreeNode *x = orbtree_node(impl, x_id);
	u32 y_id = x->left;
	ORBTreeNode *y = orbtree_node(impl, y_id);
	// Move y's right subtree to x's right subtree
	x->left = y->right;
	if (y->right != NIL) {
		ORBTreeNode *yright = orbtree_node(impl, y->right);
		yright->parent = x_id;
	}

	// Update y's parent to x's parent
	y->parent = x->parent;

	// If x was the root, now y becomes the root
	ORBTreeNode *xparent = orbtree_node(impl, x->parent);
	if (x->parent == NIL) {
		impl->root = y_id;
	} else if (x_id == xparent->right) {
		xparent->right = y_id;
	} else {
		xparent->left = y_id;
	}

	// Place x as y's left child
	y->right = x_id;
	x->parent = y_id;
}

// find the successor node
u32 orbtree_find_successor(ORBTreeImpl *impl, u32 x_id) {
	ORBTreeNode *x = orbtree_node(impl, x_id);
	u32 successor_id = x->right;
	ORBTreeNode *successor = orbtree_node(impl, x->right);
	while (successor && successor->left != NIL) {
		successor_id = successor->left;
		successor = orbtree_node(impl, successor->left);
	}
	return successor_id;
}

// perform transplant operation
void orbtree_transplant(ORBTreeImpl *impl, u32 dst, u32 src) {
	ORBTreeNode *dst_node = orbtree_node(impl, dst);
	ORBTreeNode *dst_parent = orbtree_node(impl, dst_node->parent);
	if (dst_node->parent == NIL)
		impl->root = src;
	else if (dst == dst_parent->left)
		dst_parent->left = src;
	else
		dst_parent->right = src;
	if (src != NIL) {
		ORBTreeNode *src_node = orbtree_node(impl, src);
		src_node->parent = dst_node->parent;
	}
}

void orbtree_put_fixup(ORBTreeImpl *impl, u32 k_id) {
	ORBTreeNode *k = orbtree_node(impl, k_id);

	while (k_id != impl->root && IS_RED(impl, k->parent)) {
		k = orbtree_node(impl, k_id);
		ORBTreeNode *parent = orbtree_node(impl, k->parent);
		ORBTreeNode *gparent = orbtree_node(impl, parent->parent);

		if (k->parent == gparent->left) {
			// Case 1: Uncle is on the right
			ORBTreeNode *u = orbtree_node(impl, gparent->right);
			u32 u_id = gparent->right;
			if (IS_RED(impl, u_id)) {
				// Case 1a: Uncle is red
				// Recolor the parent and uncle to black
				SET_BLACK(impl, k->parent);
				SET_BLACK(impl, u_id);
				// Recolor the grandparent to red
				SET_RED(impl, parent->parent);

				// Move up the tree
				k_id = parent->parent;
				k = orbtree_node(impl, k_id);
			} else {
				// Case 1b: Uncle is black
				if (k_id == parent->right) {
					// Case 1b1: Node is a right child
					// Rotate left to make the node the left child
					k_id = k->parent;
					k = orbtree_node(impl, k_id);
					orbtree_left_rotate(impl, k_id);
				}
				// Recolor and rotate
				ORBTreeNode *kparent = orbtree_node(impl, k->parent);
				SET_BLACK(impl, k->parent);
				SET_RED(impl, kparent->parent);

				orbtree_right_rotate(impl, kparent->parent);
			}
		} else {
			// Case 2: Uncle is on the left
			ORBTreeNode *u = orbtree_node(impl, gparent->left);
			u32 u_id = gparent->left;
			if (IS_RED(impl, u_id)) {
				// Case 2a: Uncle is red
				// Recolor the parent and uncle to black
				SET_BLACK(impl, k->parent);
				SET_BLACK(impl, u_id);
				// Recolor the grandparent to red
				SET_RED(impl, parent->parent);

				// Move up the tree
				k_id = parent->parent;
				k = orbtree_node(impl, k_id);
			} else {
				// Case 2b: Uncle is black
				if (k_id == parent->left) {
					// Case 2b1: Node is a right child
					// Rotate left to make the node the left child
					k_id = k->parent;
					k = orbtree_node(impl, k_id);
					orbtree_right_rotate(impl, k_id);
				}
				// Recolor and rotate
				SET_BLACK(impl, k->parent);
				ORBTreeNode *kparent = orbtree_node(impl, k->parent);
				SET_RED(impl, kparent->parent);
				orbtree_left_rotate(impl, kparent->parent);
			}
		}
	}
	// Ensure the root is always black
	SET_BLACK(impl, impl->root);
}

int orbtree_put(ORBTree *ptr, ORBTreeTray *value, ORBTreeTray *replaced) {
	// validate input
	if (ptr == NULL || value == NULL || replaced == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	// obtain the impl from the ptr
	ORBTreeImpl *impl = ptr->impl;
	// this pair is used to search
	ORBTreeNodePair pair;

	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	// perform search for the key
	orbtree_search(impl, value->value, &pair);
	if (pair.self != NIL) {
		replaced->value = orbtree_value(impl, pair.self);
		replaced->updated = true;
		replaced->id = pair.self;
	}

	if (impl->root == NIL) {
		impl->root = value->id;

	} else if (pair.is_right) {
		ORBTreeNode *parent = orbtree_node(impl, pair.parent);
		parent->right = value->id;
	} else {
		ORBTreeNode *parent = orbtree_node(impl, pair.parent);
		parent->left = value->id;
	}

	ORBTreeNode *node = orbtree_node(impl, value->id);
	node->right = NIL;
	node->left = NIL;
	node->parent = pair.parent;

#ifdef TEST
	node->node_id = orbtree_next_node_id++;
#endif // TEST

	SET_RED(impl, value->id);
	impl->elements++;

	orbtree_put_fixup(impl, value->id);

	return 0;
}

// set child's color to parent's
void orbtree_set_color_based_on_parent(ORBTreeImpl *impl, u32 child, u32 parent) {
	if (child != NIL) {
		if (IS_RED(impl, parent)) {
			SET_RED(impl, child);
		} else {
			SET_BLACK(impl, child);
		}
	}
}

void orbtree_remove_fixup(ORBTreeImpl *impl, u32 parent, u32 w, u32 x) {
	while (x != impl->root && IS_BLACK(impl, x)) {
		ORBTreeNode *parent_node = orbtree_node(impl, parent);
		ORBTreeNode *w_node = orbtree_node(impl, w);
		ORBTreeNode *x_node = orbtree_node(impl, x);
		u64 w_nid;
		if (w_node)
			w_nid = w_node->node_id;
		else
			w_nid = 0;
		u64 x_nid;
		if (x_node)
			x_nid = x_node->node_id;
		else
			x_nid = 0;
		u64 p_nid;
		if (parent_node)
			p_nid = parent_node->node_id;
		else
			p_nid = 0;
		if (w == parent_node->right) {
			// Case 1: Sibling is red
			if (IS_RED(impl, w)) {
				SET_BLACK(impl, w);
				SET_RED(impl, parent);
				orbtree_left_rotate(impl, parent);
				w = parent_node->right;
				w_node = orbtree_node(impl, w);
			}

			// Case 2: Sibling's children are both black
			if (IS_BLACK(impl, w_node->left) && IS_BLACK(impl, w_node->right)) {
				SET_RED(impl, w);
				x = parent;
				x_node = orbtree_node(impl, x);
				parent = parent_node->parent;
				parent_node = orbtree_node(impl, parent);
				ORBTreeNode *x_parent = orbtree_node(impl, x_node->parent);
				if (x_parent == NULL) {
					w = NIL;
					w_node = NULL;
				} else if (x == x_parent->left) {
					w = x_parent->right;
					w_node = orbtree_node(impl, w);
				} else {
					w = x_parent->left;
					w_node = orbtree_node(impl, w);
				}
			} else {
				// Case 3: Sibling's right child is black, left child is red
				if (IS_BLACK(impl, w_node->right)) {
					SET_BLACK(impl, w_node->left);
					SET_RED(impl, w);
					orbtree_right_rotate(impl, w);
					w = parent_node->right;
					w_node = orbtree_node(impl, w);
				}

				// Case 4: Sibling's right child is red
				orbtree_set_color_based_on_parent(impl, w, parent);
				SET_BLACK(impl, parent);
				SET_BLACK(impl, w_node->right);
				orbtree_left_rotate(impl, parent);
				x = impl->root; // Set x to root at the end, only once
				x_node = orbtree_node(impl, x);
			}
		} else {
			// Case 1: Sibling is red
			if (IS_RED(impl, w)) {
				SET_BLACK(impl, w);
				SET_RED(impl, parent);
				orbtree_right_rotate(impl, parent);
				w = parent_node->left;
				w_node = orbtree_node(impl, w);
			}

			// Case 2: Sibling's children are both black
			if (IS_BLACK(impl, w_node->right) && IS_BLACK(impl, w_node->left)) {
				SET_RED(impl, w);
				x = parent;
				x_node = orbtree_node(impl, x);
				parent = parent_node->parent;
				parent_node = orbtree_node(impl, parent);
				ORBTreeNode *x_parent = orbtree_node(impl, x_node->parent);
				if (x_parent == NULL) {
					w = NIL;
					w_node = NULL;
				} else if (x == x_parent->left) {
					w = x_parent->right;
					w_node = orbtree_node(impl, w);
				} else {
					w = x_parent->left;
					w_node = orbtree_node(impl, w);
				}
			} else {
				// Case 3: Sibling's right child is black, left child is red
				if (IS_BLACK(impl, w_node->left)) {
					SET_BLACK(impl, w_node->right);
					SET_RED(impl, w);
					orbtree_left_rotate(impl, w);
					w = parent_node->left;
					w_node = orbtree_node(impl, w);
				}

				// Case 4: Sibling's right child is red
				orbtree_set_color_based_on_parent(impl, w, parent);
				SET_BLACK(impl, parent);
				SET_BLACK(impl, w_node->left);
				orbtree_right_rotate(impl, parent);
				x = impl->root; // Set x to root at the end, only once
				x_node = orbtree_node(impl, x);
			}
		}
	}

	// Ensure x is black at the end of fixup
	SET_BLACK(impl, x);
}

int orbtree_remove(ORBTree *ptr, const void *value, ORBTreeTray *removed) {
	// validate input
	if (ptr == NULL || value == NULL || removed == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	// obtain impl from the pointer
	ORBTreeImpl *impl = ptr->impl;

	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	// search for the node based on this key.
	ORBTreeNodePair pair;
	orbtree_search(impl, value, &pair);

	// this node doesn't exist, return -1
	if (pair.self == NIL) {
		removed->updated = false;
		return -1;
	}

	removed->value = orbtree_value(impl, pair.self);
	removed->updated = true;
	removed->id = pair.self;

	ORBTreeNode *node_to_delete = orbtree_node(impl, pair.self);
	ORBTreeNode *x = NULL;
	ORBTreeNode *w = NULL;
	ORBTreeNode *parent = NULL;
	u32 x_id = NIL;
	u32 w_id = NIL;
	u32 parent_id = NIL;
	bool do_fixup = IS_BLACK(impl, pair.self);

	if (node_to_delete->left == NIL) {
		x_id = node_to_delete->right;
		x = orbtree_node(impl, x_id);
		orbtree_transplant(impl, pair.self, node_to_delete->right);
		ORBTreeNode *node_to_delete_parent = orbtree_node(impl, node_to_delete->parent);
		if (node_to_delete_parent == NULL) {
		} else if (node_to_delete_parent->left == NIL) {
			w_id = node_to_delete_parent->right;
			w = orbtree_node(impl, w_id);
		} else if (node_to_delete_parent) {
			w_id = node_to_delete_parent->left;
			w = orbtree_node(impl, w_id);
		}
		if (x_id != NIL) {
			x = orbtree_node(impl, x_id);
			parent_id = x->parent;
			parent = orbtree_node(impl, parent_id);
		} else if (w_id != NIL) {
			parent_id = w->parent;
			parent = orbtree_node(impl, parent_id);
		}
	} else if (node_to_delete->right == NIL) {
		x_id = node_to_delete->left;
		x = orbtree_node(impl, x_id);
		orbtree_transplant(impl, pair.self, node_to_delete->left);
		ORBTreeNode *node_to_delete_parent = orbtree_node(impl, node_to_delete->parent);
		if (node_to_delete_parent) {
			w_id = node_to_delete_parent->left;
			w = orbtree_node(impl, w_id);
		}
		parent_id = x->parent;
		parent = orbtree_node(impl, parent_id);
	} else {
		u32 successor_id = orbtree_find_successor(impl, pair.self);
		ORBTreeNode *successor = orbtree_node(impl, successor_id);
		do_fixup = IS_BLACK(impl, successor_id);

		x_id = successor->right;
		ORBTreeNode *successor_parent = orbtree_node(impl, successor->parent);
		w_id = successor_parent->right;
		x = orbtree_node(impl, x_id);
		w = orbtree_node(impl, w_id);

		if (w == NULL) {
			parent_id = NIL;
			parent = NULL;
		} else if (w->parent == pair.self) {
			w_id = node_to_delete->left;
			w = orbtree_node(impl, w_id);
			parent_id = successor_id;
			parent = successor;
		} else {
			parent_id = w->parent;
			parent = orbtree_node(impl, w_id);
		}

		if (successor->parent != pair.self) {
			orbtree_transplant(impl, successor_id, successor->right);
			successor->right = node_to_delete->right;
			ORBTreeNode *successor_right = orbtree_node(impl, successor->right);
			successor_right->parent = successor_id;
		}

		// do final transpalnnnt and update including color match
		orbtree_transplant(impl, pair.self, successor_id);
		successor->left = node_to_delete->left;
		ORBTreeNode *successor_left = orbtree_node(impl, successor->left);
		successor_left->parent = successor_id;
		orbtree_set_color_based_on_parent(impl, successor_id, pair.self);
	}

	if (do_fixup) {
		if (w_id != NIL && parent_id != NIL) {
			orbtree_remove_fixup(impl, parent_id, w_id, x_id);
		} else {
			// in these cases SET_BLACK only
			if (impl->elements > 1)
				SET_BLACK(impl, impl->root);
		}
	}

	impl->elements--;

	return 0;
}
int orbtree_get(const ORBTree *ptr, const void *searched, ORBTreeTray *found) {
	// validate input
	if (ptr == NULL || searched == NULL || found == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	// obtain the impl from the ptr
	ORBTreeImpl *impl = ptr->impl;
	// this pair is used to search
	ORBTreeNodePair pair;

	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	// perform search for the key
	orbtree_search(impl, searched, &pair);

	if (pair.self != NIL) {
		found->updated = true;
		found->value = orbtree_value(impl, pair.self);
	} else {
		found->updated = false;
	}

	return 0;
}
i64 orbtree_size(const ORBTree *ptr) {
	if (ptr == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	ORBTreeImpl *impl = ptr->impl;

	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	return impl->elements;
}
int orbtree_iterator(const ORBTree *ptr, ORBTreeIterator *iter, const void *start_value,
					 bool start_inclusive, const void *end_value, bool end_inclusive) {
	return 0;
}
int orbtree_iterator_reset(const ORBTree *ptr, ORBTreeIterator *iter, const void *start_value,
						   bool start_inclusive, const void *end_value, bool end_inclusive) {
	return 0;
}

int orbtree_init_free_list(ORBTreeImpl *impl, u8 *data, u32 offset) {
	u64 cur_index = offset / NODES_PER_CHUNK;
	for (int i = 0; i < NODES_PER_CHUNK; i++) {
		if (i < NODES_PER_CHUNK - 1)
			impl->free_list[i + offset] = offset + i + 1;
		else
			impl->free_list[i + offset] = UINT32_MAX;
	}
	return 0;
}

int orbtree_allocate_tray(ORBTree *ptr, ORBTreeTray *tray) {
	if (ptr == NULL || tray == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	ORBTreeImpl *impl = ptr->impl;
	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}
	if (impl->free_list_head == UINT32_MAX) {
		void *tmp;
		if (((impl->capacity / 100) % CHUNKS_PER_RESIZE) == 0) {
			if (impl->capacity == 0) {
				tmp = mymalloc(sizeof(void *) * CHUNKS_PER_RESIZE);
			} else {
				u64 size = (impl->cur_chunks + CHUNKS_PER_RESIZE) * sizeof(void *);
				tmp = myrealloc(impl->data_chunks, size);
			}

			if (tmp == NULL) {
				return -1;
			}
			impl->cur_chunks += CHUNKS_PER_RESIZE;
			impl->data_chunks = tmp;
		}
		u64 index = impl->size / NODES_PER_CHUNK;
		impl->data_chunks[index] =
			mymalloc(sizeof(u8) * NODES_PER_CHUNK * (impl->value_size + sizeof(ORBTreeNode)));

		if (impl->data_chunks[index] == NULL) {
			if (impl->capacity == 0) {
				impl->cur_chunks = 0;
				impl->data_chunks = NULL;
				myfree(tmp);
			}
			return -1;
		}

		void *free_list = NULL;
		if (impl->capacity == 0) {
			free_list = mymalloc(sizeof(u32) * (impl->cur_chunks + 1) * NODES_PER_CHUNK);
		} else {
			free_list =
				myrealloc(impl->free_list, sizeof(u32) * (impl->cur_chunks + 1) * NODES_PER_CHUNK);
		}
		if (free_list == NULL) {
			if (impl->capacity == 0) {
				impl->cur_chunks = 0;
				impl->data_chunks = NULL;
				myfree(tmp);
				myfree(free_list);
				return -1;
			}
		}

		impl->free_list = free_list;
		orbtree_init_free_list(impl, impl->data_chunks[index], impl->size);
		impl->free_list_head = impl->size;
		impl->capacity = (index + 1) * NODES_PER_CHUNK;
	}

	u32 next = impl->free_list_head;
	if (next == NIL)
		return -1;
	tray->value = orbtree_value(impl, next);
	impl->free_list_head = impl->free_list[next];
	tray->updated = true;
	tray->id = next;
	impl->free_list[next] = UINT32_MAX - 1;
	impl->size++;
	return 0;
}

int orbtree_deallocate_tray(ORBTree *ptr, ORBTreeTray *tray) {
	if (ptr == NULL || tray == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	ORBTreeImpl *impl = ptr->impl;
	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	if (impl->free_list == NULL) {
		panic("free list not initialized!");
	}

	if ((impl->free_list)[tray->id] != (UINT32_MAX - 1)) {
		panic("Potential double free. Id = %llu.", tray->id);
	}

	impl->free_list[tray->id] = impl->free_list_head;
	impl->free_list_head = tray->id;
	impl->size--;

	return 0;
}

#ifdef TEST

void orbtree_validate_node(const ORBTreeImpl *impl, u32 node, int *black_count,
						   int current_black_count) {
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

	ORBTreeNode *n = orbtree_node(impl, node);

	// Increment black count if the current node is black
	if (IS_BLACK(impl, node)) {
		current_black_count++;
	} else {
		//   Check if the node is red
		//   If the parent is red, return false (Red property violation)
		assert(!(n->parent != NIL && IS_RED(impl, n->parent)));
	}

	// Recursive calls for left and right children
	orbtree_validate_node(impl, n->left, black_count, current_black_count);
	orbtree_validate_node(impl, n->right, black_count, current_black_count);
}

void orbtree_validate_impl(const ORBTreeImpl *impl) {
	int black_count = 0;
	// Validate from the root and check if the root is black
	if (impl->root != NIL) {
		assert(IS_BLACK(impl, impl->root));
		orbtree_validate_node(impl, impl->root, &black_count, 0);
	}
}

void orbtree_validate(const ORBTree *ptr) {
	ORBTreeImpl *impl = ptr->impl;
	orbtree_validate_impl(impl);
}

// Function to print a single node with its color
void orbtree_print_node(const ORBTreeImpl *impl, u32 node_id, int depth) {
	if (node_id == NIL) {
		for (int i = 0; i < depth; i++) {
			printf("    ");
		}
		printf("0 (B)\n");
		return;
	}

	ORBTreeNode *node = orbtree_node(impl, node_id);

	// Print the right child first (for visual representation)
	orbtree_print_node(impl, node->right, depth + 1);

	// Indent according to depth
	for (int i = 0; i < depth; i++) {
		printf("    ");
	}

	// Print the current node with a clearer representation
	printf("%llu (%s)\n", node->node_id, (IS_BLACK(impl, node_id)) ? "B" : "R");

	// Print the left child
	orbtree_print_node(impl, node->left, depth + 1);
}

void orbtree_print_impl(ORBTreeImpl *impl) {
	ORBTreeNode *root = orbtree_node(impl, impl->root);
	if (!root)
		printf("Red-Black Tree (root = 0) Empty Tree!\n");
	else {
		printf("Red-Black Tree (root = %llu)\n", root->node_id);
		printf("===================================\n"); // Separator for better clarity
		orbtree_print_node(impl, impl->root, 0);
		printf("===================================\n"); // Separator for better clarity
	}
}

// Function to print the entire tree
void orbtree_print(const ORBTree *ptr) {
	ORBTreeImpl *impl = ptr->impl;
	orbtree_print_impl(impl);
}
#endif // TEST
