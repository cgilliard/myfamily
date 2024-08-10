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

#ifndef _CORE_TYPES__
#define _CORE_TYPES__

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define u8 uint8_t
#define i8 int8_t
#define u16 uint16_t
#define i16 int16_t
#define u32 uint32_t
#define i32 int32_t
#define u64 uint64_t
#define i64 int64_t
#define u128 __uint128_t
#define i128 __int128_t
#define f64 double
#define f32 float

// for some mapping issues, we make these definitions
#define u8Ptr u8
#define u16Ptr u16
#define u32Ptr u32
#define u64Ptr u64
#define u128Ptr u128
#define i8Ptr i8
#define i16Ptr i16
#define i32Ptr i32
#define i64Ptr i64
#define i128Ptr i128
#define f32Ptr f32
#define f64Ptr f64
#define boolPtr bool

#endif // _CORE_TYPES__
