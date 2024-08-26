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

#include <core/chain_allocator.h>
#include <core/heap.h>
#include <core/macro_utils.h>
#include <core/types.h>

#define VDATA_FLAGS_NO_CLEANUP (0x1 << 0)

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct {
	char *name;
	u64 len;
	VtableEntry *entries;
} Vtable;

typedef struct Object {
	Vtable *vtable;
	u64 id;
	u8 flags;
	FatPtr ptr;
} Object;

typedef struct TraitBound {
	char *name;
	char *binding;
} TraitBound;

typedef struct TraitRequired {
	char *name;
	TraitBound *bounds;
	u64 bounds_count;
} TraitRequired;

typedef struct TraitDefn {
	char *name;
	TraitRequired *entries;
	u64 entry_count;
} TraitDefn;

typedef struct TraitTable {
	TraitDefn *entries;
	u64 entry_count;
} TraitTable;

extern TraitTable __global_trait_table;

void Object_cleanup(const Object *ptr);
void sort_vtable(Vtable *table);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void *find_fn(const Object *obj, const char *name);
u64 unique_id();
FatPtr build_fat_ptr(u64 size);

#define Cleanup                                                                \
	Object __attribute__((warn_unused_result, cleanup(Object_cleanup)))

#define TypeName(obj) obj.vtable->name

#define var Cleanup
#define let const Cleanup

#define Type(name, ...)                                                        \
	typedef struct name name;                                              \
	static Vtable name##_Vtable__ = {#name, 0, NULL};                      \
	u64 name##_size();                                                     \
	typedef struct name {                                                  \
		__VA_ARGS__                                                    \
	} name;                                                                \
	u64 name##_size() { return sizeof(name); }                             \
	static void __attribute__((constructor))                               \
	__add_impls_##name##_vtable() {                                        \
		VtableEntry size = {"size", name##_size};                      \
		vtable_add_entry(&name##_Vtable__, size);                      \
	}

#define Field(field_type, field_name) field_type field_name;
#define With(x, y) .x = y

// TODO: _fptr__.data may be NULL, need to handle. Once we have 'Result'
// implement two versions of this macro. One which panics and one which returns
// error. Also, implement a trait 'validate' which is called here to validate
// the parameter settings.
// clang-format off
#define new(name, ...)({                                                          \
		name _type__ = {__VA_ARGS__};                                  \
		FatPtr _fptr__ = build_fat_ptr(name##_size());                 \
		*((name *)_fptr__.data) = _type__;                             \
		Object _ret__ = {&name##_Vtable__, unique_id(), 0, _fptr__};   \
		_ret__;                                                        \
	})
// clang-format on

#define get(type, obj, field_name) *type##_get_##field_name(&obj)
#define set(type, obj, field_name, value) type##_set_##field_name(&obj, value)

#define MEMBER_TYPE(type, member) typeof(((type *)0)->member)

#define Getter(name, field_name)                                               \
	MEMBER_TYPE(name, field_name) *                                        \
	    name##_get_##field_name(const Object *self) {                      \
		name *tptr = fat_ptr_data(&self->ptr);                         \
		return &tptr->field_name;                                      \
	}

#define Setter(name, field_name)                                               \
	void name##_set_##field_name(                                          \
	    Object *self, MEMBER_TYPE(name, field_name) field_name) {          \
		name *tptr = fat_ptr_data(&self->ptr);                         \
		tptr->field_name = field_name;                                 \
	}

#define GetterProto(name, field_name)                                          \
	MEMBER_TYPE(name, field_name) *                                        \
	    name##_get_##field_name(const Object *self);

#define SetterProto(name, field_name)                                          \
	void name##_set_##field_name(                                          \
	    Object *self, MEMBER_TYPE(name, field_name) field_name);

#define Impl(name, trait) EXPAND(trait(name))

#define OVERRIDE(name, trait, implfn)                                          \
	static void __attribute__((constructor))                               \
	ov__##name##_##trait##_vtable() {                                      \
		VtableEntry next = {#trait, implfn};                           \
		vtable_override(&name##_Vtable__, next);                       \
	}

#define TRAIT_SUPER(name, trait) EXPAND(trait(name))

#define TRAIT_IMPL(T, name, default)                                           \
	static void __attribute__((constructor)) add_##name##_##T##_vtable() { \
		VtableEntry next = {#name, default};                           \
		vtable_add_entry(&T##_Vtable__, next);                         \
	}

#define Required(T, R, name, ...)                                              \
	R T##_##name(__VA_ARGS__);                                             \
	static void __attribute__((constructor)) CAT(add_##T, UNIQUE_ID)() {   \
		VtableEntry next = {#name, T##_##name};                        \
		vtable_add_entry(&T##_Vtable__, next);                         \
	}

/* BoundsCheck b1 = {"dst", dst}; */
/* BoundsCheck b2 = {"src", src}; */ /* check_bounds("Clone",
					"clone",
					b1,
					b2);
				      */

// Define macros to extract the type and name from a tuple
#define EXTRACT_TYPE_NAME(type, name) type name
#define EXTRACT_NAME(type, name) name

// Define the macros for unwrapping the tuples
#define UNWRAP_Param_TYPE_NAME(type_name) EXTRACT_TYPE_NAME type_name
#define UNWRAP_Param_NAME(type_name) EXTRACT_NAME type_name

// Define PROC_FN_SIGNATURE and PROC_ParamS to use UNWRAP_Param correctly
#define PROC_FN_SIGNATURE(...) FOR_EACH(UNWRAP_Param_TYPE_NAME, __VA_ARGS__)
#define PROC_ParamS(...) FOR_EACH(UNWRAP_Param_NAME, __VA_ARGS__)

// Define a parameter macro for testing
#define Param(type, name) (type, name)

#define TraitImpl(return_type, name, ...)                                      \
	static return_type name(PROC_FN_SIGNATURE(__VA_ARGS__)) {              \
		if (!self)                                                     \
			panic(                                                 \
			    "Runtime error: Illegal argument! self was NULL"); \
		return_type (*impl)(PROC_FN_SIGNATURE(__VA_ARGS__)) =          \
		    find_fn(self, #name);                                      \
                                                                               \
		if (!impl)                                                     \
			panic("Runtime error: Trait bound violation! Type "    \
			      "'%s' does "                                     \
			      "not implement the "                             \
			      "required function [%s]l",                       \
			      TypeName((*self)));                              \
                                                                               \
		return impl(PROC_ParamS(__VA_ARGS__));                         \
	}

#endif // _CORE_TYPE__
