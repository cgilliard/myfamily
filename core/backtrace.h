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

#ifndef _CORE_BACKTRACE__
#define _CORE_BACKTRACE__

#include <core/traits.h>
#include <core/type.h>

// Define a trait for a Backtrace. This trait is used to access the data with generated
// backtraces.
#define Backtrace DefineTrait(                                          \
    Backtrace,                                                          \
    Required(Const, i32, bt_fn_name, char* buf, u64 limit, i32 index),  \
    Required(Const, i32, bt_bin_name, char* buf, u64 limit, i32 index), \
    Required(Const, i32, bt_address, char* buf, u64 limit, i32 index),  \
    Required(Const, i32, bt_file_path, char* buf, u64 limit, i32 index))

// Define a trait for the BacktraceBuilder. This builder is used to generate backtraces.
#define BacktraceBuilder DefineTrait( \
    BacktraceBuilder,                 \
    Where(T, (Backtrace)),            \
    Required(Const, T, generate_backtrace, i32 limit))

// Note Where syntax: Where(T, <optional list of trait bounds>, <optional default>)
// e.g. Where(T, (Backtrace, Equal), Default(Self)), ...
// detailed example:
// #define TestTrait DefineTrait(
//     TestTrait,
//     Where(T, (Backtrace, Equal), Default(Self)),
//     Where(R, Default(I32)),
//     Where(S),
//     Required(Const, i32, sample_fn, char *abc, int def),
//     RequiredWithDefault(Const, T, helper_fn, R *param, S param)
//     Super(Drop, Build)
// )

#endif // _CORE_BACKTRACE__
