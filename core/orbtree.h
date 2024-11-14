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

#ifndef _BASE_ORBTREE__
#define _BASE_ORBTREE__

#include <base/slabs.h>
#include <base/types.h>

#define ORB_TREE_NODE_IMPL_SIZE 20
typedef struct OrbTreeNode {
	byte impl[ORB_TREE_NODE_IMPL_SIZE];
} OrbTreeNode;

void *orbtree_node_right(const OrbTreeNode *);
void *orbtree_node_left(const OrbTreeNode *);
Ptr orbtree_node_ptr(const OrbTreeNode *, bool is_right);

typedef struct OrbTreeNodePair {
	Ptr parent;
	Ptr self;
	bool is_right;
} OrbTreeNodePair;

typedef struct OrbTreeNodeSearchWrapper {
	void *ptr;
	unsigned int offsetof;
} OrbTreeNodeSearchWrapper;

typedef struct OrbTreeNodeWrapper {
	Ptr ptr;
	unsigned int offsetof;
} OrbTreeNodeWrapper;

#define ORB_TREE_IMPL_SIZE 24
typedef struct OrbTree {
	byte impl[ORB_TREE_IMPL_SIZE];
} OrbTree;

typedef int (*OrbTreeSearch)(const OrbTreeNode *base, const OrbTreeNode *value,
							 OrbTreeNodePair *retval);

int orbtree_init(OrbTree *tree, const SlabAllocator *sa);
Ptr orbtree_get(const OrbTree *tree, const void *value, unsigned int offsetof,
				OrbTreeSearch search, int offset);
Ptr orbtree_put(OrbTree *tree, Ptr ptr, unsigned int offsetof,
				const OrbTreeSearch search);
Ptr orbtree_remove(OrbTree *tree, const void *value, unsigned int offsetof,
				   const OrbTreeSearch search);

#ifdef TEST
Ptr orbtree_root(const OrbTree *tree);
void *orbtree_node_parent(const OrbTreeNode *node);
bool orbtree_node_is_red(const OrbTreeNode *node);
unsigned int orbtree_node_right_subtree_height(const OrbTreeNode *node);
unsigned int orbtree_node_left_subtree_height(const OrbTreeNode *node);
#endif	// TEST

#endif	// _BASE_ORBTREE__
