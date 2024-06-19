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

#ifndef _CLASS_BASE__
#define _CLASS_BASE__

#define _GNU_SOURCE
#include <base/cleanup.h>
#include <base/macro_utils.h>
#include <base/panic.h>
#include <base/tlmalloc.h>
#include <stdio.h>
#include <string.h>

#define UNIQUE_ID __COUNTER__

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
} Vdata;

typedef struct {
	Vdata vdata;
} Object;

void sort_vtable(Vtable *table);
bool implements(Object *obj, const char *name);
void *find_fn(Object *obj, const char *name);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void vtable_override(Vtable *table, VtableEntry entry);
void vtable_cleanup(Vtable *table);

#define DEFINE_CLASS(x) x##Ptr Cleanup(x##_cleanup)

#define BUILD(name, ...) {{&name##Vtable, #name}, __VA_ARGS__}

#define MEMBER_TYPE(type, member) typeof(((type *)0)->member)

#define FNPTR(name) name;
#define FIELD(field_type, field_name) field_type CAT(_, field_name);
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
	typedef struct name##Ptr {                                             \
		Vdata vdata;                                                   \
		__VA_ARGS__                                                    \
	} name##Ptr;                                                           \
	static Vtable name##Vtable = {0, UNIQUE_ID, NULL};                     \
	void name##_cleanup(name##Ptr *obj);                                   \
	static void                                                            \
	    __attribute__((constructor)) add_cleanup_##name##_vtable() {       \
		char *str;                                                     \
		asprintf(&str, "cleanup");                                     \
		VtableEntry next = {str, name##_cleanup};                      \
		vtable_add_entry(&name##Vtable, next);                         \
	}

#define IMPL(name, trait) EXPAND(trait(name))

#define OVERRIDE(name, trait, implfn)                                          \
	static void                                                            \
	    __attribute__((constructor)) ov__##name##_##trait##_vtable() {     \
		char *str;                                                     \
		asprintf(&str, "%s", #trait);                                  \
		VtableEntry next = {str, implfn};                              \
		vtable_override(&name##Vtable, next);                          \
	}

#define TRAIT_SUPER(name, trait) EXPAND(trait(name))

#define TRAIT_IMPL(T, name, default)                                           \
	static void __attribute__((constructor)) add_##name##_##T##_vtable() { \
		char *str;                                                     \
		asprintf(&str, "%s", #name);                                   \
		VtableEntry next = {str, default};                             \
		vtable_add_entry(&T##Vtable, next);                            \
	}

#define TRAIT_REQUIRED(T, R, name, ...)                                        \
	R T##_##name(__VA_ARGS__);                                             \
	static void __attribute__((constructor)) CAT(add_##T, UNIQUE_ID)() {   \
		char *str;                                                     \
		asprintf(&str, "%s", #name);                                   \
		VtableEntry next = {str, T##_##name};                          \
		vtable_add_entry(&T##Vtable, next);                            \
	}

#define CLASS_NAME(x) ((Object *)(x))->vdata.name
#define CLASS_NAME_REF(x) ((Object *)(&x))->vdata.name

#endif //_CLASS_BASE__
