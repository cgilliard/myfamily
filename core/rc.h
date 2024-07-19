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
#include <core/prim.h>
#include <core/traits_base.h>

#define RC_FLAGS_NO_CLEANUP 0x1

#define TRAIT_RC_BUILD(T) TRAIT_REQUIRED(T, T##Ptr, build, void *ref)

CLASS(Rc, FIELD(void *, ref) FIELD(u64 *, count) FIELD(u8, flags))
IMPL(Rc, TRAIT_COPY)
IMPL(Rc, TRAIT_RC_BUILD)
IMPL(Rc, TRAIT_UNWRAP)
#define Rc DEFINE_CLASS(Rc)

static GETTER(Rc, flags);
static SETTER(Rc, flags);

#define RC(x) Rc_build(x)

#define DEREF(rc, clz) ((clz *)unwrap(&rc))

#define HEAPIFY_DIRECT(value)                                                  \
	({                                                                     \
		void *__ret_ = mymalloc(size(&value));                         \
		memcpy(__ret_, &value, size(&value));                          \
		__ret_;                                                        \
	})

#define HEAPIFY(v)                                                             \
	_Generic((v),                                                          \
	    u8: ({                                                             \
			 U8Ptr obj = BUILD(U8, 0);                             \
			 memcpy(&obj._value, &v, sizeof(u8));                  \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    u16: ({                                                            \
			 U16Ptr obj = BUILD(U16, 0);                           \
			 memcpy(&obj._value, &v, sizeof(u16));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    u32: ({                                                            \
			 U32Ptr obj = BUILD(U32, 0);                           \
			 memcpy(&obj._value, &v, sizeof(u32));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    u64: ({                                                            \
			 U64Ptr obj = BUILD(U64, 0);                           \
			 memcpy(&obj._value, &v, sizeof(u64));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    u128: ({                                                           \
			 U128Ptr obj = BUILD(U128, 0);                         \
			 memcpy(&obj._value, &v, sizeof(u128));                \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    i8: ({                                                             \
			 I8Ptr obj = BUILD(I8, 0);                             \
			 memcpy(&obj._value, &v, sizeof(i8));                  \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    i16: ({                                                            \
			 I16Ptr obj = BUILD(I16, 0);                           \
			 memcpy(&obj._value, &v, sizeof(i16));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    i32: ({                                                            \
			 I32Ptr obj = BUILD(I32, 0);                           \
			 memcpy(&obj._value, &v, sizeof(i32));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    i64: ({                                                            \
			 I64Ptr obj = BUILD(I64, 0);                           \
			 memcpy(&obj._value, &v, sizeof(i64));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    i128: ({                                                           \
			 I128Ptr obj = BUILD(I128, 0);                         \
			 memcpy(&obj._value, &v, sizeof(i128));                \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    bool: ({                                                           \
			 BoolPtr obj = BUILD(Bool, 0);                         \
			 memcpy(&obj._value, &v, sizeof(bool));                \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    usize: ({                                                          \
			 USizePtr obj = BUILD(USize, 0);                       \
			 memcpy(&obj._value, &v, sizeof(usize));               \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    f32: ({                                                            \
			 F32Ptr obj = BUILD(F32, 0);                           \
			 memcpy(&obj._value, &v, sizeof(f32));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    f64: ({                                                            \
			 F64Ptr obj = BUILD(F64, 0);                           \
			 memcpy(&obj._value, &v, sizeof(f64));                 \
			 void *ptr = HEAPIFY_DIRECT(obj);                      \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }),                                                           \
	    default: ({                                                        \
			 void *ptr = HEAPIFY_DIRECT(v);                        \
			 RcPtr ret = RC(ptr);                                  \
			 ret;                                                  \
		 }))

#endif // _CORE_RC__
