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

#ifndef _VTABLE_BASE__
#define _VTABLE_BASE__

#define UNIQUE_ID __COUNTER__

#include <base/types.h>

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct {
	u64 len;
	u128 id;
	VtableEntry *entries;
} Vtable;

typedef struct {
	Vtable *vtable;
} Object;

#define EMPTY_VTABLE {0, UNIQUE_ID, NULL}

void *find_fn(Object *obj, const char *name);
void vtable_add_entry(Vtable *table, VtableEntry entry);

#define DEFINE_VTABLE(name, entries)                                           \
	static Vtable name = EMPTY_VTABLE;                                     \
	static void __init_##name() {                                          \
		int count = sizeof(entries) / sizeof(VtableEntry);             \
		if (name.len == 0) {                                           \
			for (int i = 0; i < count; i++) {                      \
				vtable_add_entry(&name, entries[i]);           \
			}                                                      \
		}                                                              \
	}                                                                      \
	void __attribute__((constructor)) __init_##name();

#define VTABLE(name)                                                           \
	static Vtable name##Vtable = EMPTY_VTABLE;                             \
	static void __init_vtable_##name() {                                   \
		if (name##Vtable.len == 0) {                                   \
			for (int i = 0; i < name##VtableEntriesSize; i++) {    \
				vtable_add_entry(&name##Vtable,                \
						 name##VtableEntries[i]);      \
			}                                                      \
		}                                                              \
	}                                                                      \
	void __attribute__((constructor)) __init_vtable_##name();

// common fns
bool equal(void *obj1, void *obj2);
void *unwrap(void *obj);
void *unwrap_err(void *obj);
size_t size(void *obj);
bool copy(void *dest, void *src);
void cleanup(void *ptr);
char *to_str(void *s);

#endif // _VTABLE_BASE__
