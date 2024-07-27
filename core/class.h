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

#ifndef _CORE_CLASS__
#define _CORE_CLASS__

#include <core/cleanup.h>
#include <core/macro_utils.h>
#include <core/panic.h>
#include <core/resources.h>
#include <core/types.h>
#include <stdio.h>
#include <string.h>

#define UNIQUE_ID __COUNTER__

// global trait table
typedef struct TraitTable {
	char **traits;
	char **implementors;
	u64 count;
} TraitTable;

static TraitTable _global_traits__ = {NULL, NULL, 0};

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
	char *name;
	u64 id;
	bool no_cleanup;
} Vdata;

typedef struct {
	Vdata vdata;
} Object;

void sort_vtable(Vtable *table);
bool implements(void *obj, const char *name);
void *find_fn(Object *obj, const char *name);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void vtable_override(Vtable *table, VtableEntry entry);
void vtable_cleanup(Vtable *table);

#define DEFINE_CLASS(x) x##Ptr Cleanup(x##_cleanup_impl)

#define NEXT_ID __COUNTER__
#define BUILD(name, ...)                                                       \
	{{&name##Ptr_Vtable__, #name, NEXT_ID, false}, false, __VA_ARGS__}
#define BUILDPTR(ptr, cname)                                                   \
	({                                                                     \
		ptr->vdata.vtable = &cname##Ptr_Vtable__;                      \
		ptr->vdata.name = #cname;                                      \
		ptr->vdata.id = NEXT_ID;                                       \
		ptr->vdata.no_cleanup = false;                                 \
	})

#define MEMBER_TYPE(type, member) typeof(((type *)0)->member)

#define FNPTR(name) name;
#define FIELD(field_type, field_name) field_type CAT(_, field_name);

#define GET(type, obj, field_name) *type##_get_##field_name(obj)
#define SET(type, obj, field_name, value) type##_set_##field_name(obj, value)

#define GETTER(name, field_name)                                               \
	MEMBER_TYPE(name, CAT(_, field_name)) *                                \
	    CAT(name##_, CAT(get_, field_name))(name##Ptr * self) {            \
		return &self->CAT(_, field_name);                              \
	}

#define SETTER(name, field_name)                                               \
	void CAT(name##_, CAT(set_, field_name))(                              \
	    name##Ptr * self,                                                  \
	    MEMBER_TYPE(name, CAT(_, field_name)) CAT(_, field_name)) {        \
		self->CAT(_, field_name) = CAT(_, field_name);                 \
	}
#define GETTER_PROTO(name, field_name)                                         \
	MEMBER_TYPE(name, CAT(_, field_name)) *                                \
	    CAT(name##_, CAT(get_, field_name))(name##Ptr * self);

#define SETTER_PROTO(name, field_name)                                         \
	static void CAT(name##_, CAT(set_, field_name))(                       \
	    name##Ptr * self,                                                  \
	    MEMBER_TYPE(name, CAT(_, field_name)) CAT(_, field_name));

#define CLASS(name, ...)                                                       \
	typedef struct name name;                                              \
	typedef struct name##Ptr name##Ptr;                                    \
	typedef struct name##Ptr {                                             \
		Vdata vdata;                                                   \
		bool __dummy_;                                                 \
		__VA_ARGS__                                                    \
	} name##Ptr;                                                           \
	static Vtable name##Ptr_Vtable__ = {0, UNIQUE_ID, NULL};               \
	void name##_cleanup(name##Ptr *obj);                                   \
	static void name##_cleanup_impl(name##Ptr *obj) {                      \
		if (!obj->vdata.no_cleanup)                                    \
			name##_cleanup(obj);                                   \
	}                                                                      \
	static u64 name##_size(name##Ptr *obj) { return sizeof(name##Ptr); }   \
	static void                                                            \
	    __attribute__((constructor)) add_cleanup_##name##_vtable() {       \
		VtableEntry next = {"cleanup", name##_cleanup};                \
		vtable_add_entry(&name##Ptr_Vtable__, next);                   \
		VtableEntry next2 = {"size", name##_size};                     \
		vtable_add_entry(&name##Ptr_Vtable__, next2);                  \
	}                                                                      \
	static char *name##Ptr_vdata_name__ = #name;

#define CLASS_STATIC_CLEANUP(name, ...)                                        \
	typedef struct name name;                                              \
	typedef struct name##Ptr name##Ptr;                                    \
	typedef struct name##Ptr {                                             \
		Vdata vdata;                                                   \
		__VA_ARGS__                                                    \
	} name##Ptr;                                                           \
	static Vtable name##Ptr_Vtable__ = {0, UNIQUE_ID, NULL};               \
	static void name##_cleanup(name##Ptr *obj);                            \
	static void name##_cleanup_impl(name##Ptr *obj) {                      \
		if (!obj->vdata.no_cleanup)                                    \
			name##_cleanup(obj);                                   \
	}                                                                      \
	static u64 name##_size(name##Ptr *obj) { return sizeof(name##Ptr); }   \
	static void                                                            \
	    __attribute__((constructor)) add_cleanup_##name##_vtable() {       \
		VtableEntry next = {"cleanup", name##_cleanup};                \
		vtable_add_entry(&name##Ptr_Vtable__, next);                   \
		VtableEntry next2 = {"size", name##_size};                     \
		vtable_add_entry(&name##Ptr_Vtable__, next2);                  \
	}

#define IMPL(name, trait) EXPAND(trait(name))

#define OVERRIDE(name, trait, implfn)                                          \
	static void                                                            \
	    __attribute__((constructor)) ov__##name##_##trait##_vtable() {     \
		VtableEntry next = {#trait, implfn};                           \
		vtable_override(&name##Ptr_Vtable__, next);                    \
	}

#define TRAIT_SUPER(name, trait) EXPAND(trait(name))

#define TRAIT_IMPL(T, name, default)                                           \
	static void __attribute__((constructor)) add_##name##_##T##_vtable() { \
		VtableEntry next = {#name, default};                           \
		vtable_add_entry(&T##Ptr_Vtable__, next);                      \
	}

#define TRAIT_REQUIRED(T, R, name, ...)                                        \
	R T##_##name(__VA_ARGS__);                                             \
	static void __attribute__((constructor)) CAT(add_##T, UNIQUE_ID)() {   \
		VtableEntry next = {#name, T##_##name};                        \
		vtable_add_entry(&T##Ptr_Vtable__, next);                      \
	}

#define TRAIT_REQUIRED_EXT(T, R, name, ...)                                    \
	R T##_##name(__VA_ARGS__);                                             \
	static void __attribute__((constructor)) CAT(add_##T, UNIQUE_ID)() {   \
		VtableEntry next = {#name, T##_##name};                        \
		vtable_add_entry(&T##_Vtable__, next);                         \
	}

#define CLASS_NAME(x) ((Object *)(x))->vdata.name
#define CLASS_NAME_REF(x) ((Object *)(&x))->vdata.name

#endif // _CORE_CLASS__
