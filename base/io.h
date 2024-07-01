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

#ifndef _BASE_IO__
#define _BASE_IO__

#include <base/class.h>
#include <base/traits.h>

Result read_to_string_impl(Object *self);
Result read_exact_impl(Object *self, char *buf, u64 len);
Result myread(void *obj, char *buf, u64 limit);
Result read_to_string(void *obj);
Result read_exact(void *obj, char *buf, u64 limit);
Result myseek(void *obj, u64 pos);

#define TRAIT_SEEK(T) TRAIT_REQUIRED(T, Result, seek, T##Ptr *self, u64 pos)

#define TRAIT_READ(T)                                                          \
	TRAIT_REQUIRED(T, Result, read, T##Ptr *self, char *buf, u64 limit)    \
	TRAIT_IMPL(T, read_to_string, read_to_string_impl)                     \
	TRAIT_IMPL(T, read_exact, read_exact_impl)

#define TRAIT_READER(T)                                                        \
	TRAIT_REQUIRED(T, Result, read_fixed_bytes, T##Ptr *self, char *buf,   \
		       u64 limit)                                              \
	TRAIT_IMPL(T, read_u8, read_u8_impl)                                   \
	TRAIT_IMPL(T, read_i8, read_i8_impl)                                   \
	TRAIT_IMPL(T, read_u16, read_u16_impl)                                 \
	TRAIT_IMPL(T, read_i16, read_i16_impl)                                 \
	TRAIT_IMPL(T, read_u32, read_u32_impl)                                 \
	TRAIT_IMPL(T, read_i32, read_i32_impl)                                 \
	TRAIT_IMPL(T, read_u64, read_u64_impl)                                 \
	TRAIT_IMPL(T, read_i64, read_i64_impl)                                 \
	TRAIT_IMPL(T, read_u128, read_u128_impl)                               \
	TRAIT_IMPL(T, read_i128, read_i128_impl)                               \
	TRAIT_IMPL(T, read_usize, read_usize_impl)                             \
	TRAIT_IMPL(T, read_bool, read_bool_impl)                               \
	TRAIT_IMPL(T, expect_u8, expect_u8_impl)                               \
	TRAIT_IMPL(T, read_empty_bytes, read_empty_bytes_impl)

#endif // _BASE_IO__
