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
#include <base/print_util.h>
#include <base/slabs.h>

typedef struct OrbTreeNodeImpl {
	Ptr parent;
	Ptr right;
	Ptr left;
	unsigned int right_subtree_height_color;
	unsigned int left_subtree_height;
} OrbTreeNodeImpl;

void __attribute__((constructor)) __orbtree_check_sizes() {
	if (sizeof(OrbTreeNodeImpl) != sizeof(OrbTreeNode))
		panic("sizeof(OrbTreeNodeImpl) (%i) != sizeof(OrbTreeNode) (%i)",
			  sizeof(OrbTreeNodeImpl), sizeof(OrbTreeNode));
}

OrbTreeNode *orbtree_node_right(const OrbTreeNode *tree) {
	return NULL;
}

OrbTreeNode *orbtree_node_left(const OrbTreeNode *tree) {
	return NULL;
}

OrbTree orbtree_create(const SlabAllocator *sa) {
	OrbTree ret = {};
	return ret;
}
OrbTreeNode *orbtree_get(const OrbTree *tree, const OrbTreeNodeWrapper *value,
						 const OrbTreeSearch *search, unsigned int offset) {
	return NULL;
}
OrbTreeNodeWrapper *orbtree_put(OrbTree *tree, const OrbTreeNodeWrapper *value,
								const OrbTreeSearch *search) {
	return NULL;
}

OrbTreeNodeWrapper *orbtree_remove(OrbTree *tree,
								   const OrbTreeNodeWrapper *value,
								   const OrbTreeSearch *search) {
	return NULL;
}
