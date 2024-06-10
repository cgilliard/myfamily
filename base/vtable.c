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
#include <base/tlmalloc.h>
#include <base/vtable.h>
#include <string.h>

void *find_fn(Object *obj, const char *trait) {
	for (int i = 0; i < obj->vtable->len; i++) {
		if (!strcmp(trait, obj->vtable->entries[i].name)) {
			return obj->vtable->entries[i].fn_ptr;
		}
	}
	return NULL;
}

bool equal(void *obj1, void *obj2) {
	if (((Object *)obj1)->vtable != ((Object *)obj2)->vtable) {
		return false;
	}
	int (*equal)(Object *obj1, Object *obj2) =
	    find_fn((Object *)obj1, "equal");

	if (equal == NULL)
		panic("equal not implemented for this type");
	return equal(obj1, obj2);
}

