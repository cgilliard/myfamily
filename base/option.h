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

#ifndef _BASE_OPTION__
#define _BASE_OPTION__

#include <base/class.h>
#include <base/prim.h>
#include <base/traits_base.h>

#define TRAIT_OPTION(T) TRAIT_REQUIRED(T, OptionPtr, build, void *ref)

static bool is_some_false() { return false; }
static bool is_some_true() { return true; }

CLASS(Option,
      FNPTR(bool (*is_some)()) FIELD(void *, ref) FIELD(bool, no_cleanup))
IMPL(Option, TRAIT_COPY)
IMPL(Option, TRAIT_UNWRAP)
IMPL(Option, TRAIT_OPTION)
#define Option DEFINE_CLASS(Option)

#define DEFINE_OPTION_BUILD_SOME_PRIM(bits, type_upper, type_lower)            \
	static Option Option_build_some_##type_lower##bits(void *value) {      \
		type_upper##bits v =                                           \
		    BUILD(type_upper##bits, *((type_lower##bits *)value));     \
		OptionPtr ret = Option_build(&v);                              \
		return ret;                                                    \
	}

DEFINE_OPTION_BUILD_SOME_PRIM(8, U, u)
DEFINE_OPTION_BUILD_SOME_PRIM(16, U, u)
DEFINE_OPTION_BUILD_SOME_PRIM(32, U, u)
DEFINE_OPTION_BUILD_SOME_PRIM(64, U, u)
DEFINE_OPTION_BUILD_SOME_PRIM(128, U, u)

DEFINE_OPTION_BUILD_SOME_PRIM(8, I, i)
DEFINE_OPTION_BUILD_SOME_PRIM(16, I, i)
DEFINE_OPTION_BUILD_SOME_PRIM(32, I, i)
DEFINE_OPTION_BUILD_SOME_PRIM(64, I, i)
DEFINE_OPTION_BUILD_SOME_PRIM(128, I, i)

static Option None = BUILD(Option, is_some_false, NULL, false);

static Option Option_build_some_bool(void *value) {
	Bool v = BUILD(Bool, *((bool *)value));
	OptionPtr ret = Option_build(&v);

	return ret;
}

#define Some(x)                                                                \
	_Generic((x),                                                          \
	    u128: Option_build_some_u128,                                      \
	    u64: Option_build_some_u64,                                        \
	    u32: Option_build_some_u32,                                        \
	    u16: Option_build_some_u16,                                        \
	    u8: Option_build_some_u8,                                          \
	    i128: Option_build_some_i128,                                      \
	    i64: Option_build_some_i64,                                        \
	    i32: Option_build_some_i32,                                        \
	    i16: Option_build_some_i16,                                        \
	    i8: Option_build_some_i8,                                          \
	    bool: Option_build_some_bool,                                      \
	    default: Option_build)(&x)

#endif // _BASE_OPTION__
