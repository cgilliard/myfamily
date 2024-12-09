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

#include <base/err.h>
#include <base/macro_util.h>
#include <base/orbtree.h>
#include <base/types.h>

typedef __int128_t ObjectNc;
void object_cleanup(const ObjectNc *obj);
#define Object \
	ObjectNc __attribute((warn_unused_result, cleanup(object_cleanup)))

#define DEFINE_OBJECT(e)                                                    \
	typedef enum ObjectType { FOR_EACH(SECOND, none, (, ), e) } ObjectType; \
	static const char *ObjectText[] = {                                     \
		FOR_EACH(SECOND_STRINGIFY, none, (, ), e)};

// clang-format off
#define OBJECT_TYPES \
	UInt, \
	Int, \
	Float, \
	Box, \
	Function, \
	Thread, \
	Err
// clang-format on

DEFINE_OBJECT(OBJECT_TYPES);

typedef struct BoxSlabData {
	OrbTree ordered;
	OrbTree seq;
	void *extended;
	unsigned long long pages;
	unsigned int ref_count;
	unsigned int weak_count;
} BoxSlabData;

#define OBJ_SLAB_SIZE (128 - SLAB_LIST_SIZE)
#define OBJ_SLAB_FREE_LIST_SIZE 1024
#define OBJ_BOX_USER_DATA_SIZE (OBJ_SLAB_SIZE - sizeof(BoxSlabData))

#define $(v)                                                                   \
	_Generic((v),                                                              \
		double: object_float(_Generic((v), double: v, default: 0)),            \
		float: object_float(_Generic((v), float: v, default: 0)),              \
		long long: object_int(_Generic((v), long long: v, default: 0)),        \
		int: object_int(_Generic((v), int: (v), default: 0)),                  \
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

#define set(obj, name, value)                                       \
	_Generic((value),                                               \
		Object: object_set_property(                                \
				 &obj, name,                                        \
				 _Generic((value), Object: &value, default: NULL)), \
		default: ({                                                 \
				 ret _fn__ = object_function(value);                \
				 object_set_property(&obj, name, &_fn__);           \
			 }))
#define get(obj, name) object_get_property(&obj, name)
#define rem(obj, name) object_remove_property(&obj, name)

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
#define $fn(v) value_of_checked(&v, Function)
#define $int(obj) (*(long long *)value_of_checked(&obj, Int))
#define $is_err(obj) (object_type(&obj) == Err)
#define $err(obj) (FamErrText[(object_aux(&obj))])
#define $backtrace(obj) (object_bt_ptr(&obj))
#define $kind(obj) (object_aux(&obj))
#define $float(obj) (*(double *)value_of_checked(&obj, Float))
#define $uint(obj) (*(unsigned long long *)value_of_checked(&obj, UInt))
#define let const Object
#define var Object
#define ret ObjectNc

Object object_int(long long value);
Object object_uint(unsigned long long value);
Object object_float(double value);
Object object_function(void *fn);
Object object_string(const char *v);
Object object_err(int code);
const char *object_bt_ptr(const Object *obj);
Object box(long long size);
Object box_resize(Object *obj, long long size);
unsigned int object_aux(const Object *obj);
void *box_get_long_bytes(const Object *obj);
void *box_get_short_bytes(const Object *obj);
unsigned long long box_get_page_count(const Object *obj);
const void *value_of(const Object *obj);
const void *value_of_checked(const Object *obj, ObjectType expect);
const void *value_of_fn(const Object *obj);
ObjectType object_type(const Object *obj);
Object object_get_property(const Object *obj, const char *name);
Object object_set_property(Object *obj, const char *name, const Object *value);
Object object_remove_property(Object *obj, const char *name);
Object object_move(Object *obj);
Object object_ref(Object *obj);

// Threading
#define $thread(stack_size) object_thread(stack_size)
#define $start(th, arg)                    \
	({                                     \
		let _obj__ = object_move(&arg);    \
		object_thread_start(&th, &_obj__); \
	})
#define $join(th) object_thread_join(&th)
typedef Object (*ThreadFn)(Object *arg);
Object object_thread(u64 stack_size);
Object object_thread_start(Object *obj, Object *arg);
Object object_thread_signal(const Object *obj);
Object object_thread_ref(Object *obj);
Object object_thread_join(Object *obj);
void object_thread_cleanup(const Object *obj);
u64 object_thread_id(const Object *obj);

#endif	// _BASE_OBJECT__
