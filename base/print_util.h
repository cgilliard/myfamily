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

// #include <base/backtrace.h>
#include <base/macro_util.h>
#include <base/stream.h>
#include <base/string.h>
#include <base/types.h>

typedef enum PrintType {
	PrintTypeNum,
	PrintTypeInt,
	PrintTypeDec,
	PrintTypeCh,
	PrintTypeString,
	PrintTypeBool,
	PrintTypeTerm,
} PrintType;

typedef struct PrintPair {
	PrintType type;
	ch buf[8];
	void *data;
} PrintPair;

static const PrintPair __termination_print_pair__ = {.type = PrintTypeTerm};

#define BUILD_PRINT_PAIR(ignore, v)                                                                \
	_Generic((v),                                                                                  \
		ch: ({                                                                                     \
				 PrintPair pair = {.type = PrintTypeCh};                                           \
				 memcpy(pair.buf, &(ch) {(ch)v}, sizeof(ch));                                      \
				 pair;                                                                             \
			 }),                                                                                   \
		num: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeI64};                                          \
				 memcpy(pair.buf, &(num) {(num)v}, sizeof(num));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		int: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeInt};                                          \
				 memcpy(pair.buf, &(int) {(int)v}, sizeof(int));                                   \
				 pair;                                                                             \
			 }),                                                                                   \
		bool: ({                                                                                   \
				 PrintPair pair = {.type = PrintTypeBool};                                         \
				 memcpy(pair.buf, &(bool) {(bool)v}, sizeof(bool));                                \
				 pair;                                                                             \
			 }),                                                                                   \
		dec: ({                                                                                    \
				 PrintPair pair = {.type = PrintTypeF64};                                          \
				 dec val = _Generic((v), dec: v, default: 0.0);                                    \
				 memcpy(pair.buf, &(dec) {val}, sizeof(dec));                                      \
				 pair;                                                                             \
			 }),                                                                                   \
		ch *: ({                                                                                   \
				 u8 *val = _Generic((v), ch *: v, default: NULL);                                  \
				 (const PrintPair) {.type = PrintTypeString, .data = val};                         \
			 }),                                                                                   \
		const ch *: ({                                                                             \
				 u8 *val = _Generic((v), const ch *: v, default: NULL);                            \
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
		default: (const PrintPair) {.type = PrintTypeNum, .buf = 0})

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

num print_impl(const Stream *strm, ch *s, int capacity, bool nl, bool exit, int code,
			   const ch *prefix, const ch *fmt, ...);

#endif // _BASE_PRINT_UTIL__
