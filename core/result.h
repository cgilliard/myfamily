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

#ifndef _CORE_RESULT__
#define _CORE_RESULT__

#include <core/enum.h>
#include <core/rc.h>

ENUM(Result, VARIANTS(Ok, Err), TYPES("Rc", "Error"))
#define Result DEFINE_ENUM(Result)

#define Ok(x)                                                                  \
	({                                                                     \
		Rc rc = HEAPIFY(x);                                            \
		ResultPtr r = BUILD_ENUM(Result, Ok, rc);                      \
		r;                                                             \
	})

#define UNWRAP(x, v)                                                           \
	_Generic((v),                                                          \
	    u8: ({                                                             \
			 U8 val = ENUM_VALUE(val, U8, x);                      \
			 u8 ret = *(u8 *)unwrap(&val);                         \
			 ret;                                                  \
		 }),                                                           \
	    u16: ({                                                            \
			 U16 val = ENUM_VALUE(val, U16, x);                    \
			 u16 ret = *(u16 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    u32: ({                                                            \
			 U32 val = ENUM_VALUE(val, U32, x);                    \
			 u32 ret = *(u32 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    u64: ({                                                            \
			 U64 val = ENUM_VALUE(val, U64, x);                    \
			 u64 ret = *(u64 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    u128: ({                                                           \
			 U128 val = ENUM_VALUE(val, U128, x);                  \
			 u128 ret = *(u128 *)unwrap(&val);                     \
			 ret;                                                  \
		 }),                                                           \
	    i8: ({                                                             \
			 I8 val = ENUM_VALUE(val, I8, x);                      \
			 i8 ret = *(i8 *)unwrap(&val);                         \
			 ret;                                                  \
		 }),                                                           \
	    i16: ({                                                            \
			 I16 val = ENUM_VALUE(val, I16, x);                    \
			 i16 ret = *(i16 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    i32: ({                                                            \
			 I32 val = ENUM_VALUE(val, I32, x);                    \
			 i32 ret = *(i32 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    i64: ({                                                            \
			 I64 val = ENUM_VALUE(val, I64, x);                    \
			 i64 ret = *(i64 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    i128: ({                                                           \
			 I128 val = ENUM_VALUE(val, I128, x);                  \
			 i128 ret = *(i128 *)unwrap(&val);                     \
			 ret;                                                  \
		 }),                                                           \
	    default: ({                                                        \
			 Rc rc = *(Rc *)x.value;                               \
			 void *ptr = unwrap(&rc);                              \
			 memcpy(&v, ptr, size(ptr));                           \
			 v;                                                    \
		 }))

#define Err(e) BUILD_ENUM(Result, Err, e)

#endif // _CORE_RESULT__
