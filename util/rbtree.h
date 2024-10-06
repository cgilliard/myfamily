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

#ifndef _UTIL_RBTREE__
#define _UTIL_RBTREE__

#include <base/slabs.h>
#include <base/types.h>

typedef struct RBTreeIteratorNc {
	FatPtr impl;
} RBTreeIteratorNc;

typedef struct RbTreeKeyValue {
	void *key;
	void *value;
} RbTreeKeyValue;

void rbtree_iterator_cleanup(RBTreeIteratorNc *ptr);

#define RBTreeIterator                                                                             \
	RBTreeIteratorNc __attribute__((warn_unused_result, cleanup(rbtree_iterator_cleanup)))

bool rbtree_iterator_next(RBTreeIterator *ptr, RbTreeKeyValue *kv);

typedef struct RBTreeNc {
	FatPtr impl;
} RBTreeNc;

void rbtree_cleanup(RBTreeNc *ptr);

#define RBTree RBTreeNc __attribute__((warn_unused_result, cleanup(rbtree_cleanup)))

int rbtree_build(RBTree *ptr, const u64 key_size, const u64 value_size,
				 int (*compare)(const void *, const void *), bool send);
int rbtree_insert(RBTree *ptr, const void *key, const void *value);
int rbtree_delete(RBTree *ptr, const void *key);
const void *rbtree_get(const RBTree *ptr, const void *key);
i64 rbtree_size(const RBTree *ptr);
int rbtree_iterator(const RBTree *ptr, RBTreeIterator *iter);

#ifdef TEST
bool rbtree_validate(const RBTree *ptr);
int rbtree_max_depth(const RBTree *ptr);
void rbtree_print_debug(const RBTree *ptr);
#endif // TEST

#endif // _UTIL_RBTREE__
