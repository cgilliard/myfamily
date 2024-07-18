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

#ifndef _CORE_ENUM__
#define _CORE_ENUM__

#include <core/class.h>
#include <core/rc.h>

#define ENUM(name, variants, types)                                            \
	typedef enum { variants } name##_Variants__;                           \
	static char *name##_Arr__[] = {types};                                 \
	CLASS_STATIC_CLEANUP(name, name##_Variants__ type; void *value;        \
			     bool is_prim; bool no_cleanup;)                   \
	static inline void name##_cleanup(name##Ptr *ptr) {                    \
		if (!ptr->no_cleanup) {                                        \
			if (!ptr->is_prim) {                                   \
				cleanup(ptr->value);                           \
			}                                                      \
			myfree(ptr->value);                                    \
		}                                                              \
	}

#define DEFINE_ENUM_VALUE(sign, bits)                                          \
	sign##bits enum_value_##sign##bits(void *value);

DEFINE_ENUM_VALUE(u, 8)
DEFINE_ENUM_VALUE(u, 16)
DEFINE_ENUM_VALUE(u, 32)
DEFINE_ENUM_VALUE(u, 64)
DEFINE_ENUM_VALUE(u, 128)
DEFINE_ENUM_VALUE(i, 8)
DEFINE_ENUM_VALUE(i, 16)
DEFINE_ENUM_VALUE(i, 32)
DEFINE_ENUM_VALUE(i, 64)
DEFINE_ENUM_VALUE(i, 128)
DEFINE_ENUM_VALUE(f, 32)
DEFINE_ENUM_VALUE(f, 64)

bool enum_value_bool(void *value);
usize enum_value_usize(void *value);

#define ENUM_VALUE(ret, type, e)                                               \
	_Generic((ret),                                                        \
	    u8: enum_value_u8(e.value),                                        \
	    u16: enum_value_u16(e.value),                                      \
	    u32: enum_value_u32(e.value),                                      \
	    u64: enum_value_u64(e.value),                                      \
	    u128: enum_value_u128(e.value),                                    \
	    i8: enum_value_i8(e.value),                                        \
	    i16: enum_value_i16(e.value),                                      \
	    i32: enum_value_i32(e.value),                                      \
	    i64: enum_value_i64(e.value),                                      \
	    i128: enum_value_i128(e.value),                                    \
	    f32: enum_value_f32(e.value),                                      \
	    f64: enum_value_f64(e.value),                                      \
	    bool: enum_value_bool(e.value),                                    \
	    usize: enum_value_usize(e.value),                                  \
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
	void *build_enum_value_##sign##bits(sign##bits *v, char *type_str);

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

void *build_enum_value_bool(bool *v, char *type_str);
void *build_enum_value_usize(usize *v, char *type_str);
void *build_enum_value(void *v, char *type_str);

#define BUILD_ENUM(name, type, v)                                              \
	_Generic((v),                                                          \
	    usize: ({                                                          \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_usize(                   \
					 (usize *)&v, name##_Arr__[type]),     \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    bool: ({                                                           \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_bool(                    \
					 (bool *)&v, name##_Arr__[type]),      \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    f64: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_f64((f64 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    f32: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_f32((f32 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    i128: ({                                                           \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_i128(                    \
					 (i128 *)&v, name##_Arr__[type]),      \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    i64: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_i64((i64 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    i32: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_i32((i32 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    i16: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_i16((i16 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    i8: ({                                                             \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_i8((i8 *)&v,             \
							 name##_Arr__[type]),  \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    u128: ({                                                           \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_u128(                    \
					 (u128 *)&v, name##_Arr__[type]),      \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    u64: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_u64((u64 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    u32: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_u32((u32 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    u16: ({                                                            \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_u16((u16 *)&v,           \
							  name##_Arr__[type]), \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    u8: ({                                                             \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value_u8((u8 *)&v,             \
							 name##_Arr__[type]),  \
				     true,                                     \
				     false};                                   \
		 }),                                                           \
	    default: ({                                                        \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     build_enum_value(&v, name##_Arr__[type]), \
				     false,                                    \
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

#endif // _CORE_ENUM__
