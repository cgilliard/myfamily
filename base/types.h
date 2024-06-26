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

#ifndef _TYPES_BASE__
#define _TYPES_BASE__

#include <stdbool.h>
#include <stdint.h>
#include <base/cleanup.h>

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

struct StringImpl {
	char *ptr;
	u64 len;
};
typedef struct StringImpl StringImpl;

void string_free(StringImpl *s);
#define String StringImpl CLEANUP(string_free)

int string_set(StringImpl *s, const char *ptr);


i64 saddi64(i64 a, i64 b);
u64 saddu64(u64 a, u64 b);

#endif /* _TYPES_BASE__ */
