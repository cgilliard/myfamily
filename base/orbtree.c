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

OrbTree orbtree_create(int (*search)(const OrbTreeNode *root, const void *value,
									 OrbTreeNodePair *retval)) {
	return NULL;
}

OrbTreeNode *orbtree_get(const OrbTree *ptr, const void *value) {
	return NULL;
}
OrbTreeNode *orbtree_put(OrbTree *ptr, OrbTreeNode *value) {
	return NULL;
}

OrbTreeNode *orbtree_remove(const OrbTree *ptr, const void *value) {
	return NULL;
}
