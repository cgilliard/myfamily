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

#ifndef _CORE_IO__
#define _CORE_IO__

#include <core/class.h>
#include <core/traits.h>

Result read_to_string_impl(Object *self);
Result read_exact_impl(Object *self, char *buf, u64 len);
Result write_all(void *self, char *buf, u64 len);
Result write_all_impl(Object *self, char *buf, u64 len);
Result write_fmt_impl(Object *self, char *fmt, ...);
Result mywrite(void *obj, char *buf, u64 limit);
Result myread(void *obj, char *buf, u64 limit);
Result read_to_string(void *obj);
Result read_exact(void *obj, char *buf, u64 limit);
Result myseek(void *obj, u64 pos);
Result myflush(void *obj);

Result read_fixed_bytes(void *obj, char *buf, u64 limit);
Result read_u8_impl(void *obj);
Result read_u16_impl(void *obj);
Result read_u32_impl(void *obj);
Result read_u64_impl(void *obj);
Result read_u128_impl(void *obj);
Result read_i8_impl(void *obj);
Result read_i16_impl(void *obj);
Result read_i32_impl(void *obj);
Result read_i64_impl(void *obj);
Result read_i128_impl(void *obj);
Result read_bool_impl(void *obj);
Result expect_u8_impl(void *obj, u8 value);
Result read_empty_bytes_impl(void *obj, u64 count);

Result write_fixed_bytes(void *obj, char *buf, u64 limit);
Result write_u8_impl(void *obj);
Result write_u16_impl(void *obj);
Result write_u32_impl(void *obj);
Result write_u64_impl(void *obj);
Result write_u128_impl(void *obj);
Result write_i8_impl(void *obj);
Result write_i16_impl(void *obj);
Result write_i32_impl(void *obj);
Result write_i64_impl(void *obj);
Result write_i128_impl(void *obj);
Result write_bool_impl(void *obj);
Result write_empty_bytes_impl(void *obj, u64 count);

#define TRAIT_SEEK(T) TRAIT_REQUIRED(T, Result, seek, T##Ptr *self, u64 pos)

#define TRAIT_READ(T)                                                          \
	TRAIT_REQUIRED(T, Result, read, T##Ptr *self, char *buf, u64 len)      \
	TRAIT_IMPL(T, read_to_string, read_to_string_impl)                     \
	TRAIT_IMPL(T, read_exact, read_exact_impl)

#define TRAIT_WRITE(T)                                                         \
	TRAIT_REQUIRED(T, Result, write, T##Ptr *self, char *buf, u64 len)     \
	TRAIT_REQUIRED(T, Result, flush, T##Ptr *self)                         \
	TRAIT_IMPL(T, write_all, write_all_impl)                               \
	TRAIT_IMPL(T, write_fmt, write_fmt_impl)

#define TRAIT_BUF_READER(T)                                                    \
	TRAIT_REQUIRED(T, Result, open, int n, ...)                            \
	TRAIT_REQUIRED(T, Result, fill_buf, T##Ptr *ptr)                       \
	TRAIT_REQUIRED(T, void, consume_buf, T##Ptr *ptr, u64 amt)             \
	TRAIT_IMPL(T, read_line, read_line_impl)                               \
	TRAIT_IMPL(T, read_until, read_until_impl)                             \
	TRAIT_IMPL(T, lines, lines_impl)                                       \
	TRAIT_SUPER(T, TRAIT_READ)

#define TRAIT_BIN_READER(T)                                                    \
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
	TRAIT_IMPL(T, read_bool, read_bool_impl)                               \
	TRAIT_IMPL(T, expect_u8, expect_u8_impl)                               \
	TRAIT_IMPL(T, read_empty_bytes, read_empty_bytes_impl)                 \
	TRAIT_SUPER(T, TRAIT_READ)

#define TRAIT_BIN_WRITER(T)                                                    \
	TRAIT_IMPL(T, write_u8, write_u8_impl)                                 \
	TRAIT_IMPL(T, write_i8, write_i8_impl)                                 \
	TRAIT_IMPL(T, write_u16, write_u16_impl)                               \
	TRAIT_IMPL(T, write_i16, write_i16_impl)                               \
	TRAIT_IMPL(T, write_u32, write_u32_impl)                               \
	TRAIT_IMPL(T, write_i32, write_i32_impl)                               \
	TRAIT_IMPL(T, write_u64, write_u64_impl)                               \
	TRAIT_IMPL(T, write_i64, write_i64_impl)                               \
	TRAIT_IMPL(T, write_u128, write_u128_impl)                             \
	TRAIT_IMPL(T, write_i128, write_i128_impl)                             \
	TRAIT_IMPL(T, write_bool, write_bool_impl)                             \
	TRAIT_IMPL(T, write_empty_bytes, write_empty_bytes_impl)               \
	TRAIT_SUPER(T, TRAIT_WRITE)

#endif // _CORE_IO__
