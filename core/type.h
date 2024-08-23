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

#ifndef _CORE_TYPE__
#define _CORE_TYPE__

#include <core/heap.h>
#include <core/macro_utils.h>
#include <core/mymalloc.h>
#include <core/types.h>

#define VDATA_FLAGS_NO_CLEANUP (0x1 << 0)

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct {
	u64 len;
	VtableEntry *entries;
} Vtable;

typedef struct ObjectPtr {
	Vtable *vtable;
	u64 id;
	u8 flags;
	FatPtr ptr;
} ObjectPtr;

void ObjectPtr_cleanup(const ObjectPtr *ptr);
void sort_vtable(Vtable *table);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void *find_fn(const ObjectPtr *obj, const char *name);
u64 unique_id();

#define Cleanup                                                                \
	ObjectPtr                                                              \
	    __attribute__((warn_unused_result, cleanup(ObjectPtr_cleanup)))

#define var Cleanup
#define let const Cleanup
#define MutRef Cleanup *
#define Ref const Cleanup *

#define TYPE_DATA(name, ...)                                                   \
	typedef struct name {                                                  \
		__VA_ARGS__                                                    \
	} name;                                                                \
	u64 name##_size() { return sizeof(name); }                             \
	void __attribute__((constructor)) __add_impls_##name##_vtable() {      \
		VtableEntry next = {"cleanup", name##_cleanup};                \
		vtable_add_entry(&name##_Vtable__, next);                      \
		VtableEntry next2 = {"size", name##_size};                     \
		vtable_add_entry(&name##_Vtable__, next2);                     \
	}

#define TYPE(name)                                                             \
	typedef struct name name;                                              \
	static Vtable name##_Vtable__ = {0, NULL};                             \
	u64 name##_size();                                                     \
	void name##_cleanup(name *obj);

#define FIELD(field_type, field_name) field_type field_name;

static FatPtr build_fat_ptr(u64 size) {
	FatPtr ret;
	// mymalloc(&ret, size);
	return ret;
}

#define BUILD(type)                                                            \
	{ &type##_Vtable__, unique_id(), 0, build_fat_ptr(type##_size()) }

#endif // _CORE_TYPE__
