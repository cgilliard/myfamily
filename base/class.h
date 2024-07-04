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

#ifndef _BASE_CLASS__
#define _BASE_CLASS__

#include <base/macro_helpers.h>
#include <base/types.h>
#include <stdalign.h>

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct {
	char *name;
	u128 id;
	u64 entries_len;
	VtableEntry *entries;
	u64 traits_len;
	char **traits;
} Vtable;

typedef struct {
	Vtable *vtable;
} Vdata;

typedef struct {
	Vdata vdata;
	alignas(sizeof(Vdata)) unsigned char *buf;
} Object;

typedef struct {
	Vdata vdata;
	alignas(sizeof(Vdata)) unsigned char buf[20];
} Object1;

void sort_vtable(Vtable *table);
bool implements(void *obj, const char *name);
void *find_fn(Object *obj, const char *name);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void vtable_override(Vtable *table, VtableEntry entry);
void vtable_cleanup(Vtable *table);

#define UNIQUE_ID __COUNTER__
#define FIELD(field_name) #field_name,
#define TYPE(type) +SIZE_OF(type)
#define OFFSET(type) SIZE_OF(type),

#define CLASS(name, names, offsets, types)                                     \
	typedef struct name name;                                              \
	typedef struct name##Ptr name##Ptr;                                    \
	static char *name##Ptr_Field_Names[] = {names};                        \
	static int name##Ptr_offsets[] = {offsets};                            \
	typedef struct name##Ptr {                                             \
		Vdata vdata;                                                   \
		char buf[0 types];                                             \
	} name##Ptr;                                                           \
	static Vtable name##Ptr_Vtable__ = {                                   \
	    #name, UNIQUE_ID, 0, NULL, 0, NULL};                               \
	void name##_cleanup(name##Ptr *obj);                                   \
	static usize name##_size(name##Ptr *obj) { return sizeof(name##Ptr); } \
	static void                                                            \
	    __attribute__((constructor)) add_cleanup_##name##_vtable() {       \
		VtableEntry next = {"cleanup", name##_cleanup};                \
		vtable_add_entry(&name##Ptr_Vtable__, next);                   \
		VtableEntry next2 = {"size", name##_size};                     \
		vtable_add_entry(&name##Ptr_Vtable__, next2);                  \
	}                                                                      \
	static char *name##Ptr_vdata_name__ = #name;

#endif // _BASE_CLASS__
