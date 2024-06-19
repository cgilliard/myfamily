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

#ifndef _RESULT_BASE__
#define _RESULT_BASE__

#include <base/class.h>
#include <base/error.h>
#include <base/prim.h>
#include <base/traits.h>

#define TRAIT_RESULT(T)                                                        \
	TRAIT_REQUIRED(T, ResultPtr, build_ok, void *ref)                      \
	TRAIT_REQUIRED(T, ResultPtr, build_err, Error *ref)

#define TRAIT_UNWRAP_ERR(T) TRAIT_REQUIRED(T, Error, unwrap_err, T##Ptr *result)

CLASS(Result, FNPTR(bool (*is_ok)()) FIELD(Error *, err) FIELD(void *, ref)
		  FIELD(bool, no_cleanup))
IMPL(Result, TRAIT_UNWRAP)
IMPL(Result, TRAIT_UNWRAP_ERR)
IMPL(Result, TRAIT_RESULT)
#define Result DEFINE_CLASS(Result)

static Error unwrap_err(Result *obj) { return Result_unwrap_err(obj); }

#define DEFINE_RESULT_BUILD_OK_PRIM(bits, type_upper, type_lower)              \
	static Result Result_build_ok_##type_lower##bits(void *value) {        \
		type_upper##bits v =                                           \
		    BUILD(type_upper##bits, *((type_lower##bits *)value));     \
		ResultPtr ret = Result_build_ok(&v);                           \
		return ret;                                                    \
	}

DEFINE_RESULT_BUILD_OK_PRIM(128, U, u)
DEFINE_RESULT_BUILD_OK_PRIM(64, U, u)
DEFINE_RESULT_BUILD_OK_PRIM(32, U, u)
DEFINE_RESULT_BUILD_OK_PRIM(16, U, u)
DEFINE_RESULT_BUILD_OK_PRIM(8, U, u)
DEFINE_RESULT_BUILD_OK_PRIM(128, I, i)
DEFINE_RESULT_BUILD_OK_PRIM(64, I, i)
DEFINE_RESULT_BUILD_OK_PRIM(32, I, i)
DEFINE_RESULT_BUILD_OK_PRIM(16, I, i)
DEFINE_RESULT_BUILD_OK_PRIM(8, I, i)

static Result Result_build_ok_bool(void *value) {
	Bool v = BUILD(Bool, *((bool *)value));
	ResultPtr ret = Result_build_ok(&v);

	return ret;
}

#define Ok(x)                                                                  \
	_Generic((x),                                                          \
	    u128: Result_build_ok_u128,                                        \
	    u64: Result_build_ok_u64,                                          \
	    u32: Result_build_ok_u32,                                          \
	    u16: Result_build_ok_u16,                                          \
	    u8: Result_build_ok_u8,                                            \
	    i128: Result_build_ok_i128,                                        \
	    i64: Result_build_ok_i64,                                          \
	    i32: Result_build_ok_i32,                                          \
	    i16: Result_build_ok_i16,                                          \
	    i8: Result_build_ok_i8,                                            \
	    bool: Result_build_ok_bool,                                        \
	    default: Result_build_ok)(&x)

#define Err(x) Result_build_err(&x)

#define Try(x)                                                                 \
	({                                                                     \
		if (!x.is_ok()) {                                              \
			Error e = unwrap_err(&x);                              \
			return Err(e);                                         \
		};                                                             \
		unwrap(&x);                                                    \
	})

#define Expect(x)                                                              \
	({                                                                     \
		if (!x.is_ok()) {                                              \
			panic("Expected Ok, found Err");                       \
		}                                                              \
		unwrap(&x);                                                    \
	})

#endif // _RESULT_BASE__
