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

#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/orbtree.h>
#include <base/osdef.h>
#include <base/print_util.h>

#define CACHE_ENTRIES 128
#define MURMUR_SEED 0xF9

const static unsigned int NIL = (UINT32_MAX - 2);

typedef struct OrbTreeNode {
	unsigned int color;
	unsigned int right;
	unsigned int left;
	unsigned int parent;
	unsigned int right_subtree_size;
	unsigned int left_subtree_size;
	unsigned long long namespace;
	unsigned long long seqno;
	Ptr name;
	Ptr value;
} OrbTreeNode;

typedef struct OrbTreeCacheEntry {
	unsigned int hash;
	unsigned int node;
	unsigned int last;
	unsigned int index;
	int64 mutation_id;
} OrbTreeCacheEntry;

typedef struct OrbTreeNodePair {
	unsigned int parent;
	unsigned int self;
	bool is_right;
} OrbTreeNodePair;

typedef struct OrbTreeImpl {
	unsigned int root_name;
	unsigned int root_seq;
	unsigned int elements;
	int64 mutation_id;
	unsigned int cache_entries;
	unsigned int slab_overhead;
	SlabAllocator sa;
	OrbTreeCacheEntry cache[];
} OrbTreeImpl;

OrbTreeNode *orbtree_node(const OrbTreeImpl *impl, unsigned int node) {
	if (node == NIL) {
		return NULL;
	}
	Ptr ptr = ptr_for(impl->sa, node, impl->slab_overhead + sizeof(OrbTreeNode));
	return $(ptr);
}

OrbTree orbtree_create() {
	Ptr ret = fam_alloc(sizeof(OrbTreeImpl) + CACHE_ENTRIES * sizeof(OrbTreeCacheEntry));
	if (ret == NULL)
		return NULL;

	OrbTreeImpl *tree = $(ret);

	tree->sa = slab_allocator_create();

	if (tree->sa == NULL) {
		fam_release(&ret);
		return NULL;
	}

	tree->elements = 0;
	tree->mutation_id = 0;
	tree->cache_entries = CACHE_ENTRIES;
	tree->root_seq = NIL;
	tree->root_name = NIL;
	tree->slab_overhead = slab_overhead();

	return ret;
}

int orbtree_compare_seqno(const OrbTreeImpl *impl, int64 namespace, int64 seqno,
						  unsigned int node_id) {
	OrbTreeNode *node = orbtree_node(impl, node_id);
	if (!node)
		panic("node returned was null!");
	if (namespace != node->namespace) {
		if (namespace < node->namespace) {
			return -1;
		} else {
			return 1;
		}
	} else {
		if (seqno < node->seqno)
			return -1;
		else if (seqno > node->seqno)
			return 1;
		return 0;
	}
}

int obrtree_compare_name(const OrbTreeImpl *impl, int64 namespace, const char *name,
						 unsigned int node_id) {
	OrbTreeNode *node = orbtree_node(impl, node_id);
	if (!node)
		panic("node returned was null!");

	if (namespace != node->namespace) {
		if (namespace < node->namespace) {
			return -1;
		} else {
			return 1;
		}
	} else {
		const char *node_name_value = $(node->name);
		return strcmp(name, node_name_value);
	}
}

void orbtree_search(const OrbTreeImpl *impl, int64 namespace, const char *name, int64 seqno,
					OrbTreeNodePair *nodes) {
	nodes->parent = NIL;
	if (name != NULL)
		nodes->self = impl->root_name;
	else
		nodes->self = impl->root_seq;
	int i = 0;
	while (nodes->self != NIL) {
		nodes->parent = nodes->self;
		int v;
		if (name != NULL)
			v = obrtree_compare_name(impl, namespace, name, nodes->self);
		else
			v = orbtree_compare_seqno(impl, namespace, seqno, nodes->self);
		if (v == 0) {
			break;
		} else if (v < 0) {
			OrbTreeNode *self = orbtree_node(impl, nodes->self);
			nodes->self = self->right;
			nodes->is_right = true;
		} else {
			OrbTreeNode *self = orbtree_node(impl, nodes->self);
			nodes->self = self->left;
			nodes->is_right = false;
		}
	}
}

const void *orbtree_get(const OrbTree tree, int64 namespace, int64 seqno, const char *name,
						int offset) {
	return NULL;
}
unsigned int orbtree_range_size(const OrbTree tree, int64 namespace) {
	return 0;
}
int orbtree_remove(OrbTree tree, int64 namespace, int64 seqno, const char *name, int offset) {
	return 0;
}
int orbtree_put(OrbTree tree, const char *name, int64 namespace, int64 seqno, Ptr value) {
	return 0;
}

// Functions that require override of const
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

void OrbTree_cleanup(const OrbTree *tree) {
	if (initialized(*tree)) {
		OrbTreeImpl *impl = $(*tree);
		slab_allocator_cleanup(&impl->sa);
		fam_release(tree);
	}
}
