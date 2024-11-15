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

typedef struct OrbTreeNode {
	OrbTreeNode *parent_color;
	OrbTreeNode *right;
	OrbTreeNode *left;
} OrbTreeNode;

typedef struct OrbTreeNodePair {
	OrbTreeNode *parent;
	OrbTreeNode *self;
	bool is_right;
} OrbTreeNodePair;

#define ORB_TREE_IMPL_SIZE 24
typedef struct OrbTree {
	byte impl[ORB_TREE_IMPL_SIZE];
} OrbTree;

typedef int (*OrbTreeSearch)(const OrbTreeNode *base, const OrbTreeNode *value,
							 OrbTreeNodePair *retval);

int orbtree_init(OrbTree *tree, const SlabAllocator *sa);
void *orbtree_get(const OrbTree *tree, const void *value, unsigned int offsetof,
				  OrbTreeSearch search);
void *orbtree_put(OrbTree *tree, OrbTreeNode *value, unsigned int offsetof,
				  const OrbTreeSearch search);
void *orbtree_remove(OrbTree *tree, OrbTreeNode *value, unsigned int offsetof,
					 const OrbTreeSearch search);

#ifdef TEST
void *orbtree_root(const OrbTree *tree);
#endif	// TEST

#endif	// _BASE_ORBTREE__
