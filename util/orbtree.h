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

#ifndef _UTIL_ORBTREE__
#define _UTIL_ORBTREE__

#include <base/fam_err.h>
#include <base/slabs.h>

typedef struct ORBTreeIteratorNc {
	FatPtr impl;
} ORBTreeIteratorNc;

// the value and a boolean indicating whether or not the value was updated.
// this value is set by orbtree_put, orbtree_delete, and orbtree_get to indicate
// when a value was found and thus the ORBTreeTray was updated.
typedef struct ORBTreeTray {
	void *value;
	bool updated;
	u32 id;
} ORBTreeTray;

void orbtree_iterator_cleanup(ORBTreeIteratorNc *ptr);

#define ORBTreeIterator                                                                            \
	ORBTreeIteratorNc __attribute__((warn_unused_result, cleanup(orbtree_iterator_cleanup)))

bool orbtree_iterator_next(ORBTreeIterator *ptr, ORBTreeTray *value);

typedef struct ORBTreeNc {
	void *impl;
} ORBTreeNc;

void orbtree_cleanup(ORBTreeNc *ptr);

#define ORBTree ORBTreeNc __attribute__((warn_unused_result, cleanup(orbtree_cleanup)))

int orbtree_create(ORBTree *ptr, const u64 value_size, int (*compare)(const void *, const void *));

// get the value that matches 'value' and return it in 'tray'.
int orbtree_get(const ORBTree *ptr, const void *value, ORBTreeTray *tray);
// Put the tray into the tree. If this value replaces an existing value return the
int orbtree_put(ORBTree *ptr, const ORBTreeTray *value, ORBTreeTray *replaced);
// Remove the specified value. If removed, the removed tray will be populated with the previous
// value.
int orbtree_remove(ORBTree *ptr, const void *value, ORBTreeTray *removed);

// search for the value at the specified index, storing the value in 'tray' if found.
int orbtree_get_index(const ORBTree *ptr, u32 index, ORBTreeTray *tray);
// remove the value at the specified index, storing the value in 'removed' if removed.
int orbtree_remove_index(ORBTree *ptr, u32 index, ORBTreeTray *removed);

// deallocate a tray. To be called after a deletion so the caller can do its own deallocations
// before finally deallocating the tray itself.
int orbtree_deallocate_tray(ORBTree *ptr, ORBTreeTray *tray);
// allocate a tray for insertion. This tray may be used for insertion. If not needed it may also be
// freed with orbtree_deallocate.
int orbtree_allocate_tray(ORBTree *ptr, ORBTreeTray *tray);

i64 orbtree_size(const ORBTree *ptr);

int orbtree_iterator(const ORBTree *ptr, ORBTreeIterator *iter, const void *start_value,
					 bool start_inclusive, const void *end_value, bool end_inclusive);
int orbtree_iterator_reset(const ORBTree *ptr, ORBTreeIterator *iter, const void *start_value,
						   bool start_inclusive, const void *end_value, bool end_inclusive);

#ifdef TEST
void orbtree_print(const ORBTree *ptr);
void orbtree_validate(const ORBTree *ptr);
#endif // TEST

static const ORBTreeNc _nil_orbtree__ = {.impl = NULL};

#define INIT_ORBTREE _nil_rbtree__
#define ORBTreeIsNil(tree) (!nil(tree.impl))

#endif // _UTIL_ORBTREE__
