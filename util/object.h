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
#include <util/object_macros.h>
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

Object object_move(const Object *src);
Object object_ref(const Object *src);
int object_create(Object *obj, bool send, ObjectType type, const void *primitive);
ObjectType object_type(const Object *obj);
int object_send(Object *obj, Channel *channel);
int object_set_property(Object *obj, const char *key, const Object *value);
Object object_get_property(const Object *obj, const char *key);
const char *object_as_string(const Object *obj);
int object_as_u64(const Object *obj, u64 *value);
void object_cleanup_thread_local();

#ifdef TEST
u64 get_thread_local_rbtree_size();
u64 get_global_rbtree_size();
void object_cleanup_global();
#endif // TEST

#endif // _UTIL_OBJECT__
