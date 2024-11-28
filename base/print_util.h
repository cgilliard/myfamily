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
	PrintTypeTerm,
} PrintType;

typedef struct Printable {
	PrintType type;
	union {
		long long int_value;
		unsigned long long uint_value;
		double float_value;
		Object object_value;
		char *string_value;
	} value;
} Printable;

static const Printable __termination_print_pair__ = {.type = PrintTypeTerm};

#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

#define BUILD_PRINTABLE(ignore, v)                                             \
	_Generic((v),                                                              \
		Object: (const Printable){.type = PrintTypeObject,                     \
								  .value.object_value = (__int128_t)(v)},      \
		int: (const Printable){.type = PrintTypeInt,                           \
							   .value.int_value = (long long)(v)},             \
		long long: (const Printable){.type = PrintTypeInt,                     \
									 .value.int_value = (long long)(v)},       \
		unsigned long: (const Printable){.type = PrintTypeUInt,                \
										 .value.uint_value =                   \
											 (unsigned long long)(v)},         \
		unsigned long long: (const Printable){.type = PrintTypeUInt,           \
											  .value.uint_value =              \
												  (unsigned long long)(v)},    \
		double: (const Printable){.type = PrintTypeFloat,                      \
								  .value.float_value =                         \
									  _Generic((v), double: v, default: 0.0)}, \
		float: (const Printable){.type = PrintTypeFloat,                       \
								 .value.float_value =                          \
									 _Generic((v), float: v, default: 0.0)},   \
		default: (const Printable){                                            \
			.type = PrintTypeString,                                           \
			.value.string_value =                                              \
				_Generic((v), char *: (v), const char *: (v), default: 0)})

long long print_impl(Channel *channel, char *buffer, long long capacity,
					 int newline, int exit, const char *prefix, const char *fmt,
					 ...);

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

#define print(fmt, ...) print_impl(&STDOUT, 0, 0, 1, 0, 0, fmt, ##__VA_ARGS__)

#define sprint(buffer, capacity, fmt, ...) \
	print_impl(0, buffer, capacity, 0, 0, 0, fmt, ##__VA_ARGS__)

#endif	// _BASE_PRINT_UTIL__
