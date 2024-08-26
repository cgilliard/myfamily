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
	u64 len;
	VtableEntry *entries;
} Vtable;

typedef struct Object {
	Vtable *vtable;
	u64 id;
	u8 flags;
	FatPtr ptr;
} Object;

void Object_cleanup(const Object *ptr);
void sort_vtable(Vtable *table);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void *find_fn(const Object *obj, const char *name);
u64 unique_id();
FatPtr build_fat_ptr(u64 size);

#define Cleanup                                                                \
	Object __attribute__((warn_unused_result, cleanup(Object_cleanup)))

#define var Cleanup
#define let const Cleanup
#define MutRef Cleanup *
#define Ref const Cleanup *

#define PROC_SETTER(x, y) x abc y
#define PROC_FORMAT(x, y) x y
#define BUILD_SETTERS(x, ...) PROC_SETTER __VA_ARGS__
#define BUILD_ARGS(...) PROC_FORMAT __VA_ARGS__;

#define Type(name, ...)                                                        \
	typedef struct name name;                                              \
	static Vtable name##_Vtable__ = {0, NULL};                             \
	u64 name##_size();                                                     \
	void name##_cleanup(name *obj);                                        \
	typedef struct name {                                                  \
		FOR_EACH(BUILD_ARGS, __VA_ARGS__)                              \
	} name;                                                                \
	u64 name##_size() { return sizeof(name); }                             \
	void __attribute__((constructor)) __add_impls_##name##_vtable() {      \
		VtableEntry cleanup = {"cleanup", name##_cleanup};             \
		vtable_add_entry(&name##_Vtable__, cleanup);                   \
		VtableEntry size = {"size", name##_size};                      \
		vtable_add_entry(&name##_Vtable__, size);                      \
	}

// FOR_EACH(BUILD_SETTERS, __VA_ARGS__)

#define xSetable(field_type, field_name) (v##field_type, v##field_name)
#define Setable(field_type, field_name) (field_type, field_name)
#define Getable(field_type, field_name) (field_type, field_name)
#define Field(field_type, field_name) (field_type, field_name)

// TODO: _fptr__.data may be NULL, need to handle. Once we have 'Result'
// implement two versions of this macro. One which panics and one which returns
// error.
// clang-format off
#define new(name, ...)({                                                          \
		FatPtr _fptr__ = build_fat_ptr(name##_size());                 \
		name _type__ = {__VA_ARGS__};                                  \
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

#endif // _CORE_TYPE__
