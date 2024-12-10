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

#include <base/channel.h>
#include <base/macro_util.h>
#include <base/object.h>

#define va_list __builtin_va_list

typedef enum PrintType {
	PrintTypeInt,
	PrintTypeUInt,
	PrintTypeFloat,
	PrintTypeString,
	PrintTypeObject,
	PrintTypeByte,
	PrintTypeTerm,
} PrintType;

typedef struct Printable {
	PrintType type;
	union {
		i64 int_value;
		u64 uint_value;
		double float_value;
		Object object_value;
		char *string_value;
	} value;
} Printable;

static const Printable __termination_print_pair__ = {.type = PrintTypeTerm};

#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma clang diagnostic ignored \
	"-Wincompatible-pointer-types-discards-qualifiers"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wint-conversion"

#define BUILD_PRINTABLE(ignore, v)                                      \
	_Generic((v),                                                       \
		Object: (const Printable){.type = PrintTypeObject,              \
								  .value.object_value = (i128)(v)},     \
		char: (const Printable){.type = PrintTypeInt,                   \
								.value.int_value = (i64)(v)},           \
		byte: (const Printable){.type = PrintTypeInt,                   \
								.value.int_value = (i64)(v)},           \
		int: (const Printable){.type = PrintTypeInt,                    \
							   .value.int_value = (i64)(v)},            \
		i64: (const Printable){.type = PrintTypeInt,                    \
							   .value.int_value = (i64)(v)},            \
		unsigned int: (const Printable){.type = PrintTypeUInt,          \
										.value.uint_value = (u64)(v)},  \
		unsigned long: (const Printable){.type = PrintTypeUInt,         \
										 .value.uint_value = (u64)(v)}, \
		u64: (const Printable){.type = PrintTypeUInt,                   \
							   .value.uint_value = (u64)(v)},           \
		default: (const Printable){.type = PrintTypeString,             \
								   .value.string_value = (v)})
i64 print_impl(Channel *channel, char *buffer, i64 capacity, int newline,
			   int exit, const char *prefix, const char *fmt, ...);
i64 sprint_impl(char *buffer, i64 capacity, const char *fmt, ...);

#define panic(fmt, ...)                                                  \
	print_impl(&STDERR, 0, 0, 1, 1, "Panic: ",                           \
			   fmt __VA_OPT__(, )                                        \
				   FOR_EACH(BUILD_PRINTABLE, ignore, (, ), __VA_ARGS__), \
			   __termination_print_pair__)

#define println(fmt, ...)                                                \
	print_impl(&STDOUT, 0, 0, 1, 0, 0,                                   \
			   fmt __VA_OPT__(, )                                        \
				   FOR_EACH(BUILD_PRINTABLE, ignore, (, ), __VA_ARGS__), \
			   __termination_print_pair__)

#define print(fmt, ...)                                                  \
	print_impl(&STDOUT, 0, 0, 0, 0, 0,                                   \
			   fmt __VA_OPT__(, )                                        \
				   FOR_EACH(BUILD_PRINTABLE, ignore, (, ), __VA_ARGS__), \
			   __termination_print_pair__)

#define sprint(buffer, capacity, fmt, ...)                                \
	sprint_impl(buffer, capacity,                                         \
				fmt __VA_OPT__(, )                                        \
					FOR_EACH(BUILD_PRINTABLE, ignore, (, ), __VA_ARGS__), \
				__termination_print_pair__)

#endif	// _BASE_PRINT_UTIL__
