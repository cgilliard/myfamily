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

#ifndef _BASE_PRINT_UTIL__
#define _BASE_PRINT_UTIL__

#include <base/backtrace.h>
#include <base/macro_util.h>
#include <base/stream.h>
#include <base/string.h>
#include <base/types.h>

typedef enum PrintType {
	PrintTypeU128,
	PrintTypeU64,
	PrintTypeU32,
	PrintTypeU16,
	PrintTypeU8,
	PrintTypeI128,
	PrintTypeI64,
	PrintTypeI32,
	PrintTypeI16,
	PrintTypeI8,
	PrintTypeBool,
	PrintTypeF64,
	PrintTypeF32,
	PrintTypeString,
	PrintTypeTerm,
} PrintType;

typedef struct PrintPair {
	PrintType type;
	u8 buf[16];
	void *data;
} PrintPair;

static const PrintPair __termination_print_pair__ = {.type = PrintTypeTerm};

typedef struct F64Wrap {
	f64 value;
} F64Wrap;

#define F64(v)                                                                                     \
	({                                                                                             \
		F64Wrap _wrap__ = {v};                                                                     \
		_wrap__;                                                                                   \
	})

#define BUILD_PRINT_PAIR(ignore, v)                                                                \
	_Generic((v),                                                                                  \
		u64: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeU64};                                          \
				 memcpy(pair.buf, &(u64) {(u64)v}, sizeof(u64));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		u32: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeU32};                                          \
				 memcpy(pair.buf, &(u32) {(u32)v}, sizeof(u32));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		u16: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeU16};                                          \
				 memcpy(pair.buf, &(u16) {(u16)v}, sizeof(u16));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		u8: ({                                                                                     \
				 PrintPair pair = {.type = PrintTypeU8};                                           \
				 memcpy(pair.buf, &(u8) {(u8)v}, sizeof(u8));                                      \
				 pair;                                                                             \
			 }),                                                                                   \
		i64: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeI64};                                          \
				 memcpy(pair.buf, &(i64) {(i64)v}, sizeof(i64));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		i32: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeI32};                                          \
				 memcpy(pair.buf, &(i32) {(i32)v}, sizeof(i32));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		i16: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeI16};                                          \
				 memcpy(pair.buf, &(i16) {(i16)v}, sizeof(i16));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		i8: ({                                                                                     \
				 PrintPair pair = {.type = PrintTypeI8};                                           \
				 memcpy(pair.buf, &(i8) {(i8)v}, sizeof(i8));                                      \
				 pair;                                                                             \
			 }),                                                                                   \
		bool: ({                                                                                   \
				 PrintPair pair = {.type = PrintTypeBool};                                         \
				 memcpy(pair.buf, &(bool) {(bool)v}, sizeof(bool));                                \
				 pair;                                                                             \
			 }),                                                                                   \
		f64: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeF64};                                          \
				 f64 val = _Generic((v), f64: v, default: 0.0);                                    \
				 memcpy(pair.buf, &(f64) {val}, sizeof(f64));                                      \
				 pair;                                                                             \
			 }),                                                                                   \
		f32: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeF32};                                          \
				 f32 val = _Generic((v), f32: v, default: 0.0);                                    \
				 memcpy(pair.buf, &(f32) {val}, sizeof(f32));                                      \
				 pair;                                                                             \
			 }),                                                                                   \
		u8 *: ({                                                                                   \
				 u8 *val = _Generic((v), u8 *: v, default: NULL);                                  \
				 (const PrintPair) {.type = PrintTypeString, .data = val};                         \
			 }),                                                                                   \
		const u8 *: ({                                                                             \
				 u8 *val = _Generic((v), const u8 *: v, default: NULL);                            \
				 (const PrintPair) {.type = PrintTypeString, .data = val};                         \
			 }),                                                                                   \
		i8 *: ({                                                                                   \
				 i8 *val = _Generic((v), i8 *: v, default: NULL);                                  \
				 (const PrintPair) {.type = PrintTypeString, .data = val};                         \
			 }),                                                                                   \
		const i8 *: ({                                                                             \
				 i8 *val = _Generic((v), const i8 *: v, default: NULL);                            \
				 (const PrintPair) {.type = PrintTypeString, .data = val};                         \
			 }),                                                                                   \
		char *: ({                                                                                 \
				 char *val = _Generic((v), char *: v, default: NULL);                              \
				 (const PrintPair) {.type = PrintTypeString, .data = val};                         \
			 }),                                                                                   \
		const char *: ({                                                                           \
				 char *val = _Generic((v), const char *: v, default: NULL);                        \
				 (const PrintPair) {.type = PrintTypeString, .data = val};                         \
			 }),                                                                                   \
		default: (const PrintPair) {.type = PrintTypeU64, .buf = 0})

const static Stream __slen__impl__ = {-1};
const static Stream *__slen__ = &__slen__impl__;

#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif
#define __do_print_impl_(strm, s, capacity, nl, exit, code, prefix, fmt, ...)                      \
	print_impl(strm, s, capacity, nl, exit, code, prefix,                                          \
			   fmt __VA_OPT__(, ) FOR_EACH(BUILD_PRINT_PAIR, ignore, (, ), __VA_ARGS__),           \
			   __termination_print_pair__)

#define print(fmt, ...)                                                                            \
	__do_print_impl_(out_strm, NULL, UINT32_MAX, false, false, 0, NULL, fmt, __VA_ARGS__)
#define println(fmt, ...)                                                                          \
	__do_print_impl_(out_strm, NULL, UINT32_MAX, true, false, 0, NULL, fmt, __VA_ARGS__)

#define printerr(fmt, ...)                                                                         \
	__do_print_impl_(err_strm, NULL, UINT32_MAX, false, false, 0, NULL, fmt, __VA_ARGS__)
#define printerrln(fmt, ...)                                                                       \
	__do_print_impl_(err_strm, NULL, UINT32_MAX, true, false, 0, NULL, fmt, __VA_ARGS__)

#define sprint(s, len, fmt, ...)                                                                   \
	__do_print_impl_(out_strm, s, len, false, false, 0, NULL, fmt, __VA_ARGS__)
#define sprintln(s, len, fmt, ...)                                                                 \
	__do_print_impl_(out_strm, s, len, true, false, 0, NULL, fmt, __VA_ARGS__)

#define slen(fmt, ...) __do_print_impl_(__slen__, " ", 2, false, false, 0, NULL, fmt, __VA_ARGS__)

#define panic(fmt, ...)                                                                            \
	__do_print_impl_(err_strm, NULL, UINT32_MAX, true, true, -1, "Panic: ", fmt, __VA_ARGS__)

i32 print_impl(const Stream *strm, u8 *s, i32 capacity, bool nl, bool exit, i32 code,
			   const u8 *prefix, const u8 *fmt, ...);

#endif // _BASE_PRINT_UTIL__
