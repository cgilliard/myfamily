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

#include <base/panic.h>
#include <base/resources.h>
#include <stdio.h>
#include <util/orbtree.h>

#define NODES_PER_CHUNK 100
#define CHUNKS_PER_RESIZE 10

typedef struct ORBTreeNode {
	u32 right;
	u32 left;
	u32 parent;
	u32 right_seqno;
	u32 left_seqno;
	u32 parent_seqno;
	u32 right_subtree_size;
	u32 left_subtree_size;
	u64 seqno_and_color;
	char data[];
} ORBTreeNode;

// all these could be true except seqno_and_color cannot be 0. Even the first entry would be 2^32.
const static ORBTreeNode NIL_DEFN = {.right = 0,
									 .left = 0,
									 .parent = 0,
									 .right_seqno = 0,
									 .left_seqno = 0,
									 .parent_seqno = 0,
									 .right_subtree_size = 0,
									 .left_subtree_size = 0,
									 .seqno_and_color = 0};
#if defined(__clang__)
// Clang-specific pragma
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__) && !defined(__clang__)
// GCC-specific pragma
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#warning "Unknown compiler or platform. No specific warning pragmas applied."
#endif
static ORBTreeNode *NIL = &NIL_DEFN;
#pragma GCC diagnostic pop

// The internal ORBTreeImpl storage data structure
typedef struct ORBTreeImpl {
	u64 value_size;								// size of the values
	int (*compare)(const void *, const void *); // a comparion function (like qsort)
	ORBTreeNode *root;							// pointer to the root node.
	u32 size;									// current size of the tree.
	u32 capacity;								// current capacity of the tree.
	u32 free_list_head;
	u8 **data_chunks;
	u32 cur_chunks;
	u32 *free_list;
} ORBTreeImpl;

void orbtree_iterator_cleanup(ORBTreeIteratorNc *ptr) {
}

bool orbtree_iterator_next(ORBTreeIterator *ptr, ORBTreeTray *value) {
	return false;
}

void orbtree_cleanup(ORBTreeNc *ptr) {
	if (ptr->impl) {
		ORBTreeImpl *impl = ptr->impl;
		if (impl->data_chunks) {
			for (u32 i = 0; i < impl->capacity; i += NODES_PER_CHUNK) {
				myfree(impl->data_chunks[i / NODES_PER_CHUNK]);
			}
			myfree(impl->data_chunks);
			impl->data_chunks = NULL;
		}

		if (impl->free_list) {
			myfree(impl->free_list);
			impl->free_list = NULL;
		}
		myfree(ptr->impl);
		ptr->impl = NULL;
	}
}

int orbtree_create(ORBTree *ptr, const u64 value_size, int (*compare)(const void *, const void *)) {
	// validate input
	if (ptr == NULL || value_size == 0 || compare == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	ptr->impl = mymalloc(sizeof(ORBTreeImpl));
	if (ptr->impl == NULL) {
		return -1;
	}
	ORBTreeImpl *impl = ptr->impl;
	impl->value_size = value_size;
	impl->compare = compare;
	impl->root = NIL;
	impl->size = 0;
	impl->capacity = 0;
	impl->free_list_head = UINT32_MAX;
	impl->data_chunks = NULL;
	impl->cur_chunks = 0;
	impl->free_list = NULL;

	return 0;
}
int orbtree_put(ORBTree *ptr, ORBTreeTray *value, ORBTreeTray *replaced) {
	return 0;
}
int orbtree_remove(ORBTree *ptr, const void *value, ORBTreeTray *removed) {
	return 0;
}
int orbtree_get(const ORBTree *ptr, const void *searched, ORBTreeTray *found) {
	return 0;
}
i64 orbtree_size(const ORBTree *ptr) {
	return 0;
}
int orbtree_iterator(const ORBTree *ptr, ORBTreeIterator *iter, const void *start_value,
					 bool start_inclusive, const void *end_value, bool end_inclusive) {
	return 0;
}
int orbtree_iterator_reset(const ORBTree *ptr, ORBTreeIterator *iter, const void *start_value,
						   bool start_inclusive, const void *end_value, bool end_inclusive) {
	return 0;
}

int orbtree_init_free_list(ORBTreeImpl *impl, u8 *data, u32 offset) {
	u64 cur_index = offset / NODES_PER_CHUNK;
	for (int i = 0; i < NODES_PER_CHUNK; i++) {
		if (i < NODES_PER_CHUNK - 1)
			impl->free_list[i + offset] = offset + i + 1;
		else
			impl->free_list[i + offset] = UINT32_MAX;
	}
	return 0;
}

int orbtree_allocate_tray(ORBTree *ptr, ORBTreeTray *tray) {
	if (ptr == NULL || tray == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	ORBTreeImpl *impl = ptr->impl;
	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}
	if (impl->free_list_head == UINT32_MAX) {
		void *tmp;
		if (((impl->capacity / 100) % CHUNKS_PER_RESIZE) == 0) {
			if (impl->capacity == 0) {
				tmp = mymalloc(sizeof(void *) * CHUNKS_PER_RESIZE);
			} else {
				u64 size = (impl->cur_chunks + CHUNKS_PER_RESIZE) * sizeof(void *);
				tmp = myrealloc(impl->data_chunks, size);
			}

			if (tmp == NULL) {
				return -1;
			}
			impl->cur_chunks += CHUNKS_PER_RESIZE;
			impl->data_chunks = tmp;
		}
		u64 index = impl->size / NODES_PER_CHUNK;
		impl->data_chunks[index] = mymalloc(sizeof(u8) * NODES_PER_CHUNK *
											(impl->value_size + ORBTREE_NODE_RESERVED_SIZE));

		if (impl->data_chunks[index] == NULL) {
			if (impl->capacity == 0) {
				impl->cur_chunks = 0;
				impl->data_chunks = NULL;
				myfree(tmp);
			}
			return -1;
		}

		void *free_list = NULL;
		if (impl->capacity == 0) {
			free_list = mymalloc(sizeof(u32) * (impl->cur_chunks + 1) * NODES_PER_CHUNK);
		} else {
			free_list =
				myrealloc(impl->free_list, sizeof(u32) * (impl->cur_chunks + 1) * NODES_PER_CHUNK);
		}
		if (free_list == NULL) {
			if (impl->capacity == 0) {
				impl->cur_chunks = 0;
				impl->data_chunks = NULL;
				myfree(tmp);
				myfree(free_list);
				return -1;
			}
		}

		impl->free_list = free_list;
		orbtree_init_free_list(impl, impl->data_chunks[index], impl->size);
		impl->free_list_head = impl->size;
		impl->capacity = (index + 1) * NODES_PER_CHUNK;
	}

	u32 next = impl->free_list_head;
	tray->value =
		impl->data_chunks[next / NODES_PER_CHUNK] + ((next % NODES_PER_CHUNK) * impl->value_size);
	impl->free_list_head = impl->free_list[next];
	tray->updated = true;
	tray->id = next;
	impl->free_list[next] = UINT32_MAX - 1;
	impl->size++;
	return 0;
}

int orbtree_deallocate_tray(ORBTree *ptr, ORBTreeTray *tray) {
	if (ptr == NULL || tray == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	ORBTreeImpl *impl = ptr->impl;
	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	if (impl->free_list == NULL) {
		panic("free list not initialized!");
	}

	if ((impl->free_list)[tray->id] != (UINT32_MAX - 1)) {
		panic("Potential double free. Id = %llu.", tray->id);
	}

	impl->free_list[tray->id] = impl->free_list_head;
	impl->free_list_head = tray->id;
	impl->size--;

	return 0;
}
