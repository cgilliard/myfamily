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

#ifndef _BASE_TYPES__
#define _BASE_TYPES__

#include <base/limits.h>

typedef unsigned char u8;		// 8 bits
typedef unsigned short u16;		// 16 bits
typedef unsigned int u32;		// 32 bits
typedef unsigned long long u64; // 64 bits
typedef signed char i8;			// 8 bits
typedef signed short i16;		// 16 bits
typedef signed int i32;			// 32 bits
typedef signed long long i64;	// 64 bits
#define usize u64				// 64 bit only supported type.c guards this
#define u128 __uint128_t
#define i128 __int128_t
#define f64 double
#define f32 float
#define bool _Bool
#define true (_Bool)1
#define false (_Bool)0

#define num __int128_t
#define ch unsigned char
#define dec double

// atomics
typedef _Atomic unsigned long long au64;
typedef _Atomic unsigned int au32;
typedef _Atomic _Bool abool;

// NULL
#define NULL ((void *)0)

#endif // _BASE_TYPES__
