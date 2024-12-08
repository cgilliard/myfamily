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

#include <base/hash.h>
#include <base/print_util.h>
#include <base/sys.h>
#include <base/util.h>

typedef struct HashtableImpl {
	HashEntry **arr;
	u32 entry_size;
	u32 array_size;
} HashtableImpl;

#define SLOT(x) (HashEntry **)((byte *)impl->arr + x * sizeof(void *))

int hash_init(Hashtable *h, u32 array_size, u32 entry_size) {
	HashtableImpl *impl = (HashtableImpl *)h;
	u64 pages = 1 + (((array_size * sizeof(void *)) - 1) / PAGE_SIZE);
	impl->arr = map(pages);
	if (impl->arr == NULL) return -1;
	impl->array_size = array_size;
	impl->entry_size = entry_size;
	return 0;
}

void hash_cleanup(Hashtable *h) {
	HashtableImpl *impl = (HashtableImpl *)h;
	u64 pages = 1 + (((impl->array_size * sizeof(void *)) - 1) / PAGE_SIZE);
	if (impl->arr) unmap(impl->arr, pages);
}

void hash_put(Hashtable *h, HashEntry *v, hash_fn hfn, hash_eq heq) {
	v->next = NULL;
	HashtableImpl *impl = (HashtableImpl *)h;
	u32 x = hfn(v->data) % impl->array_size;
	HashEntry **slot = SLOT(x), *dst = *slot;
	if (dst) {
		for (HashEntry *ent = *slot; ent && ent->key_len; ent = ent->next)
			dst = ent;
		dst->next = v;
	} else
		impl->arr[x] = v;
}
HashEntry *hash_get(Hashtable *h, const void *k, hash_fn hfn, hash_eq heq) {
	HashtableImpl *impl = (HashtableImpl *)h;
	u32 x = hfn(k) % ((HashtableImpl *)h)->array_size;
	HashEntry **slot = (HashEntry **)SLOT(x);
	if (*slot && heq(k, (*slot)->data)) return *slot;
	for (HashEntry *ent = *slot; ent; ent = ent->next)
		if (heq(k, ent->data)) return ent;
	return NULL;
}
HashEntry *hash_rem(Hashtable *h, const void *k, hash_fn hfn, hash_eq heq) {
	HashtableImpl *impl = (HashtableImpl *)h;
	u32 x = hfn(k) % ((HashtableImpl *)h)->array_size;
	HashEntry **slot = (HashEntry **)SLOT(x);
	if (*slot && heq(k, (*slot)->data)) {
		HashEntry *ret = *slot;
		*slot = (*slot)->next;
		return ret;
	}
	HashEntry *prev = *slot;
	for (HashEntry *ent = *slot; ent; ent = ent->next) {
		if (heq(k, ent->data)) {
			prev->next = ent->next;
			return ent;
		}
		prev = ent;
	}
	return NULL;
}
