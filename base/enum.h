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

#ifndef _BASE_ENUM__
#define _BASE_ENUM__

#include <base/class.h>
#include <base/rc.h>

#define ENUM(name, variants, types)                                            \
	typedef enum { variants } name##_Variants__;                           \
	static char *name##_Arr__[] = {types};                                 \
	CLASS_STATIC_CLEANUP(name, name##_Variants__ type; void *value;        \
			     bool is_prim;)                                    \
	static inline void name##_cleanup(name##Ptr *ptr) {                    \
		if (!ptr->is_prim) {                                           \
			cleanup(ptr->value);                                   \
		}                                                              \
		tlfree(ptr->value);                                            \
	}

#define ENUM_VALUE(ret, type, e)                                               \
	_Generic((ret),                                                        \
	    u64: ({                                                            \
			 u64 r;                                                \
			 memcpy(&r, e.value, sizeof(u64));                     \
			 *(&r);                                                \
		 }),                                                           \
	    default: ({                                                        \
			 if (!strcmp(CLASS_NAME(e.value), "Rc")) {             \
				 ret = *(type *)unwrap(e.value);               \
			 } else if (!copy(&ret, e.value))                      \
				 panic("Could not copy object");               \
			 *(type *)&ret;                                        \
		 }))
#define ENUM_TYPE(e) e.type

#define DEFINE_ENUM(x) x##Ptr Cleanup(x##_cleanup)

#define DEFINE_BUILD_ENUM_VALUE(sign, bits)                                    \
	static void *build_enum_value_##sign##bits(sign##bits *v,              \
						   char *type_str) {           \
		char type_compare[20];                                         \
		snprintf(type_compare, 20, "%s%s", #sign, #bits);              \
		if (strcmp(type_str, type_compare)) {                          \
			panic("Attempt to build an enum with the "             \
			      "wrong value. Expected [%s], Found [%s]",        \
			      type_compare, type_str);                         \
		}                                                              \
		void *value = tlmalloc(sizeof(sign##bits));                    \
		if (value == NULL) {                                           \
			panic("Could not allocate sufficient memory");         \
		}                                                              \
		memcpy(value, v, sizeof(sign##bits));                          \
		return value;                                                  \
	}

DEFINE_BUILD_ENUM_VALUE(u, 128)
DEFINE_BUILD_ENUM_VALUE(u, 64)
DEFINE_BUILD_ENUM_VALUE(u, 32)
DEFINE_BUILD_ENUM_VALUE(u, 16)
DEFINE_BUILD_ENUM_VALUE(u, 8)
DEFINE_BUILD_ENUM_VALUE(i, 128)
DEFINE_BUILD_ENUM_VALUE(i, 64)
DEFINE_BUILD_ENUM_VALUE(i, 32)
DEFINE_BUILD_ENUM_VALUE(i, 16)
DEFINE_BUILD_ENUM_VALUE(i, 8)
DEFINE_BUILD_ENUM_VALUE(f, 64)
DEFINE_BUILD_ENUM_VALUE(f, 32)

static void *build_enum_value_bool(bool *v, char *type_str) {
	if (strcmp(type_str, "bool")) {
		panic("Attempt to build an enum with the "
		      "wrong value. Expected [%s], Found [%s]",
		      "bool", type_str);
	}
	void *value = tlmalloc(sizeof(bool));
	if (value == NULL) {
		panic("Could not allocate sufficient memory");
	}
	memcpy(value, v, sizeof(bool));
	return value;
}

static void *build_enum_value_usize(usize *v, char *type_str) {
	if (strcmp(type_str, "usize")) {
		panic("Attempt to build an enum with the "
		      "wrong value. Expected [%s], Found [%s]",
		      "usize", type_str);
	}
	void *value = tlmalloc(sizeof(usize));
	if (value == NULL) {
		panic("Could not allocate sufficient memory");
	}
	memcpy(value, v, sizeof(usize));
	return value;
}

static void *build_enum_value(void *v, char *type_str) {
	if (!strcmp(CLASS_NAME(v), "Rc")) {
		// rc, check inner type instead
		Rc rc_clone;
		clone(&rc_clone, v);
		Object *obj = unwrap(&rc_clone);
		if (strcmp(CLASS_NAME(obj), type_str))
			panic("Attempt to build an enum with the "
			      "wrong value. Expected [%s], Found "
			      "[%s]",
			      type_str, CLASS_NAME(v));
	} else if (strcmp(type_str, CLASS_NAME(v))) {
		panic("Attempt to build an enum with the "
		      "wrong value. Expected [%s], Found [%s]",
		      type_str, CLASS_NAME(v));
	}
	if (!implements(v, "size") || !implements(v, "copy")) {
		panic("size and copy are required. RC can "
		      "be used to wrap other types");
	}

	void *value = tlmalloc(size((Object *)v));
	if (!value)
		panic("Could not allocate sufficient memory");
	if (!copy(value, v))
		panic("Could not copy object");
	return value;
}

#define BUILD_ENUM(name, type, v)                                              \
	_Generic((v),                                                          \
	    usize: ({                                                          \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_usize(                   \
					 (usize *)&v, name##_Arr__[type]),     \
				     true};                                    \
		 }),                                                           \
	    bool: ({                                                           \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_bool(                    \
					 (bool *)&v, name##_Arr__[type]),      \
				     true};                                    \
		 }),                                                           \
	    f64: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_f64((f64 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    f32: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_f32((f32 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    i128: ({                                                           \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_i128(                    \
					 (i128 *)&v, name##_Arr__[type]),      \
				     true};                                    \
		 }),                                                           \
	    i64: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_i64((i64 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    i32: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_i32((i32 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    i16: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_i16((i16 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    i8: ({                                                             \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_i8((i8 *)&v,             \
							 name##_Arr__[type]),  \
				     true};                                    \
		 }),                                                           \
	    u128: ({                                                           \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_u128(                    \
					 (u128 *)&v, name##_Arr__[type]),      \
				     true};                                    \
		 }),                                                           \
	    u64: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_u64((u64 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    u32: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_u32((u32 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    u16: ({                                                            \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_u16((u16 *)&v,           \
							  name##_Arr__[type]), \
				     true};                                    \
		 }),                                                           \
	    u8: ({                                                             \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value_u8((u8 *)&v,             \
							 name##_Arr__[type]),  \
				     true};                                    \
		 }),                                                           \
	    default: ({                                                        \
			 (name##Ptr){{&name##_Vtable__, #name},                \
				     type,                                     \
				     build_enum_value(&v, name##_Arr__[type]), \
				     false};                                   \
		 }))

#define VARIANTS(...) __VA_ARGS__
#define TYPE(field_type, field_name) field_type field_name;
#define TYPES(...) __VA_ARGS__

#define MATCH(name, ...)                                                       \
	({                                                                     \
		int mytype = name.type;                                        \
		__VA_ARGS__                                                    \
	})
#define VARIANT(type, exe)                                                     \
	if (mytype == type)                                                    \
	exe

#endif // _BASE_ENUM__
