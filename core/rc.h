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

#ifndef _CORE_RC__
#define _CORE_RC__

#include <core/class.h>
#include <core/traits_base.h>

#define RC_FLAGS_NO_CLEANUP (0x1)
#define RC_FLAGS_PRIM (0x1 << 1)
#define RC_FLAGS_NO_COUNTER (0x1 << 2)
#define RC_FLAGS_CONSUMED (0x1 << 3)

#define TRAIT_RC_BUILD(T) TRAIT_REQUIRED(T, T##Ptr, build, Slab ref)

CLASS(Rc, FIELD(Slab, ref) FIELD(Slab, count) FIELD(u8, flags))
IMPL(Rc, TRAIT_CLONE)
IMPL(Rc, TRAIT_DEEP_COPY)
IMPL(Rc, TRAIT_RC_BUILD)
IMPL(Rc, TRAIT_UNWRAP)
IMPL(Rc, TRAIT_BORROW)
#define Rc DEFINE_CLASS(Rc)

static GETTER(Rc, flags);
static SETTER(Rc, flags);

#define RC(x) Rc_build(x)

#define HEAPIFY_OBJ(obj, is_prim)                                              \
	({                                                                     \
		Slab _slab__;                                                  \
		_slab__.data = 0;                                              \
		RcPtr _ret_heapify_obj_;                                       \
		u64 _sz__ = mysize(&obj);                                      \
		if (mymalloc(&_slab__, _sz__)) {                               \
			_slab__.data = NULL;                                   \
			_ret_heapify_obj_ = RC(_slab__);                       \
		} else {                                                       \
			memcpy(_slab__.data, &obj, _sz__);                     \
			NO_CLEANUP(obj);                                       \
			_ret_heapify_obj_ = RC(_slab__);                       \
			if (is_prim)                                           \
				SET(Rc, &_ret_heapify_obj_, flags,             \
				    RC_FLAGS_PRIM | RC_FLAGS_NO_CLEANUP);      \
			else                                                   \
				SET(Rc, &_ret_heapify_obj_, flags, 0);         \
		}                                                              \
		_ret_heapify_obj_;                                             \
	})

#define HEAPIFY(v)                                                             \
	_Generic((v),                                                          \
	    uint8_t: ({                                                        \
			 U8Ptr _obj__ = BUILD(U8, 0);                          \
			 memcpy(&_obj__._value, &v, sizeof(u8));               \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    uint16_t: ({                                                       \
			 U16Ptr _obj__ = BUILD(U16, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(u16));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    uint32_t: ({                                                       \
			 U32Ptr _obj__ = BUILD(U32, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(u32));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    uint64_t: ({                                                       \
			 U64Ptr _obj__ = BUILD(U64, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(u64));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    __uint128_t: ({                                                    \
			 U128Ptr _obj__ = BUILD(U128, 0);                      \
			 memcpy(&_obj__._value, &v, sizeof(u128));             \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    int8_t: ({                                                         \
			 I8Ptr _obj__ = BUILD(I8, 0);                          \
			 memcpy(&_obj__._value, &v, sizeof(i8));               \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    int16_t: ({                                                        \
			 I16Ptr _obj__ = BUILD(I16, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(i16));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    int32_t: ({                                                        \
			 I32Ptr _obj__ = BUILD(I32, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(i32));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    int64_t: ({                                                        \
			 I64Ptr _obj__ = BUILD(I64, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(i64));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    __int128_t: ({                                                     \
			 I128Ptr _obj__ = BUILD(I128, 0);                      \
			 memcpy(&_obj__._value, &v, sizeof(i128));             \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    bool: ({                                                           \
			 BoolPtr _obj__ = BUILD(Bool, 0);                      \
			 memcpy(&_obj__._value, &v, sizeof(bool));             \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    float: ({                                                          \
			 F32Ptr _obj__ = BUILD(F32, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(f32));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    double: ({                                                         \
			 F64Ptr _obj__ = BUILD(F64, 0);                        \
			 memcpy(&_obj__._value, &v, sizeof(f64));              \
			 HEAPIFY_OBJ(_obj__, true);                            \
		 }),                                                           \
	    default: ({ HEAPIFY_OBJ(v, false); }))

#endif // _CORE_RC__
