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

#ifndef _BASE_HASH__
#define _BASE_HASH__

#include <base/types.h>

typedef __int128_t Hashtable;

typedef struct HashEntry {
	struct HashEntry *next;
	u32 key_len;
	u32 value_len;
	byte data[];
} HashEntry;

typedef u32 (*hash_fn)(const void *);
typedef bool (*hash_eq)(const void *v1, const void *v2);

int hash_init(Hashtable *h, u32 array_size, u32 entry_size);
void hash_cleanup(Hashtable *h);
void hash_put(Hashtable *h, HashEntry *v, hash_fn, hash_eq);
HashEntry *hash_get(Hashtable *h, const void *key, hash_fn, hash_eq);
HashEntry *hash_rem(Hashtable *h, const void *key, hash_fn, hash_eq);

#endif	// _BASE_HASH__
