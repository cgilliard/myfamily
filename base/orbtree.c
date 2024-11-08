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
#include <base/orbtree.h>
#include <base/print_util.h>
#include <base/slabs.h>

#define RED_NODE 0x80000000

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

_Thread_local const SlabAllocator *tl_slab_allocator = NULL;
_Thread_local const OrbTreeImpl *tl_tree = NULL;
_Thread_local unsigned int tl_offset = 0;

void __attribute__((constructor)) __orbtree_check_sizes() {
	if (sizeof(OrbTreeNodeImpl) != sizeof(OrbTreeNode))
		panic("sizeof(OrbTreeNodeImpl) (%i) != sizeof(OrbTreeNode) (%i)",
			  sizeof(OrbTreeNodeImpl), sizeof(OrbTreeNode));
	if (sizeof(OrbTreeImpl) != sizeof(OrbTree))
		panic("sizeof(OrbTreeImpl) (%i) != sizeof(OrbTree) (%i)",
			  sizeof(OrbTreeImpl), sizeof(OrbTree));
}

void *orbtree_value(Ptr ptr) {
	if (tl_slab_allocator == NULL) panic("no slab allocator specified!");
	return slab_get(tl_slab_allocator, ptr);
}

OrbTreeNode *orbtree_node(Ptr ptr, unsigned int offset) {
	if (tl_slab_allocator == NULL) panic("no slab allocator specified!");
	byte *slab = slab_get(tl_slab_allocator, ptr);
	if (slab == NULL) return NULL;
	return (OrbTreeNode *)(slab + offset);
}

int orbtree_insert(OrbTreeImpl *impl, OrbTreeNodePair *pair,
				   const OrbTreeNodeWrapper *value) {
	if (pair->parent == null) {
		impl->root = pair->self;
		OrbTreeNodeImpl *self = (OrbTreeNodeImpl *)orbtree_node(
			pair->self, value->offset_of_orbtree_node);
		self->right = null;
		self->left = null;
		self->parent = null;
		self->right_subtree_height_color = 0;
		self->left_subtree_height = 0;
	} else {
		OrbTreeNodeImpl *self = (OrbTreeNodeImpl *)orbtree_node(
			pair->self, value->offset_of_orbtree_node);
		if (self == NULL) {
			OrbTreeNodeImpl *parent = (OrbTreeNodeImpl *)orbtree_node(
				pair->parent, value->offset_of_orbtree_node);
			if (pair->is_right)
				parent->right = value->ptr;
			else
				parent->left = value->ptr;
			self = (OrbTreeNodeImpl *)orbtree_node(
				value->ptr, value->offset_of_orbtree_node);
			self->parent = pair->parent;
			self->right = null;
			self->left = null;
			self->right_subtree_height_color = RED_NODE;
			self->left_subtree_height = 0;
		} else {
			// println("self not null update");
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

Ptr orbtree_node_ptr(const OrbTreeNode *node, bool is_right) {
	const OrbTreeNodeImpl *impl = (const OrbTreeNodeImpl *)node;
	if (impl->parent == null) {
		return tl_tree->root;
	} else {
		OrbTreeNodeImpl *node =
			(OrbTreeNodeImpl *)orbtree_node(impl->parent, tl_offset);
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
	tl_slab_allocator = impl->sa;
	tl_tree = impl;
	tl_offset = value->offset_of_orbtree_node;
}

void *orbtree_get(const OrbTree *tree, const OrbTreeNodeWrapper *value,
				  OrbTreeSearch search, unsigned int offset) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	if (impl->root == null) return NULL;
	orbtree_set_tl_context(impl, value);
	OrbTreeNode *root = orbtree_node(impl->root, value->offset_of_orbtree_node);
	OrbTreeNode *target =
		orbtree_node(value->ptr, value->offset_of_orbtree_node);
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

	OrbTreeNode *target =
		orbtree_node(value->ptr, value->offset_of_orbtree_node);

	OrbTreeNodePair pair = {.parent = null, .self = value->ptr};
	if (impl->root != null) {
		OrbTreeNode *root =
			orbtree_node(impl->root, value->offset_of_orbtree_node);
		search(root, target, &pair);
	}

	orbtree_insert(impl, &pair, value);

	return NULL;
}

void *orbtree_remove(OrbTree *tree, const OrbTreeNodeWrapper *value,
					 OrbTreeSearch search) {
	OrbTreeImpl *impl = (OrbTreeImpl *)tree;
	orbtree_set_tl_context(impl, value);
	return NULL;
}
