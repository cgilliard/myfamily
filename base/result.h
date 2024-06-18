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

#define TRAIT_UNWRAP_ERR(T)                                                    \
	TRAIT_REQUIRED(T, ErrorPtr *, unwrap_err, T##Ptr *result)

CLASS(Result, FNPTR(bool (*is_ok)()) FIELD(Error *, err) FIELD(void *, ref))
IMPL(Result, TRAIT_UNWRAP)
IMPL(Result, TRAIT_UNWRAP_ERR)
IMPL(Result, TRAIT_RESULT)
#define Result DEFINE_CLASS(Result)

static ErrorPtr *unwrap_err(Result *obj) { return Result_unwrap_err(obj); }

static Result Result_build_ok_i32(void *value) {
	I32 v = BUILD(I32, *((i32 *)value));
	ResultPtr ret = Result_build_ok(&v);

	return ret;
}

static Result Result_build_ok_u64(void *value) {
	U64 v = BUILD(U64, *((u64 *)value));
	ResultPtr ret = Result_build_ok(&v);

	return ret;
}

#define Ok(x)                                                                  \
	_Generic((x),                                                          \
	    u64: Result_build_ok_u64,                                          \
	    i32: Result_build_ok_i32,                                          \
	    default: Result_build_ok)(&x)

#define Err(x) Result_build_err(&x)

#endif // _RESULT_BASE__
