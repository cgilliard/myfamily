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

#include <core/panic.h>
#include <core/type.h>
#include <core/types.h>
#include <stdlib.h>
#include <string.h>

u64 __global_counter__ = 0;

u64 unique_id() { return __global_counter__++; }

// need to access the const mutably here for cleanup only
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored                                                 \
    "-Wincompatible-pointer-types-discards-qualifiers"
void ObjectPtr_cleanup(Ref ptr) {
	ObjectPtr *unconst = ptr;
	if ((unconst->flags & VDATA_FLAGS_NO_CLEANUP) == 0) {
		void (*do_cleanup)(ObjectPtr * ptr) = find_fn(ptr, "cleanup");
		if (do_cleanup)
			do_cleanup(ptr);
		if (fat_ptr_data(&unconst->ptr)) {
			chain_free(&unconst->ptr);
		}
	}
}
#pragma GCC diagnostic pop // re-enable

FatPtr build_fat_ptr(u64 size) {
	FatPtr ret;
	chain_malloc(&ret, size);
	return ret;
}

int compare_vtable_entry(const void *ent1, const void *ent2) {
	const VtableEntry *vtent1 = ent1;
	const VtableEntry *vtent2 = ent2;
	return strcmp(vtent1->name, vtent2->name);
}

void sort_vtable(Vtable *table) {
	qsort(table->entries, table->len, sizeof(VtableEntry),
	      compare_vtable_entry);
}

void vtable_add_entry(Vtable *table, VtableEntry entry) {
	if (table->entries == NULL) {
		table->entries = malloc(sizeof(VtableEntry) * (table->len + 1));
		if (table->entries == NULL)
			panic("Couldn't allocate memory for vtable");
	} else {
		void *tmp = realloc(table->entries,
				    sizeof(VtableEntry) * (table->len + 1));
		if (tmp == NULL)
			panic("Couldn't allocate memory for vtable");
		table->entries = tmp;
	}

	memcpy(&table->entries[table->len], &entry, sizeof(VtableEntry));
	table->len += 1;
	sort_vtable(table);
}

void *find_fn(Ref obj, const char *name) {
	int left = 0;
	int right = obj->vtable->len - 1;
	while (left <= right) {
		int mid = left + (right - left) / 2;
		int cmp = strcmp(name, obj->vtable->entries[mid].name);

		if (cmp == 0) {
			return obj->vtable->entries[mid].fn_ptr;
		} else if (cmp < 0) {
			right = mid - 1;
		} else {
			left = mid + 1;
		}
	}
	return NULL;
}
