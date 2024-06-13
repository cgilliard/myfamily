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

bool equal(void *obj1, void *obj2) {
	if (((Object *)obj1)->vdata.vtable->id !=
	    ((Object *)obj2)->vdata.vtable->id) {
		return false;
	}
	bool (*do_equal)(Object *obj1, Object *obj2) =
	    find_fn((Object *)obj1, "equal");

	if (do_equal == NULL)
		panic("equal not implemented for this type");
	return do_equal(obj1, obj2);
}

void *unwrap(void *obj) {
	void *(*do_unwrap)(Object *obj) = find_fn((Object *)obj, "unwrap");
	if (do_unwrap == NULL)
		panic("unwrap not implemented for this type");
	return do_unwrap(obj);
}

char *to_str(void *obj) {
	char *(*do_to_str)(Object *obj) = find_fn((Object *)obj, "to_str");
	if (do_to_str == NULL)
		panic("to_str not implemented for this type");
	return do_to_str(obj);
}

void *unwrap_err(void *obj) {
	void *(*do_unwrap)(Object *obj) = find_fn((Object *)obj, "unwrap_err");
	if (do_unwrap == NULL)
		panic("unwrap_err not implemented for this type");
	return do_unwrap(obj);
}

bool copy(void *dst, void *src) {
	bool *(*do_copy)(Object *dst, Object *src) =
	    find_fn((Object *)dst, "copy");
	if (do_copy == NULL)
		panic("copy not implemented for this type");
	return do_copy(dst, src);
}

size_t size(void *obj) {
	size_t (*do_size)(Object *obj) = find_fn((Object *)obj, "size");
	if (do_size == NULL)
		panic("size not implemented for this type");
	return do_size(obj);
}

void cleanup(void *ptr) {
	bool (*do_cleanup)(Object *ptr) = find_fn((Object *)ptr, "cleanup");

	if (do_cleanup != NULL)
		do_cleanup(ptr);
}
