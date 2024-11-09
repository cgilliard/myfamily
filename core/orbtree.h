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
	// parent of the node
	Ptr parent;
	// the node
	Ptr self;
	// whether the node is the right node of its parent
	bool is_right;
} OrbTreeNodePair;

typedef struct OrbTreeNodeWrapper {
	Ptr ptr;
	unsigned int offsetof;
} OrbTreeNodeWrapper;

#define ORB_TREE_IMPL_SIZE 16
typedef struct OrbTree {
	byte impl[ORB_TREE_IMPL_SIZE];
} OrbTree;

// OrbTree user defined search - provide a function to search from a given base
// node to the specified value. The function is responsible for populating the
// 'retval' field with the appropriate values for the search.
typedef int (*OrbTreeSearch)(const OrbTreeNode *base, const OrbTreeNode *value,
							 OrbTreeNodePair *retval);

// create an empty orbtree with the specified slab allocator (for lookup of Ptr
// only)
int orbtree_init(OrbTree *tree, const SlabAllocator *sa);
// get the specified value in the OrbTree. The implementation returns a void *
// to the container of the final 'self' returned by the search function. If
// offset is > 0, the appropriate offset will be applied by the orbtree
// implementation.
Ptr orbtree_get(const OrbTree *tree, const OrbTreeNodeWrapper *value,
				OrbTreeSearch search, unsigned int offset);
// inserts a node into the the tree overwriting an existing node with the same
// value. if a node is overwritten it is returned by the function in Ptr form,
// otherwise returns null. If there are more than 2^31 entries inserted into a
// tree at any given time, behaviour is undefined.
Ptr orbtree_put(OrbTree *tree, const OrbTreeNodeWrapper *value,
				const OrbTreeSearch search);
// removes a node from the tree, if it is removed, a pointer to the removed node
// is returned.
Ptr orbtree_remove(OrbTree *tree, const OrbTreeNodeWrapper *value,
				   const OrbTreeSearch search);

#ifdef TEST
Ptr orbtree_root(const OrbTree *tree);
void *orbtree_node_parent(const OrbTreeNode *node);
bool orbtree_node_is_red(const OrbTreeNode *node);
unsigned int orbtree_node_right_subtree_height(const OrbTreeNode *node);
unsigned int orbtree_node_left_subtree_height(const OrbTreeNode *node);
#endif	// TEST

#endif	// _BASE_ORBTREE__
