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

#include <core/ekinds.h>
#include <core/enum.h>
#include <core/error.h>
#include <core/rc.h>

ENUM(Result, VARIANTS(Ok, Err), TYPES("Rc", "Error"))
#define Result DEFINE_ENUM(Result)

#define IS_ERR(x) x.type == Err
#define IS_OK(x) x.type == Ok

#define TRY(x, v)                                                              \
	({                                                                     \
		if (IS_ERR(x)) {                                               \
			Error e = UNWRAP_ERR(x);                               \
			return Err(e);                                         \
		}                                                              \
		UNWRAP_PRIM(x, v);                                             \
	})

#define TRYU(x)                                                                \
	({                                                                     \
		if (IS_ERR(x)) {                                               \
			Error e = UNWRAP_ERR(x);                               \
			return Err(e);                                         \
		}                                                              \
	})

#define EXPECT(x, v)                                                           \
	({                                                                     \
		if (IS_ERR(x)) {                                               \
			Error e = UNWRAP_ERR(x);                               \
			print(&e);                                             \
			panic("Expect on an error");                           \
		}                                                              \
		UNWRAP_PRIM(x, v);                                             \
	})

#define OkR(x)                                                                 \
	({                                                                     \
		Rc rc = HEAPIFY(x);                                            \
		ResultPtr r = BUILD_ENUM(Result, Ok, rc);                      \
		r;                                                             \
	})

#define Ok(x)                                                                  \
	({                                                                     \
		Rc rc = HEAPIFY(x);                                            \
		ResultPtr r = TRY_BUILD_ENUM(Result, Ok, rc);                  \
		r;                                                             \
	})

#define UNWRAP(x, type)                                                        \
	({                                                                     \
		type##Ptr ret;                                                 \
		ret = UNWRAP_PRIM(x, ret);                                     \
		if (strcmp(#type, CLASS_NAME(&ret)))                           \
			panic("Expected [%s]. Found [%s].", #type,             \
			      CLASS_NAME(&ret));                               \
		ret;                                                           \
	})

#define UNWRAP_PRIM(x, v)                                                      \
	_Generic((v),                                                          \
	    usize: ({                                                          \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 USizePtr val = ENUM_VALUE(val, USize, x);             \
			 if (strcmp(CLASS_NAME(&val), "USize"))                \
				 panic("Expected [USize]. Found [%s].",        \
				       CLASS_NAME(&val));                      \
			 usize ret = *(usize *)unwrap(&val);                   \
			 ret;                                                  \
		 }),                                                           \
	    isize: ({                                                          \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 ISizePtr val = ENUM_VALUE(val, ISize, x);             \
			 if (strcmp(CLASS_NAME(&val), "ISize"))                \
				 panic("Expected [ISize]. Found [%s].",        \
				       CLASS_NAME(&val));                      \
			 isize ret = *(isize *)unwrap(&val);                   \
			 ret;                                                  \
		 }),                                                           \
	    uint8_t: ({                                                        \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 U8Ptr val = ENUM_VALUE(val, U8, x);                   \
			 if (strcmp(CLASS_NAME(&val), "U8"))                   \
				 panic("Expected [U8]. Found [%s].",           \
				       CLASS_NAME(&val));                      \
			 u8 ret = *(u8 *)unwrap(&val);                         \
			 ret;                                                  \
		 }),                                                           \
	    uint16_t: ({                                                       \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 U16Ptr val = ENUM_VALUE(val, U16, x);                 \
			 if (strcmp(CLASS_NAME(&val), "U16"))                  \
				 panic("Expected [U16]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 u16 ret = *(u16 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    uint32_t: ({                                                       \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 U32Ptr val = ENUM_VALUE(val, U32, x);                 \
			 if (strcmp(CLASS_NAME(&val), "U32"))                  \
				 panic("Expected [U32]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 u32 ret = *(u32 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    uint64_t: ({                                                       \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 U64Ptr val = ENUM_VALUE(val, U64, x);                 \
			 if (strcmp(CLASS_NAME(&val), "U64"))                  \
				 panic("Expected [U64]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 u64 ret = *(u64 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    __uint128_t: ({                                                    \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 U128Ptr val = ENUM_VALUE(val, U128, x);               \
			 if (strcmp(CLASS_NAME(&val), "U128"))                 \
				 panic("Expected [U128]. Found [%s].",         \
				       CLASS_NAME(&val));                      \
			 u128 ret = *(u128 *)unwrap(&val);                     \
			 ret;                                                  \
		 }),                                                           \
	    int8_t: ({                                                         \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 I8Ptr val = ENUM_VALUE(val, I8, x);                   \
			 if (strcmp(CLASS_NAME(&val), "I8"))                   \
				 panic("Expected [I8]. Found [%s].",           \
				       CLASS_NAME(&val));                      \
			 i8 ret = *(i8 *)unwrap(&val);                         \
			 ret;                                                  \
		 }),                                                           \
	    int16_t: ({                                                        \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 I16Ptr val = ENUM_VALUE(val, I16, x);                 \
			 if (strcmp(CLASS_NAME(&val), "I16"))                  \
				 panic("Expected [I16]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 i16 ret = *(i16 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    int32_t: ({                                                        \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 I32Ptr val = ENUM_VALUE(val, I32, x);                 \
			 if (strcmp(CLASS_NAME(&val), "I32"))                  \
				 panic("Expected [I32]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 i32 ret = *(i32 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    int64_t: ({                                                        \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 I64Ptr val = ENUM_VALUE(val, I64, x);                 \
			 if (strcmp(CLASS_NAME(&val), "I64"))                  \
				 panic("Expected [I64]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 i64 ret = *(i64 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    __int128_t: ({                                                     \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 I128Ptr val = ENUM_VALUE(val, I128, x);               \
			 if (strcmp(CLASS_NAME(&val), "I128"))                 \
				 panic("Expected [I128]. Found [%s].",         \
				       CLASS_NAME(&val));                      \
			 i128 ret = *(i128 *)unwrap(&val);                     \
			 ret;                                                  \
		 }),                                                           \
	    float: ({                                                          \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 F32Ptr val = ENUM_VALUE(val, F32, x);                 \
			 if (strcmp(CLASS_NAME(&val), "F32"))                  \
				 panic("Expected [F32]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 f32 ret = *(f32 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    double: ({                                                         \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 F64Ptr val = ENUM_VALUE(val, F64, x);                 \
			 if (strcmp(CLASS_NAME(&val), "F64"))                  \
				 panic("Expected [F64]. Found [%s].",          \
				       CLASS_NAME(&val));                      \
			 f64 ret = *(f64 *)unwrap(&val);                       \
			 ret;                                                  \
		 }),                                                           \
	    bool: ({                                                           \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 BoolPtr val = ENUM_VALUE(val, Bool, x);               \
			 if (strcmp(CLASS_NAME(&val), "Bool"))                 \
				 panic("Expected [Bool]. Found [%s].",         \
				       CLASS_NAME(&val));                      \
			 bool ret = *(bool *)unwrap(&val);                     \
			 ret;                                                  \
		 }),                                                           \
	    default: ({                                                        \
			 if (IS_ERR(x))                                        \
				 panic("Attempt to unwrap an error or none");  \
			 RcPtr rc = *(Rc *)x.value;                            \
			 SET(Rc, &rc, flags, RC_FLAGS_NO_CLEANUP);             \
			 void *ptr = unwrap(&rc);                              \
			 memcpy(&v, ptr, size(ptr));                           \
			 x.no_cleanup = true;                                  \
			 cleanup(&rc);                                         \
			 myfree(x.value);                                      \
			 v;                                                    \
		 }))

static Error STATIC_ALLOC_ERROR = {
    {&ErrorPtr_Vtable__, "Error", NEXT_ID},
    false,
    {{&ErrorKindPtr_Vtable__, "ErrorKind", NEXT_ID}, false, "AllocError"},
    "Could not allocate sufficient memory",
    {{&BacktracePtr_Vtable__, "Backtrace", NEXT_ID}, false, NULL, 0},
    ERROR_NO_CLEANUP};
static Result STATIC_ALLOC_RESULT = {{&ResultPtr_Vtable__, "Result", NEXT_ID},
				     Err,
				     &STATIC_ALLOC_ERROR,
				     true,
				     true};

#define ErrP(e) BUILD_ENUM(Result, Err, e)
#define Err(e) TRY_BUILD_ENUM(Result, Err, e)

#define UNWRAP_ERR(x)                                                          \
	({                                                                     \
		if (IS_OK(x))                                                  \
			panic("Attempt to unwrap_err an ok");                  \
		ErrorPtr ret;                                                  \
		copy(&ret, x.value);                                           \
		ret;                                                           \
	})

#endif // _CORE_RESULT__
