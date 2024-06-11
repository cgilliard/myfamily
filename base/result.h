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

#include <base/error.h>
#include <base/vtable.h>

// Result

// typedef
typedef struct {
	Vtable *vtable;
	bool (*is_ok)();
	Error *err;
	void *ref;
	bool no_clean;
} ResultPtr;

// cleanup
void result_free(ResultPtr *ptr);
#define Result ResultPtr CLEANUP(result_free)

void *result_unwrap(Result *result);
Error *result_unwrap_err(Result *result);

// vtable
static VtableEntry ResultVtableEntries[] = {{"unwrap", result_unwrap},
					    {"unwrap_err", result_unwrap_err}};
DEFINE_VTABLE(ResultVtable, ResultVtableEntries)

// builders
Result result_build_ok(void *ref);
Result result_build_err(Error err);

void result_init_prim_generic(ResultPtr *ptr, size_t size, void *ref);

#define DEFINE_RESULT_BUILD_OK(type, type_name)                                \
	static Result result_build_ok_##type_name(void *ref) {                 \
		ResultPtr ret;                                                 \
		result_init_prim_generic(&ret, sizeof(type), ref);             \
		return ret;                                                    \
	}

DEFINE_RESULT_BUILD_OK(u8, u8);
DEFINE_RESULT_BUILD_OK(u16, u16);
DEFINE_RESULT_BUILD_OK(u32, u32);
DEFINE_RESULT_BUILD_OK(u64, u64);
DEFINE_RESULT_BUILD_OK(u128, u128);

DEFINE_RESULT_BUILD_OK(i8, i8);
DEFINE_RESULT_BUILD_OK(i16, i16);
DEFINE_RESULT_BUILD_OK(i32, i32);
DEFINE_RESULT_BUILD_OK(i64, i64);
DEFINE_RESULT_BUILD_OK(i128, i128);

DEFINE_RESULT_BUILD_OK(f32, f32);
DEFINE_RESULT_BUILD_OK(f64, f64);

DEFINE_RESULT_BUILD_OK(bool, bool);

// macros
#define Ok(x)                                                                  \
	_Generic((x),                                                          \
	    i8: result_build_ok_i8,                                            \
	    i16: result_build_ok_i16,                                          \
	    i32: result_build_ok_i32,                                          \
	    i64: result_build_ok_i64,                                          \
	    i128: result_build_ok_i128,                                        \
	    u8: result_build_ok_u8,                                            \
	    u16: result_build_ok_u16,                                          \
	    u32: result_build_ok_u32,                                          \
	    u64: result_build_ok_u64,                                          \
	    u128: result_build_ok_u128,                                        \
	    f32: result_build_ok_f32,                                          \
	    f64: result_build_ok_f64,                                          \
	    bool: result_build_ok_bool,                                        \
	    default: result_build_ok)(&x)

#define Err(x) result_build_err(x)

#define Try(x)                                                                 \
	({                                                                     \
		if (!x.is_ok()) {                                              \
			ErrorPtr *e = unwrap_err(&x);                          \
			return Err(*e);                                        \
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
