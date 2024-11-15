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

#include <base/types.h>

#define INIT_ORBTREE \
	{}
#define PARENT(node) \
	((OrbTreeNode *)((unsigned long long)node->parent_color & ~0x1))

typedef struct OrbTreeNode {
	struct OrbTreeNode *parent_color;
	struct OrbTreeNode *right;
	struct OrbTreeNode *left;
} OrbTreeNode;

typedef struct OrbTreeNodePair {
	OrbTreeNode *parent;
	OrbTreeNode *self;
	bool is_right;
} OrbTreeNodePair;

typedef struct OrbTree {
	OrbTreeNode *root;
} OrbTree;

typedef int (*OrbTreeSearch)(OrbTreeNode *base, const OrbTreeNode *value,
							 OrbTreeNodePair *retval);

OrbTreeNode *orbtree_put(OrbTree *tree, OrbTreeNode *value,
						 const OrbTreeSearch search);
OrbTreeNode *orbtree_remove(OrbTree *tree, OrbTreeNode *value,
							const OrbTreeSearch search);

#endif	// _BASE_ORBTREE__
