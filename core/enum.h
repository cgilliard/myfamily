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

#define ENUM_FLAG_PRIM (0x1)
#define ENUM_FLAG_NO_CLEANUP (0x1 << 1)
#define ENUM_FLAG_CONSUMED (0x1 << 2)

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
#define TYPES(...) __VA_ARGS__

#define MATCH(name, ...)                                                       \
	({                                                                     \
		int mytype = name.type;                                        \
		__VA_ARGS__                                                    \
	})
#define VARIANT(type, exe)                                                     \
	if (mytype == type)                                                    \
	exe

#define BUILD_ENUM_PRIM(name, variant, v, size)                                \
	({                                                                     \
		({                                                             \
			Slab _slab;                                            \
			int _res = mymalloc(&_slab, size);                     \
			u8 _flags = ENUM_FLAG_PRIM;                            \
			if (!_res)                                             \
				memcpy(_slab.data, &v, size);                  \
			else {                                                 \
				_slab.data = NULL;                             \
				_flags =                                       \
				    ENUM_FLAG_NO_CLEANUP | ENUM_FLAG_PRIM;     \
			}                                                      \
			name ret = {{&name##_Vtable__, #name},                 \
				    variant,                                   \
				    _flags,                                    \
				    _slab};                                    \
			ret;                                                   \
		});                                                            \
	})

#define BUILD_ENUM(name, variant, v)                                           \
	_Generic((v),                                                          \
	    u8: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(u8)),            \
	    u16: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(u16)),          \
	    u32: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(u32)),          \
	    u64: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(u64)),          \
	    u128: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(u128)),        \
	    i8: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(i8)),            \
	    i16: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(i16)),          \
	    i32: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(i32)),          \
	    i64: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(i64)),          \
	    i128: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(i128)),        \
	    f32: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(f32)),          \
	    f64: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(f64)),          \
	    bool: BUILD_ENUM_PRIM(name##Ptr, variant, v, sizeof(bool)),        \
	    default: ({                                                        \
			 ({                                                    \
				 RcPtr _rc_build_enum_impl__ = HEAPIFY(v);     \
				 u8 _flags;                                    \
				 Slab _slab;                                   \
				 if (_rc_build_enum_impl__._ref.data) {        \
					 _flags = 0;                           \
					 u64 sz =                              \
					     mysize(&_rc_build_enum_impl__);   \
					 if (!mymalloc(&_slab, sz))            \
						 memcpy(                       \
						     _slab.data,               \
						     &_rc_build_enum_impl__,   \
						     sz);                      \
					 else {                                \
						 _flags =                      \
						     ENUM_FLAG_NO_CLEANUP;     \
						 _slab.data = NULL;            \
						 cleanup(                      \
						     &_rc_build_enum_impl__);  \
					 }                                     \
                                                                               \
				 } else {                                      \
					 _flags = ENUM_FLAG_NO_CLEANUP;        \
					 _slab.data = NULL;                    \
				 }                                             \
				 (name##Ptr){{&name##Ptr_Vtable__, #name},     \
					     variant,                          \
					     _flags,                           \
					     _slab};                           \
			 });                                                   \
		 }))

#define ENUM_VALUE_PRIM(e, storage_type)                                       \
	({                                                                     \
		if (((e).flags & ENUM_FLAG_CONSUMED) != 0) {                   \
			panic("Enum has already been consumed");               \
		}                                                              \
		(e).flags |= ENUM_FLAG_CONSUMED;                               \
		storage_type _ret_enum_value_prim__;                           \
		memcpy(&_ret_enum_value_prim__, (e).slab.data,                 \
		       sizeof(storage_type));                                  \
		_ret_enum_value_prim__;                                        \
	})

#define ENUM_VALUE(ret, type, e)                                               \
	_Generic((ret),                                                        \
	    u8: ENUM_VALUE_PRIM(e, u8),                                        \
	    u16: ENUM_VALUE_PRIM(e, u16),                                      \
	    u32: ENUM_VALUE_PRIM(e, u32),                                      \
	    u64: ENUM_VALUE_PRIM(e, u64),                                      \
	    u128: ENUM_VALUE_PRIM(e, u128),                                    \
	    i8: ENUM_VALUE_PRIM(e, i8),                                        \
	    i16: ENUM_VALUE_PRIM(e, i16),                                      \
	    i32: ENUM_VALUE_PRIM(e, i32),                                      \
	    i64: ENUM_VALUE_PRIM(e, i64),                                      \
	    i128: ENUM_VALUE_PRIM(e, i128),                                    \
	    f32: ENUM_VALUE_PRIM(e, f32),                                      \
	    f64: ENUM_VALUE_PRIM(e, f64),                                      \
	    bool: ENUM_VALUE_PRIM(e, bool),                                    \
	    default: ({                                                        \
			 type##Ptr _ret__enum_value__ =                        \
			     *(type *)unwrap((e).slab.data);                   \
			 _ret__enum_value__;                                   \
		 }))

#define ENUM_VALUE_PRIM_BORROW(e, storage_type)                                \
	({                                                                     \
		if (((e).flags & ENUM_FLAG_CONSUMED) != 0) {                   \
			panic("Enum has already been consumed");               \
		}                                                              \
		storage_type _ret_enum_value_prim__;                           \
		memcpy(&_ret_enum_value_prim__, (e).slab.data,                 \
		       sizeof(storage_type));                                  \
		_ret_enum_value_prim__;                                        \
	})

#define ENUM_VALUE_BORROW(ret, type, e)                                        \
	_Generic((ret),                                                        \
	    u8: ENUM_VALUE_PRIM_BORROW(e, u8),                                 \
	    u16: ENUM_VALUE_PRIM_BORROW(e, u16),                               \
	    u32: ENUM_VALUE_PRIM_BORROW(e, u32),                               \
	    u64: ENUM_VALUE_PRIM_BORROW(e, u64),                               \
	    u128: ENUM_VALUE_PRIM_BORROW(e, u128),                             \
	    i8: ENUM_VALUE_PRIM_BORROW(e, i8),                                 \
	    i16: ENUM_VALUE_PRIM_BORROW(e, i16),                               \
	    i32: ENUM_VALUE_PRIM_BORROW(e, i32),                               \
	    i64: ENUM_VALUE_PRIM_BORROW(e, i64),                               \
	    i128: ENUM_VALUE_PRIM_BORROW(e, i128),                             \
	    f32: ENUM_VALUE_PRIM_BORROW(e, f32),                               \
	    f64: ENUM_VALUE_PRIM_BORROW(e, f64),                               \
	    bool: ENUM_VALUE_PRIM_BORROW(e, bool),                             \
	    default: ({                                                        \
			 type##Ptr _ret__enum_value__ =                        \
			     *(type *)borrow((e).slab.data);                   \
			 NO_CLEANUP(_ret__enum_value__);                       \
			 _ret__enum_value__;                                   \
		 }))

#endif // _CORE_ENUM__
