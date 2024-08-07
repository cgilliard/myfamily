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

#define RC_FLAGS_NO_CLEANUP 0x1
#define RC_FLAGS_PRIM 0x1 << 1
#define RC_FLAGS_NO_COUNTER 0x1 << 2

#define TRAIT_RC_BUILD(T) TRAIT_REQUIRED(T, T##Ptr, build, Slab ref)

CLASS(Rc, FIELD(Slab, ref) FIELD(Slab, count) FIELD(u8, flags))
IMPL(Rc, TRAIT_COPY)
IMPL(Rc, TRAIT_RC_BUILD)
IMPL(Rc, TRAIT_UNWRAP)
#define Rc DEFINE_CLASS(Rc)

static GETTER(Rc, flags);
static SETTER(Rc, flags);

#define RC(x) Rc_build(x)

#define HEAPIFY_OBJ(obj, is_prim)                                              \
	({                                                                     \
		Slab slab;                                                     \
		slab.data = 0;                                                 \
		RcPtr ret;                                                     \
		u64 _sz__ = mysize(&obj);                                      \
		if (mymalloc(&slab, _sz__)) {                                  \
			slab.data = NULL;                                      \
			ret = RC(slab);                                        \
		} else {                                                       \
			memcpy(slab.data, &obj, _sz__);                        \
			NO_CLEANUP(obj);                                       \
			ret = RC(slab);                                        \
			if (is_prim)                                           \
				SET(Rc, &ret, flags, RC_FLAGS_PRIM);           \
			else                                                   \
				SET(Rc, &ret, flags, 0);                       \
		}                                                              \
		ret;                                                           \
	})

#define HEAPIFY(v)                                                             \
	_Generic((v),                                                          \
	    uint8_t: ({                                                        \
			 U8Ptr obj = BUILD(U8, 0);                             \
			 memcpy(&obj._value, &v, sizeof(u8));                  \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    uint16_t: ({                                                       \
			 U16Ptr obj = BUILD(U16, 0);                           \
			 memcpy(&obj._value, &v, sizeof(u16));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    uint32_t: ({                                                       \
			 U32Ptr obj = BUILD(U32, 0);                           \
			 memcpy(&obj._value, &v, sizeof(u32));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    uint64_t: ({                                                       \
			 U64Ptr obj = BUILD(U64, 0);                           \
			 memcpy(&obj._value, &v, sizeof(u64));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    __uint128_t: ({                                                    \
			 U128Ptr obj = BUILD(U128, 0);                         \
			 memcpy(&obj._value, &v, sizeof(u128));                \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    int8_t: ({                                                         \
			 I8Ptr obj = BUILD(I8, 0);                             \
			 memcpy(&obj._value, &v, sizeof(i8));                  \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    int16_t: ({                                                        \
			 I16Ptr obj = BUILD(I16, 0);                           \
			 memcpy(&obj._value, &v, sizeof(i16));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    int32_t: ({                                                        \
			 I32Ptr obj = BUILD(I32, 0);                           \
			 memcpy(&obj._value, &v, sizeof(i32));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    int64_t: ({                                                        \
			 I64Ptr obj = BUILD(I64, 0);                           \
			 memcpy(&obj._value, &v, sizeof(i64));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    __int128_t: ({                                                     \
			 I128Ptr obj = BUILD(I128, 0);                         \
			 memcpy(&obj._value, &v, sizeof(i128));                \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    bool: ({                                                           \
			 BoolPtr obj = BUILD(Bool, 0);                         \
			 memcpy(&obj._value, &v, sizeof(bool));                \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    float: ({                                                          \
			 F32Ptr obj = BUILD(F32, 0);                           \
			 memcpy(&obj._value, &v, sizeof(f32));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    double: ({                                                         \
			 F64Ptr obj = BUILD(F64, 0);                           \
			 memcpy(&obj._value, &v, sizeof(f64));                 \
			 HEAPIFY_OBJ(obj, true);                               \
		 }),                                                           \
	    default: ({ HEAPIFY_OBJ(v, false); }))

#endif // _CORE_RC__
