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
#include <core/slabs.h>

#define ENUM_FLAG_PRIM 0x1
#define ENUM_FLAG_NO_CLEANUP 0x1 << 1

#define ENUM(name, variants, types)                                            \
	typedef enum { variants } name##_Variants__;                           \
	static char *name##_Arr__[] = {types};                                 \
	CLASS_STATIC_CLEANUP(name, name##_Variants__ type; u8 flags;           \
			     Slab slab;)                                       \
	static inline void name##_cleanup(name##Ptr *ptr) {                    \
		printf("enum cleanup %p\n", ptr);                              \
		if ((ptr->flags & ENUM_FLAG_NO_CLEANUP) == 0) {                \
			printf("cleanup enabled\n");                           \
			if ((ptr->flags & ENUM_FLAG_PRIM) == 0) {              \
				printf("not prim. cleanup = %p\n",             \
				       ptr->slab.data);                        \
				cleanup(ptr->slab.data);                       \
				printf("type cleanup complete\n");             \
			}                                                      \
			if (myfree(&ptr->slab))                                \
				panic("Cleanup failed!");                      \
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
isize enum_value_isize(void *value);
usize enum_value_usize(void *value);

#define ENUM_VALUE(ret, type, e)                                               \
	_Generic((ret),                                                        \
	    uint8_t: enum_value_u8(e.value),                                   \
	    uint16_t: enum_value_u16(e.value),                                 \
	    uint32_t: enum_value_u32(e.value),                                 \
	    uint64_t: enum_value_u64(e.value),                                 \
	    __uint128_t: enum_value_u128(e.value),                             \
	    int8_t: enum_value_i8(e.value),                                    \
	    int16_t: enum_value_i16(e.value),                                  \
	    int32_t: enum_value_i32(e.value),                                  \
	    int64_t: enum_value_i64(e.value),                                  \
	    __int128_t: enum_value_i128(e.value),                              \
	    float: enum_value_f32(e.value),                                    \
	    double: enum_value_f64(e.value),                                   \
	    bool: enum_value_bool(e.value),                                    \
	    default: ({                                                        \
			 if (!strcmp(CLASS_NAME(e.value), "Rc")) {             \
				 ret = *(type *)unwrap(e.value);               \
			 } else if (!copy(&ret, e.value))                      \
				 panic("Could not copy object");               \
			 *(type *)&ret;                                        \
		 }))

#define TRY_ENUM_VALUE(ret, type, e)                                           \
	_Generic((ret),                                                        \
	    uint8_t: enum_value_u8(e.value),                                   \
	    uint16_t: enum_value_u16(e.value),                                 \
	    uint32_t: enum_value_u32(e.value),                                 \
	    uint64_t: enum_value_u64(e.value),                                 \
	    __uint128_t: enum_value_u128(e.value),                             \
	    int8_t: enum_value_i8(e.value),                                    \
	    int16_t: enum_value_i16(e.value),                                  \
	    int32_t: enum_value_i32(e.value),                                  \
	    int64_t: enum_value_u64(e.value),                                  \
	    __int128_t: enum_value_i128(e.value),                              \
	    float: enum_value_f32(e.value),                                    \
	    double: enum_value_f64(e.value),                                   \
	    bool: enum_value_bool(e.value),                                    \
	    default: ({                                                        \
			 if (!strcmp(CLASS_NAME(e.value), "Rc")) {             \
				 ret = *(type *)unwrap(e.value);               \
			 } else if (!copy(&ret, e.value)) {                    \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 *(type *)&ret;                                        \
		 }))

#define ENUM_TYPE(e) e.type

#define DEFINE_ENUM(x)                                                         \
	x##Ptr __attribute__((warn_unused_result, cleanup(x##_cleanup_impl)))

#define DEFINE_BUILD_ENUM_VALUE(sign, bits)                                    \
	void *build_enum_value_##sign##bits(Slab *slab, sign##bits *v,         \
					    char *type_str);

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

void *build_enum_value_bool(Slab *slab, bool *v, char *type_str);
void *build_enum_value(Slab *slab, void *v, char *type_str);

// try to build the enum and return the static result which contains the error
// we don't try to create a new error because it requires malloc.
#define TRY_BUILD_ENUM(name, type, v)                                          \
	_Generic((v),                                                          \
	    bool: ({                                                           \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_bool(                    \
			     &slab, (bool *)&v, name##_Arr__[type]);           \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    double: ({                                                         \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_f64(&slab, (f64 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    float: ({                                                          \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_f32(&slab, (f32 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    __int128_t: ({                                                     \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_i128(                    \
			     &slab, (i128 *)&v, name##_Arr__[type]);           \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    int64_t: ({                                                        \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_i64(&slab, (i64 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    int32_t: ({                                                        \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_i32(&slab, (i32 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    int16_t: ({                                                        \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_i16(&slab, (i16 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    int8_t: ({                                                         \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_i8(&slab, (i8 *)&v,      \
							 name##_Arr__[type]);  \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    __uint128_t: ({                                                    \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_u128(                    \
			     &slab, (u128 *)&v, name##_Arr__[type]);           \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    uint64_t: ({                                                       \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_u64(&slab, (u64 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    uint32_t: ({                                                       \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_u32(&slab, (u32 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    uint16_t: ({                                                       \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_u16(&slab, (u16 *)&v,    \
							  name##_Arr__[type]); \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    uint8_t: ({                                                        \
			 Slab slab;                                            \
			 void *ptr = build_enum_value_u8(&slab, (u8 *)&v,      \
							 name##_Arr__[type]);  \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     true,                                     \
				     false,                                    \
				     slab};                                    \
		 }),                                                           \
	    default: ({                                                        \
			 Slab slab;                                            \
			 void *ptr =                                           \
			     build_enum_value(&slab, &v, name##_Arr__[type]);  \
			 if (!ptr) {                                           \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ptr,                                      \
				     false,                                    \
				     false,                                    \
				     slab};                                    \
		 }))

#define BUILD_ENUM(name, type, v)                                              \
	_Generic(                                                              \
	    (v),                                                               \
	    bool: ({                                                           \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_bool(&slab, (bool *)&v,       \
						      name##_Arr__[type]);     \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    double: ({                                                         \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_f64(&slab, (f64 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    float: ({                                                          \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_f32(&slab, (f32 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    __int128_t: ({                                                     \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_i128(&slab, (i128 *)&v,       \
						      name##_Arr__[type]);     \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    int64_t: ({                                                        \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_i64(&slab, (i64 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    int32_t: ({                                                        \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_i32(&slab, (i32 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    int16_t: ({                                                        \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_i16(&slab, (i16 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    int8_t: ({                                                         \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_i8(&slab, (i8 *)&v,           \
						    name##_Arr__[type]);       \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    __uint128_t: ({                                                    \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_u128(&slab, (u128 *)&v,       \
						      name##_Arr__[type]);     \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    uint64_t: ({                                                       \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_u64(&slab, (u64 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    uint32_t: ({                                                       \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_u32(&slab, (u32 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    uint16_t: ({                                                       \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_u16(&slab, (u16 *)&v,         \
						     name##_Arr__[type]);      \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    uint8_t: ({                                                        \
		    Slab slab;                                                 \
		    void *ptr = build_enum_value_u8(&slab, (u8 *)&v,           \
						    name##_Arr__[type]);       \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				true,                                          \
				false,                                         \
				slab};                                         \
	    }),                                                                \
	    default: ({                                                        \
		    Slab slab;                                                 \
		    void *ptr =                                                \
			build_enum_value(&slab, &v, name##_Arr__[type]);       \
		    if (!ptr)                                                  \
			    panic("Could not allocate sufficient memory");     \
		    (name##Ptr){{&name##Ptr_Vtable__, #name},                  \
				type,                                          \
				ptr,                                           \
				false,                                         \
				false,                                         \
				slab};                                         \
	    }))

#define BUILD_ENUM2(name, type, v)                                             \
	_Generic((v),                                                          \
	    u64: ({                                                            \
			 Slab slab;                                            \
			 mymalloc(&slab, sizeof(u64));                         \
			 memcpy(slab.data, &v, sizeof(u64));                   \
			 (name##Ptr){{&name##Ptr_Vtable__, #name},             \
				     type,                                     \
				     ENUM_FLAG_PRIM,                           \
				     slab};                                    \
		 }),                                                           \
	    default: ({                                                        \
			 RcPtr _rc__ = HEAPIFY(v);                             \
			 Slab slab;                                            \
			 u64 _sz__ = size(&_rc__);                             \
			 mymalloc(&slab, _sz__);                               \
			 myclone(slab.data, &_rc__);                           \
			 printf("slab.data=%p\n", slab.data);                  \
			 (name##Ptr){                                          \
			     {&name##Ptr_Vtable__, #name}, type, 0, slab};     \
		 }))

#define ENUM_VALUE2(ret, type, e)                                              \
	_Generic((ret),                                                        \
	    u64: ({                                                            \
			 u64 _ret__;                                           \
			 memcpy(&_ret__, e.slab.data, sizeof(u64));            \
			 _ret__;                                               \
		 }),                                                           \
	    default: ({                                                        \
			 type##Ptr _ret__;                                     \
			 memcpy(&_ret__, e.slab.data, size(e.slab.data));      \
			 _ret__;                                               \
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