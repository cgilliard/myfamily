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

struct Collection {
	
};
typedef struct Collection Collection;

struct Iterator {

};
typedef struct Iterator Iterator;

int hashtable_init(Collection *collection);
int hashset_init(Collection *collection);
int list_init(Collection *collection);

int collection_insert(Collection *collection, int num, ...);
int hashtable_get(Collection *collection, Serializable *s);
int hashset_contains(Collection *collection, Serializable *s);
int list_push(Collection *collection, Serializable *s);
int collection_clear(Collection *collection);
u64 collection_len(Collection *collection);
int collection_iter(Collection *collection, Iterator *iter);
int collection_iter_rev(Collection *collection, Iterator *iter);

#endif // _COLLECTIONS_UTIL__
