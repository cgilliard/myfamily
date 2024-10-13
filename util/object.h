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

#ifndef _UTIL_OBJECT__
#define _UTIL_OBJECT__

#include <base/panic.h>
#include <util/channel.h>
#include <util/rc.h>

typedef struct ObjectNc {
	Rc impl;
	FatPtr flags;
} ObjectNc;

typedef enum ObjectType {
	ObjectTypeString,
	ObjectTypeU64,
	ObjectTypeObject,
} ObjectType;

void object_cleanup(const ObjectNc *ptr);

#define Object ObjectNc __attribute__((warn_unused_result, cleanup(object_cleanup)))

int object_move(Object *dst, const Object *src);
int object_ref(Object *dst, Object *src);
int object_mut_ref(Object *dst, Object *src);
int object_create(Object *obj, bool send, ObjectType type, const void *primitive);
int object_send(Object *obj, Channel *channel);
int object_set_property(Object *obj, const char *key, const Object *value);
Object object_get_property(const Object *obj, const char *key);
const char *object_as_string(const Object *obj);
int object_as_u64(const Object *obj, u64 *value);

#define let const Object
#define var Object

#define NIL {.impl = rc_null, .flags = null}

#define $objnil(v) fat_ptr_is_nil(&(&((ObjectNc *)&(v))->impl)->impl)
#define obj_nil(v)                                                                                 \
	_Generic((v),                                                                                  \
		Object: $objnil(v),                                                                        \
		FatPtr: fat_ptr_is_nil((FatPtr *)&v),                                                      \
		Rc: fat_ptr_is_nil(&((Rc *)&v)->impl),                                                     \
		default: ({}))
#undef nil
#define nil(v) obj_nil(v)

#define $object_impl(v)                                                                            \
	_Generic((v),                                                                                  \
		Object: ({                                                                                 \
				 ObjectNc _ret__ = NIL;                                                            \
				 object_create(&_ret__, false, ObjectTypeObject, NULL);                            \
				 _ret__;                                                                           \
			 }),                                                                                   \
		default: $rc_impl(v))
#undef $
#define $(...) $object_impl(__VA_ARGS__)

#ifdef TEST
void object_cleanup_global();
#endif // TEST

#endif // _UTIL_OBJECT__
