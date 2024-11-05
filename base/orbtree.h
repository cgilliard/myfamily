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

#include <base/macros.h>
#include <base/slabs.h>
#include <base/types.h>

typedef struct OrbTreeNode {
	// ordered tree
	// upper bit indicates if this is a 'box' or 'weak' type and as such data is
	// treated as a pointer.
	unsigned int ord_parent;
	// if upper bit set and upper bit of parent is set, this indicates 'weak'.
	// if upper bit not set and upper bit of parent is set, this indicates
	// 'box'.
	unsigned int ord_right;
	unsigned int ord_left;
	unsigned int ord_right_subtree_size;
	// upper bit for left subtree size is ord_color
	unsigned int ord_left_subtree_size;

	// sequenced tree
	unsigned int seq_parent;
	unsigned int seq_right;
	unsigned int seq_left;
	unsigned int seq_right_subtree_size;
	// upper bit for left subtree size is seq_color
	unsigned int seq_left_subtree_size;

	// namespace
	unsigned int namespace;
	unsigned int seqno;
	// eventually move to our string type for this (both are pointers so 8
	// bytes)
	char *name;

	// if this is a 'box' or 'weak' type as indicated in the upper bit of
	// ord_parent/ord_right, we treat data as pointer. otherwise, the first byte
	// is type indicator (int, byte, bool, float, or weak). Since all types only
	// require 4 bytes or less the remaining 7 bytes. We use the same 'aux'
	// reference counting for types as in the previous object model.
	byte data[8];
} OrbTreeNode;

// Data structure used for searching ORBTrees.
typedef struct OrbTreeNodePair {
	OrbTreeNode *parent;
	OrbTreeNode *self;
	bool is_right;
} OrbTreeNodePair;

Type(OrbTree);
#define OrbTree DefineType(OrbTree)

OrbTree orbtree_create(int (*search)(const OrbTreeNode *root, const void *value,
									 OrbTreeNodePair *retval));

// using the specified search function, find the specified value if found return
// a pointer to its node. Otherwise return NULL.
OrbTreeNode *orbtree_get(const OrbTree *ptr, const void *value);
// insert the specified OrbTreeNode into the tree. If the value replaces another
// value in the tree, the value that was replaced is returned
// Otherwise, NULL is returned. (This is used so that the caller can handle
// memory management operations on the node).
OrbTreeNode *orbtree_put(OrbTree *ptr, OrbTreeNode *value);

// remove the specified value from the tree. If the value is found and thus
// removed, it is returned to the calling function. Otherwise NULL is returned.
OrbTreeNode *orbtree_remove(const OrbTree *ptr, const void *value);

#endif	// _BASE_ORBTREE__
