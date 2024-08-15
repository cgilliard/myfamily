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

#include <core/macro_utils.h>
#include <core/types.h>
#include <stdio.h>
#include <string.h>

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
void *find_fn(const Object *obj, const char *name);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void vtable_override(Vtable *table, VtableEntry entry);
void vtable_cleanup(Vtable *table);

#define DEFINE_CLASS(x)                                                        \
	x##Ptr __attribute__((warn_unused_result, cleanup(x##_cleanup_impl)))

#define NEXT_ID __COUNTER__
#define NO_CLEANUP(obj) ((Object *)&obj)->vdata.no_cleanup = true
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
	    CAT(name##_, CAT(get_, field_name))(const void *self) {            \
		return &((name##Ptr *)self)->CAT(_, field_name);               \
	}

#define SETTER(name, field_name)                                               \
	void CAT(name##_, CAT(set_, field_name))(                              \
	    name##Ptr * self,                                                  \
	    MEMBER_TYPE(name, CAT(_, field_name)) CAT(_, field_name)) {        \
		self->CAT(_, field_name) = CAT(_, field_name);                 \
	}
#define GETTER_PROTO(name, field_name)                                         \
	MEMBER_TYPE(name, CAT(_, field_name)) *                                \
	    CAT(name##_, CAT(get_, field_name))(const void *self);

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
	static u64 name##_mysize(name##Ptr *obj) { return sizeof(name##Ptr); } \
	static void                                                            \
	    __attribute__((constructor)) add_cleanup_##name##_vtable() {       \
		VtableEntry next = {"cleanup", name##_cleanup};                \
		vtable_add_entry(&name##Ptr_Vtable__, next);                   \
		VtableEntry next2 = {"mysize", name##_mysize};                 \
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
	static u64 name##_mysize(name##Ptr *obj) { return sizeof(name##Ptr); } \
	static void                                                            \
	    __attribute__((constructor)) add_cleanup_##name##_vtable() {       \
		VtableEntry next = {"cleanup", name##_cleanup};                \
		vtable_add_entry(&name##Ptr_Vtable__, next);                   \
		VtableEntry next2 = {"mysize", name##_mysize};                 \
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

#define CLASS_NAME(x) ((Object *)(x))->vdata.name

#define GENERIC(type, g) (type##Ptr, g##Ptr)

#define EXTRACT_GENERIC_TYPE(type, g) type
#define EXTRACT_GENERIC_G(type, g) #g

#define UNWRAP_GENERIC_TYPE(type_g) EXTRACT_GENERIC_TYPE type_g
#define UNWRAP_GENERIC_G(type_g) EXTRACT_GENERIC_G type_g

// Define macros to extract the type and name from a tuple
#define EXTRACT_TYPE_NAME(type, name) type name
#define EXTRACT_NAME(type, name) name

// Define the macros for unwrapping the tuples
#define UNWRAP_FN_PARAM_TYPE_NAME(type_name) EXTRACT_TYPE_NAME type_name
#define UNWRAP_FN_PARAM_NAME(type_name) EXTRACT_NAME type_name

// Define PROC_FN_SIGNATURE and PROC_FN_PARAMS to use UNWRAP_FN_PARAM correctly
#define PROC_FN_SIGNATURE(...) FOR_EACH(UNWRAP_FN_PARAM_TYPE_NAME, __VA_ARGS__)
#define PROC_FN_PARAMS(...) FOR_EACH(UNWRAP_FN_PARAM_NAME, __VA_ARGS__)

// Define a parameter macro for testing
#define FN_PARAM(type, name) (type, name)

#define FUNCTION(rtype, name, ...)                                             \
	UNWRAP_GENERIC_TYPE(rtype)                                             \
	name##_helper(PROC_FN_SIGNATURE(__VA_ARGS__));                         \
	UNWRAP_GENERIC_TYPE(rtype)                                             \
	name(PROC_FN_SIGNATURE(__VA_ARGS__)) {                                 \
		printf("calling helper2\n");                                   \
		UNWRAP_GENERIC_TYPE(rtype)                                     \
		ret = name##_helper(PROC_FN_PARAMS(__VA_ARGS__));              \
		char *cn = CLASS_NAME(                                         \
		    ((Rc *)((Rc *)ret.slab.data)->_ref.data)->_ref.data);      \
		char cnbuf[strlen(cn) + 5];                                    \
		strcpy(cnbuf, cn);                                             \
		strcat(cnbuf, "Ptr");                                          \
		char *exp_type = UNWRAP_GENERIC_G(rtype);                      \
		if (strcmp(cnbuf, exp_type))                                   \
			panic("Expected function %s to return type an Enum "   \
			      "containing type %s, but it returned type %s.",  \
			      #name, exp_type, cn);                            \
		return ret;                                                    \
	}                                                                      \
	UNWRAP_GENERIC_TYPE(rtype) name##_helper(PROC_FN_SIGNATURE(__VA_ARGS__))

#endif // _CORE_CLASS__
