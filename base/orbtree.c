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

#include <base/orbtree.h>
#include <base/util.h>

#define RED 1
#define BLACK 0
#define SET_NODE(node, color, parent, rightv, leftv)             \
	({                                                           \
		node->parent_color =                                     \
			(OrbTreeNode *)((unsigned long long)parent | color); \
		node->right = rightv;                                    \
		node->left = leftv;                                      \
	})
#define IS_RED(node) (node && ((unsigned long long)node->parent_color & 0x1))
#define IS_BLACK(node) !IS_RED(node)
#define SET_BLACK(node)   \
	(node->parent_color = \
		 (OrbTreeNode *)((unsigned long long)node->parent_color & ~RED))
#define SET_RED(node)     \
	(node->parent_color = \
		 (OrbTreeNode *)((unsigned long long)node->parent_color | RED))
#define SET_RIGHT(node, rightv) node->right = rightv
#define SET_LEFT(node, leftv) node->left = leftv
#define SET_PARENT(node, parentv)                      \
	(node->parent_color =                              \
		 (OrbTreeNode *)((unsigned long long)parentv + \
						 ((unsigned long long)node->parent_color & 0x1)))
#define PARENT(node) \
	((OrbTreeNode *)((unsigned long long)node->parent_color & ~0x1))
#define RIGHT(node) node->right
#define LEFT(node) node->left
#define SET_ROOT(tree, node) tree->root = node
#define ROOT(tree) (tree->root)
#define IS_ROOT(tree, value) (tree->root == value)

void orbtree_rotate_right(OrbTree *tree, OrbTreeNode *x) {
	OrbTreeNode *y = LEFT(x);
	SET_LEFT(x, RIGHT(y));
	if (RIGHT(y)) SET_PARENT(RIGHT(y), x);
	SET_PARENT(y, PARENT(x));
	if (!PARENT(x))
		SET_ROOT(tree, y);
	else if (x == RIGHT(PARENT(x)))
		SET_RIGHT(PARENT(x), y);
	else
		SET_LEFT(PARENT(x), y);
	SET_RIGHT(y, x);
	SET_PARENT(x, y);
}

void orbtree_rotate_left(OrbTree *tree, OrbTreeNode *x) {
	OrbTreeNode *y = RIGHT(x);
	SET_RIGHT(x, LEFT(y));
	if (LEFT(y)) SET_PARENT(LEFT(y), x);
	SET_PARENT(y, PARENT(x));
	if (!PARENT(x))
		SET_ROOT(tree, y);
	else if (x == LEFT(PARENT(x)))
		SET_LEFT(PARENT(x), y);
	else
		SET_RIGHT(PARENT(x), y);
	SET_LEFT(y, x);
	SET_PARENT(x, y);
}

void orbtree_insert_fixup(OrbTree *tree, OrbTreeNode *k) {
	OrbTreeNode *parent, *gparent, *uncle;
	while (!IS_ROOT(tree, k) && IS_RED(PARENT(k))) {
		parent = PARENT(k);
		gparent = PARENT(parent);
		if (parent == LEFT(gparent)) {
			uncle = RIGHT(gparent);
			if (IS_RED(uncle)) {
				SET_BLACK(parent);
				SET_BLACK(uncle);
				SET_RED(gparent);
				k = gparent;
			} else {
				if (k == RIGHT(parent)) {
					k = PARENT(k);
					orbtree_rotate_left(tree, k);
				}
				parent = PARENT(k);
				gparent = PARENT(parent);
				SET_BLACK(parent);
				SET_RED(gparent);
				orbtree_rotate_right(tree, gparent);
			}
		} else {
			uncle = LEFT(gparent);
			if (IS_RED(uncle)) {
				SET_BLACK(parent);
				SET_BLACK(uncle);
				SET_RED(gparent);
				k = gparent;
			} else {
				if (k == LEFT(parent)) {
					k = PARENT(k);
					orbtree_rotate_right(tree, k);
				}
				parent = PARENT(k);
				gparent = PARENT(parent);
				SET_BLACK(parent);
				SET_RED(gparent);
				orbtree_rotate_left(tree, gparent);
			}
		}
	}
	SET_BLACK(ROOT(tree));
}

void orbtree_insert_transplant(OrbTreeNode *prev, OrbTreeNode *next,
							   int is_right) {
	copy_bytes((unsigned char *)next, (unsigned char *)prev,
			   sizeof(OrbTreeNode));
	OrbTreeNode *parent = PARENT(next);
	if (parent != 0) {
		if (is_right)
			SET_RIGHT(parent, next);
		else
			SET_LEFT(parent, next);
	}
	if (LEFT(next)) SET_PARENT(LEFT(next), next);
	if (RIGHT(next)) SET_PARENT(RIGHT(next), next);
}

OrbTreeNode *orbtree_insert(OrbTree *tree, OrbTreeNodePair *pair,
							OrbTreeNode *value) {
	OrbTreeNode *ret = 0;
	if (pair->self) {
		orbtree_insert_transplant(pair->self, value, pair->is_right);
		if (IS_ROOT(tree, pair->self)) SET_ROOT(tree, value);
		ret = pair->self;
	} else {
		if (pair->parent == 0) {
			SET_ROOT(tree, value);
			tree->root->parent_color = 0;
			tree->root->right = tree->root->left = 0;
		} else {
			SET_NODE(value, RED, pair->parent, 0, 0);
			if (pair->is_right)
				SET_RIGHT(pair->parent, value);
			else
				SET_LEFT(pair->parent, value);
		}
	}
	return ret;
}

OrbTreeNode *orbtree_find_successor(OrbTreeNode *x) {
	x = RIGHT(x);
	while (1) {
		if (!LEFT(x)) return x;
		x = LEFT(x);
	}
}

void orbtree_remove_transplant(OrbTree *tree, OrbTreeNode *dst,
							   OrbTreeNode *src) {
	if (PARENT(dst) == 0)
		SET_ROOT(tree, src);
	else if (dst == LEFT(PARENT(dst)))
		SET_LEFT(PARENT(dst), src);
	else
		SET_RIGHT(PARENT(dst), src);
	if (src) SET_PARENT(src, PARENT(dst));
}

void orbtree_set_color(OrbTreeNode *child, OrbTreeNode *parent) {
	if (IS_RED(parent))
		SET_RED(child);
	else
		SET_BLACK(child);
}

void orbtree_remove_fixup(OrbTree *tree, OrbTreeNode *p, OrbTreeNode *w,
						  OrbTreeNode *x) {
	while (!IS_ROOT(tree, x) && IS_BLACK(x)) {
		if (w == RIGHT(p)) {
			if (IS_RED(w)) {
				SET_BLACK(w);
				SET_RED(p);
				orbtree_rotate_left(tree, p);
				w = RIGHT(p);
			}
			if (IS_BLACK(LEFT(w)) && IS_BLACK(RIGHT(w))) {
				SET_RED(w);
				x = p;
				if ((p = PARENT(p))) {
					OrbTreeNode *pl = LEFT(p);
					if (x == pl)
						w = RIGHT(p);
					else
						w = pl;
				} else
					w = 0;
			} else {
				if (IS_BLACK(RIGHT(w))) {
					SET_BLACK(LEFT(w));
					SET_RED(w);
					orbtree_rotate_right(tree, w);
					w = RIGHT(p);
				}
				orbtree_set_color(w, p);
				SET_BLACK(p);
				SET_BLACK(RIGHT(w));
				orbtree_rotate_left(tree, p);
				x = ROOT(tree);
			}
		} else {
			if (IS_RED(w)) {
				SET_BLACK(w);
				SET_RED(p);
				orbtree_rotate_right(tree, p);
				w = LEFT(p);
			}
			if (IS_BLACK(RIGHT(w)) && IS_BLACK(LEFT(w))) {
				SET_RED(w);
				x = p;
				if ((p = PARENT(p))) {
					OrbTreeNode *pl = LEFT(p);
					if (x == pl)
						w = RIGHT(p);
					else
						w = pl;
				} else
					w = 0;
			} else {
				if (IS_BLACK(LEFT(w))) {
					SET_BLACK(RIGHT(w));
					SET_RED(w);
					orbtree_rotate_left(tree, w);
					w = LEFT(p);
				}
				orbtree_set_color(w, p);
				SET_BLACK(p);
				SET_BLACK(LEFT(w));
				orbtree_rotate_right(tree, p);
				x = ROOT(tree);
			}
		}
	}

	SET_BLACK(x);
}

void orbtree_remove_impl(OrbTree *tree, OrbTreeNodePair *pair,
						 OrbTreeNode *value) {
	OrbTreeNode *node_to_delete = pair->self;
	int do_fixup = IS_BLACK(node_to_delete);
	OrbTreeNode *x = 0, *w = 0, *p = 0;

	if (LEFT(node_to_delete) == 0) {
		x = RIGHT(node_to_delete);
		orbtree_remove_transplant(tree, node_to_delete, x);
		p = PARENT(node_to_delete);
		if (p) {
			if (p->left == 0)
				w = RIGHT(p);
			else if (p)
				w = LEFT(p);
		} else {
			do_fixup = 0;
			if (ROOT(tree) != 0) SET_BLACK(ROOT(tree));
		}
	} else if (RIGHT(node_to_delete) == 0) {
		x = LEFT(node_to_delete);
		orbtree_remove_transplant(tree, node_to_delete, LEFT(node_to_delete));
		p = PARENT(node_to_delete);
		w = LEFT(p);
	} else {
		OrbTreeNode *successor = orbtree_find_successor(node_to_delete);
		do_fixup = IS_BLACK(successor);
		x = RIGHT(successor);
		w = RIGHT(PARENT(successor));
		if (w) {
			if (PARENT(w) == node_to_delete) {
				w = LEFT(node_to_delete);
				p = successor;
			} else {
				p = PARENT(w);
			}
		}

		if (PARENT(successor) != node_to_delete) {
			orbtree_remove_transplant(tree, successor, RIGHT(successor));
			SET_RIGHT(successor, RIGHT(node_to_delete));
			if (RIGHT(successor)) SET_PARENT(RIGHT(successor), successor);
		}

		orbtree_remove_transplant(tree, node_to_delete, successor);
		SET_LEFT(successor, LEFT(node_to_delete));
		SET_PARENT(LEFT(successor), successor);
		orbtree_set_color(successor, node_to_delete);
	}

	if (do_fixup) orbtree_remove_fixup(tree, p, w, x);
}

OrbTreeNode *orbtree_put(OrbTree *tree, OrbTreeNode *value,
						 const OrbTreeSearch search) {
	OrbTreeNodePair pair = {};
	if (search(ROOT(tree), value, &pair)) return 0;
	OrbTreeNode *ret = orbtree_insert(tree, &pair, value);
	if (!ret) orbtree_insert_fixup(tree, value);
	return ret;
}
OrbTreeNode *orbtree_remove(OrbTree *tree, OrbTreeNode *value,
							const OrbTreeSearch search) {
	OrbTreeNodePair pair = {};
	if (search(ROOT(tree), value, &pair)) return 0;
	if (pair.self) orbtree_remove_impl(tree, &pair, value);
	return pair.self;
}
