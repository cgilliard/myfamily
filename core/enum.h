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
		if ((ptr->flags & ENUM_FLAG_NO_CLEANUP) == 0) {                \
			if ((ptr->flags & ENUM_FLAG_PRIM) == 0) {              \
				cleanup(ptr->slab.data);                       \
			}                                                      \
			if (myfree(&ptr->slab))                                \
				printf("Cleanup failed!\n");                   \
		}                                                              \
	}

#define DEFINE_ENUM(x)                                                         \
	x##Ptr __attribute__((warn_unused_result, cleanup(x##_cleanup_impl)))

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

#define BUILD_ENUM_PRIM(name, type, v, _size__)                                \
	({                                                                     \
		Slab slab;                                                     \
		int _res__ = mymalloc(&slab, _size__);                         \
		u8 flags = ENUM_FLAG_PRIM;                                     \
		if (!_res__)                                                   \
			memcpy(slab.data, &v, _size__);                        \
		else {                                                         \
			slab.data = NULL;                                      \
			flags = ENUM_FLAG_NO_CLEANUP | ENUM_FLAG_PRIM;         \
		}                                                              \
		(name){{&name##_Vtable__, #name}, type, flags, slab};          \
	})

#define BUILD_ENUM(name, type, v)                                              \
	_Generic((v),                                                          \
	    u8: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(u8)),               \
	    u16: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(u16)),             \
	    u32: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(u32)),             \
	    u64: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(u64)),             \
	    u128: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(u128)),           \
	    i8: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(i8)),               \
	    i16: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(i16)),             \
	    i32: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(i32)),             \
	    i64: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(i64)),             \
	    i128: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(i128)),           \
	    f32: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(f32)),             \
	    f64: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(f64)),             \
	    bool: BUILD_ENUM_PRIM(name##Ptr, type, v, sizeof(bool)),           \
	    default: ({                                                        \
			 RcPtr _rc__ = HEAPIFY(v);                             \
			 u8 flags;                                             \
			 Slab slab;                                            \
			 if (_rc__._ref.data) {                                \
				 flags = 0;                                    \
				 u64 _sz__ = mysize(&_rc__);                   \
				 if (!mymalloc(&slab, _sz__))                  \
					 memcpy(slab.data, &_rc__, _sz__);     \
				 else {                                        \
					 flags = ENUM_FLAG_NO_CLEANUP;         \
					 slab.data = NULL;                     \
					 cleanup(&_rc__);                      \
				 }                                             \
                                                                               \
			 } else {                                              \
				 flags = ENUM_FLAG_NO_CLEANUP;                 \
				 slab.data = NULL;                             \
			 }                                                     \
			 (name##Ptr){                                          \
			     {&name##Ptr_Vtable__, #name}, type, flags, slab}; \
		 }))

#define ENUM_VALUE_PRIM(ret, type, e, storage_type)                            \
	({                                                                     \
		storage_type _ret__;                                           \
		memcpy(&_ret__, e.slab.data, sizeof(storage_type));            \
		_ret__;                                                        \
	})

#define ENUM_VALUE(ret, type, e)                                               \
	_Generic((ret),                                                        \
	    u8: ENUM_VALUE_PRIM(ret, type, e, u8),                             \
	    u16: ENUM_VALUE_PRIM(ret, type, e, u16),                           \
	    u32: ENUM_VALUE_PRIM(ret, type, e, u32),                           \
	    u64: ENUM_VALUE_PRIM(ret, type, e, u64),                           \
	    u128: ENUM_VALUE_PRIM(ret, type, e, u128),                         \
	    i8: ENUM_VALUE_PRIM(ret, type, e, i8),                             \
	    i16: ENUM_VALUE_PRIM(ret, type, e, i16),                           \
	    i32: ENUM_VALUE_PRIM(ret, type, e, i32),                           \
	    i64: ENUM_VALUE_PRIM(ret, type, e, i64),                           \
	    i128: ENUM_VALUE_PRIM(ret, type, e, i128),                         \
	    f32: ENUM_VALUE_PRIM(ret, type, e, f32),                           \
	    f64: ENUM_VALUE_PRIM(ret, type, e, f64),                           \
	    bool: ENUM_VALUE_PRIM(ret, type, e, bool),                         \
	    default: ({                                                        \
			 type##Ptr _ret__ = *(type *)unwrap(e.slab.data);      \
			 _ret__;                                               \
		 }))

#endif // _CORE_ENUM__
