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

#ifndef _OPTION_BASE__
#define _OPTION_BASE__

#include <base/cleanup.h>
#include <base/vtable.h>

#define Option OptionPtr Cleanup(option_free)

typedef struct {
	Vtable *vtable;
	bool (*is_some)();
	void *ref;
	size_t prim_size;
	bool is_prim;
} OptionPtr;

void option_free(OptionPtr *ptr);

Option option_build(void *ref);
void *option_unwrap(Option *option);
size_t option_size(Option *option);
bool option_copy(Option *dest, Option *src);

// vtable
static VtableEntry OptionVtableEntries[] = {{"unwrap", option_unwrap},
					    {"copy", option_copy},
					    {"size", option_size},
					    {"cleanup", option_free}

};
DEFINE_VTABLE(OptionVtable, OptionVtableEntries)

static bool option_is_some_false() { return false; }
static bool option_is_some_true() { return true; }
#define None {&OptionVtable, option_is_some_false, NULL};

void option_init_prim_generic(OptionPtr *ptr, size_t size, void *ref);

#define DEFINE_OPTION_BUILD_SOME(type, type_name)                              \
	static Option option_build_some_##type_name(void *ref) {               \
		OptionPtr ret;                                                 \
		option_init_prim_generic(&ret, sizeof(type), ref);             \
		return ret;                                                    \
	}

DEFINE_OPTION_BUILD_SOME(u8, u8);
DEFINE_OPTION_BUILD_SOME(u16, u16);
DEFINE_OPTION_BUILD_SOME(u32, u32);
DEFINE_OPTION_BUILD_SOME(u64, u64);
DEFINE_OPTION_BUILD_SOME(u128, u128);

DEFINE_OPTION_BUILD_SOME(i8, i8);
DEFINE_OPTION_BUILD_SOME(i16, i16);
DEFINE_OPTION_BUILD_SOME(i32, i32);
DEFINE_OPTION_BUILD_SOME(i64, i64);
DEFINE_OPTION_BUILD_SOME(i128, i128);

DEFINE_OPTION_BUILD_SOME(f32, f32);
DEFINE_OPTION_BUILD_SOME(f64, f64);

DEFINE_OPTION_BUILD_SOME(bool, bool);

// macros
#define Some(x)                                                                \
	_Generic((x),                                                          \
	    i8: option_build_some_i8,                                          \
	    i16: option_build_some_i16,                                        \
	    i32: option_build_some_i32,                                        \
	    i64: option_build_some_i64,                                        \
	    i128: option_build_some_i128,                                      \
	    u8: option_build_some_u8,                                          \
	    u16: option_build_some_u16,                                        \
	    u32: option_build_some_u32,                                        \
	    u64: option_build_some_u64,                                        \
	    u128: option_build_some_u128,                                      \
	    f32: option_build_some_f32,                                        \
	    f64: option_build_some_f64,                                        \
	    bool: option_build_some_bool,                                      \
	    default: option_build)(&x)

// #define Some(x) option_build(&x)

#endif // _OPTION_BASE__
