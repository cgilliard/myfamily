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

#include <base/class.h>
#include <base/panic.h>
#include <base/tlmalloc.h>
#include <stdlib.h>
#include <string.h>

void *find_fn(Object *obj, const char *name) {
	for (int i = 0; i < obj->vdata.vtable->len; i++) {
		if (!strcmp(name, obj->vdata.vtable->entries[i].name)) {
			return obj->vdata.vtable->entries[i].fn_ptr;
		}
	}
	return NULL;
}

void vtable_add_entry(Vtable *table, VtableEntry entry) {
	if (table->entries == NULL) {
		table->entries =
		    tlmalloc(sizeof(VtableEntry) * (table->len + 1));
		if (table->entries == NULL)
			panic("Couldn't allocate memory for vtable");
	} else {
		table->entries = tlrealloc(
		    table->entries, sizeof(VtableEntry) * (table->len + 1));
		if (table->entries == NULL)
			panic("Couldn't allocate memory for vtable");
	}

	memcpy(&table->entries[table->len], &entry, sizeof(VtableEntry));
	table->len += 1;
}

void vtable_override(Vtable *table, VtableEntry entry) {
	for (int i = 0; i < table->len; i++) {
		if (!strcmp(entry.name, table->entries[i].name)) {
			table->entries[i].fn_ptr = entry.fn_ptr;
		}
	}
}
