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

#include <core/class.h>
#include <core/panic.h>
#include <core/resources.h>
#include <stdlib.h>
#include <string.h>

void vtable_cleanup(Vtable *table) { myfree(table->entries); }

int compare_vtable_entry(const void *ent1, const void *ent2) {
	const VtableEntry *vtent1 = ent1;
	const VtableEntry *vtent2 = ent2;
	return strcmp(vtent1->name, vtent2->name);
}

void sort_vtable(Vtable *table) {
	qsort(table->entries, table->len, sizeof(VtableEntry),
	      compare_vtable_entry);
}

bool implements(void *obj, const char *name) {
	return find_fn((Object *)obj, name) != NULL;
}

void *find_fn(const Object *obj, const char *name) {
	int left = 0;
	int right = obj->vdata.vtable->len - 1;
	while (left <= right) {
		int mid = left + (right - left) / 2;
		int cmp = strcmp(name, obj->vdata.vtable->entries[mid].name);

		if (cmp == 0) {
			return obj->vdata.vtable->entries[mid].fn_ptr;
		} else if (cmp < 0) {
			right = mid - 1;
		} else {
			left = mid + 1;
		}
	}
	return NULL;
}

void vtable_add_entry(Vtable *table, VtableEntry entry) {
	if (table->entries == NULL) {
		table->entries =
		    mymalloc(sizeof(VtableEntry) * (table->len + 1));
		if (table->entries == NULL)
			panic("Couldn't allocate memory for vtable");
	} else {
		table->entries = myrealloc(
		    table->entries, sizeof(VtableEntry) * (table->len + 1));
		if (table->entries == NULL)
			panic("Couldn't allocate memory for vtable");
	}

	memcpy(&table->entries[table->len], &entry, sizeof(VtableEntry));
	table->len += 1;
	sort_vtable(table);
}

void vtable_override(Vtable *table, VtableEntry entry) {
	for (int i = 0; i < table->len; i++) {
		if (!strcmp(entry.name, table->entries[i].name)) {
			table->entries[i].fn_ptr = entry.fn_ptr;
		}
	}
}
