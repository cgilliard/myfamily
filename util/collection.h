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

#ifndef _COLLECTIONS_UTIL__
#define _COLLECTIONS_UTIL__

#include <base/ser.h>
#include <util/slabs.h>

enum CollectionConfigType {
	CollectionConfigTypeSlabAllocator = 0,
	CollectionConfigTypeMaxEntries = 1,
	CollectionConfigTypeEntryArrayLen = 2,
	CollectionConfigTypeSlabSize = 3,
};
typedef enum CollectionConfigType CollectionConfigType;

struct CollectionConfig {
	CollectionConfigType type;
	void *ptr;
};
typedef struct CollectionConfig CollectionConfigImpl;

int collection_config_slab_allocator(CollectionConfigImpl *c, SlabAllocator *s);
int collection_config_max_entries(CollectionConfigImpl *c, u64 value);
int collection_config_array_len(CollectionConfigImpl *c, u64 value);
int collection_config_slab_size(CollectionConfigImpl *c, u64 value);
int collection_config_free(CollectionConfigImpl *c);
#define CollectionConfig CollectionConfigImpl CLEANUP(collection_config_free)

enum CollectionType {
	CollectionTypeHashtable = 1,
	CollectionTypeHashset = 2,
	CollectionTypeList = 3,
};
typedef enum CollectionType CollectionType;

struct Collection {
	CollectionType type;
};
typedef struct Collection Collection;

struct Iterator {
	Collection *ptr;
};
typedef struct Iterator Iterator;

int hashtable_init(Collection *collection);
int hashset_init(Collection *collection);
int list_init(Collection *collection);

int hashtable_insert(Collection *collection, Serializable *k, Serializable *v);
int hashset_insert(Collection *collection, Serializable *k);
int hashtable_get(Collection *collection, Serializable *k, Serializable *v);
int hashset_contains(Collection *collection, Serializable *k);
int list_push(Collection *collection, Serializable *v);
int collection_clear(Collection *collection);
u64 collection_len(Collection *collection);
int collection_iter(Collection *collection, Iterator *iter);
int collection_iter_rev(Collection *collection, Iterator *iter);

#endif // _COLLECTIONS_UTIL__
