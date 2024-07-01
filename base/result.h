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

#ifndef _BASE_RESULT__
#define _BASE_RESULT__

#include <base/class.h>
#include <base/error.h>
#include <base/prim.h>
#include <base/rc.h>
#include <base/traits_base.h>

#define TRAIT_RESULT(T)                                                        \
	TRAIT_REQUIRED(T, ResultPtr, build_ok, void *ref)                      \
	TRAIT_REQUIRED(T, ResultPtr, build_err, Error *ref)

#define TRAIT_UNWRAP_ERR(T) TRAIT_REQUIRED(T, Error, unwrap_err, T##Ptr *result)

CLASS(Result, FNPTR(bool (*is_ok)()) FIELD(Error *, err) FIELD(void *, ref)
		  FIELD(bool, no_cleanup))
IMPL(Result, TRAIT_UNWRAP)
IMPL(Result, TRAIT_UNWRAP_AS)
IMPL(Result, TRAIT_COPY)
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

static Result Result_build_ok_usize(void *value) {
	USize v = BUILD(USize, *((usize *)value));
	ResultPtr ret = Result_build_ok(&v);
	return ret;
}

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
	    usize: Result_build_ok_usize,                                      \
	    bool: Result_build_ok_bool,                                        \
	    default: Result_build_ok)(&x)

#define Err(x) Result_build_err(&x)

#define Try(type, x)                                                           \
	({                                                                     \
		if (!x.is_ok()) {                                              \
			Error e = unwrap_err(&x);                              \
			return Err(e);                                         \
		};                                                             \
		*(type *)unwrap(&x);                                           \
	})

#define Try2(type, res)                                                        \
	({                                                                     \
		if (!res.is_ok()) {                                            \
			Error e = unwrap_err(&res);                            \
			return Err(e);                                         \
		};                                                             \
		type##Ptr ret;                                                 \
		type##Ptr *tmp = unwrap(&res);                                 \
		memcpy(&ret, tmp, sizeof(type));                               \
		if (!implements((Object *)tmp, "copy"))                        \
			tlfree(tmp);                                           \
		ret;                                                           \
	})

#define todo()                                                                 \
	({                                                                     \
		ErrorKind NOT_IMPLEMENTED = BUILD(ErrorKind, "unimplemented"); \
		Error err = ERROR(NOT_IMPLEMENTED, "Not yet implemented!");    \
		return Err(err);                                               \
	});

#define DEFINE_TRY_IMPL(sign, bits)                                            \
	static sign##bits try_impl_##sign##bits(void *ret, Result r) {         \
		*(sign##bits *)ret = *(sign##bits *)unwrap(&r);                \
		return *(sign##bits *)ret;                                     \
	}

DEFINE_TRY_IMPL(u, 128)
DEFINE_TRY_IMPL(u, 64)
DEFINE_TRY_IMPL(u, 32)
DEFINE_TRY_IMPL(u, 16)
DEFINE_TRY_IMPL(u, 8)
DEFINE_TRY_IMPL(i, 128)
DEFINE_TRY_IMPL(i, 64)
DEFINE_TRY_IMPL(i, 32)
DEFINE_TRY_IMPL(i, 16)
DEFINE_TRY_IMPL(i, 8)

static usize try_impl_usize(void *ret, Result r) {
	*(usize *)ret = *(usize *)unwrap(&r);
	return *(usize *)ret;
}

static bool try_impl_bool(void *ret, Result r) {
	*(bool *)ret = *(bool *)unwrap(&r);
	return *(bool *)ret;
}

#define Expect(type, res)                                                      \
	({                                                                     \
		if (!res.is_ok()) {                                            \
			panic("Expected Ok, found Err");                       \
		}                                                              \
		type##Ptr ret;                                                 \
		type##Ptr *tmp = unwrap(&res);                                 \
		memcpy(&ret, tmp, sizeof(type));                               \
		if (!implements((Object *)tmp, "copy"))                        \
			tlfree(tmp);                                           \
		ret;                                                           \
	})

#define Try3(type, res)                                                        \
	({                                                                     \
		if (!res.is_ok()) {                                            \
			Error e = unwrap_err(&res);                            \
			return Err(e);                                         \
		}                                                              \
		type##Ptr ret;                                                 \
		type##Ptr *tmp = unwrap(&res);                                 \
		memcpy(&ret, tmp, sizeof(type));                               \
		if (!implements((Object *)tmp, "copy"))                        \
			tlfree(tmp);                                           \
		ret;                                                           \
	})

/*
#define Try3(ret, res)                                                         \
	_Generic((ret),                                                        \
	    u128: try_impl_u128(&ret, res),                                    \
	    u64: try_impl_u64(&ret, res),                                      \
	    u32: try_impl_u32(&ret, res),                                      \
	    u16: try_impl_u16(&ret, res),                                      \
	    u8: try_impl_u8(&ret, res),                                        \
	    i128: try_impl_i128(&ret, res),                                    \
	    i64: try_impl_i64(&ret, res),                                      \
	    i32: try_impl_i32(&ret, res),                                      \
	    i16: try_impl_i16(&ret, res),                                      \
	    i8: try_impl_i8(&ret, res),                                        \
	    usize: try_impl_usize(&ret, res),                                  \
	    default: try_impl(&ret, res))
	    */

#endif // _BASE_RESULT__
