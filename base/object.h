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

#ifndef _BASE_OBJECT__
#define _BASE_OBJECT__

#include <base/macro_util.h>

typedef __int128_t ObjectNc;
void object_cleanup(const ObjectNc *obj);
#define Object \
	ObjectNc __attribute((warn_unused_result, cleanup(object_cleanup)))

typedef enum ObjectType {
	UInt,
	Int,
	Float,
	Box,
	Function,
	Err,
} ObjectType;

#define $(v)                                                                   \
	_Generic((v),                                                              \
		double: object_float(_Generic((v), double: v, default: 0)),            \
		float: object_float(_Generic((v), float: v, default: 0)),              \
		long long: object_int(_Generic((v), long long: v, default: 0)),        \
		int: object_int(_Generic((v), int: v, default: 0)),                    \
		unsigned long long: object_uint(                                       \
				 _Generic((v), unsigned long long: v, default: 0)),            \
		unsigned int: object_uint(_Generic((v), unsigned int: v, default: 0)), \
		char *: object_string(_Generic((v), char *: v, default: 0)),           \
		const char *: object_string(                                           \
				 _Generic((v), const char *: v, default: 0)),                  \
		unsigned char *: object_string(                                        \
				 _Generic((v), unsigned char *: v, default: 0)),               \
		const unsigned char *: object_string(                                  \
				 _Generic((v), const unsigned char *: v, default: 0)),         \
		default: object_function(_Generic((v),                                 \
		double: 0,                                                             \
		int: 0,                                                                \
		long long: 0,                                                          \
		unsigned long long: 0,                                                 \
		default: v)))

#define _(type, ...)

#define PROC_DEFAULT(code) \
	default: {             \
		_ret__ = (code);   \
	} break;

#define PROC_MATCH__(cond, ...)                                        \
	__VA_OPT__(case cond : ({ _ret__ = (__VA_ARGS__); }); break; NONE) \
	__VA_OPT__(PAREN) PROC_DEFAULT(cond) __VA_OPT__(PAREN_END)
#define PROC_MATCH_(...) PROC_MATCH__(__VA_ARGS__)
#define PROC_MATCH(ignore, v) PROC_MATCH_(EXPAND_ALL v)

#pragma clang diagnostic ignored "-Wcompound-token-split-by-macro"

#define match(obj, ...)                                                   \
	({                                                                    \
		ObjectType _t__ = object_type(&obj);                              \
		var _ret__;                                                       \
		switch (_t__) { FOR_EACH(PROC_MATCH, ignore, (), __VA_ARGS__); }; \
		_ret__;                                                           \
	})

#define Err(code) object_err(code)
#define $fn(v) value_of_checked(v, Function)
#define $int(obj) (*(long long *)value_of_checked(&obj, Int))
#define $is_err(obj) (object_type(&obj) == Err)
#define $kind(obj) (*(int *)value_of_checked(&obj, Err))
#define $float(obj) (*(double *)value_of_checked(&obj, Float))
#define $uint(obj) (*(unsigned long long *)value_of_checked(&obj, UInt))
#define let const Object
#define var Object

Object object_int(long long value);
Object object_uint(unsigned long long value);
Object object_float(double value);
Object object_function(void *fn);
Object object_string(const char *v);
Object object_err(int code);
Object box(long long size);
Object box_resize(Object *obj, long long size);
void *box_get_long_bytes(const Object *obj);
void *box_get_short_bytes(const Object *obj);
const void *value_of(const Object *obj);
const void *value_of_checked(const Object *obj, ObjectType expect);
const void *value_of_fn(const Object *obj);
ObjectType object_type(const Object *obj);

#endif	// _BASE_OBJECT__
