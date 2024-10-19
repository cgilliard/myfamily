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

#define OBJECT_FLAGS_SEND 0x1
#define OBJECT_FLAG_NO_CLEANUP (0x1 << 1)
#define OBJECT_FLAG_CONSUMED (0x1 << 2)

#include <util/channel.h>
#include <util/object_macros.h>
#include <util/rc.h>

typedef struct ObjectNc {
	Rc impl;
	u8 flags;
} ObjectNc;

typedef enum ObjectType {
	ObjectTypeString,
	ObjectTypeU64,
	ObjectTypeObject,
} ObjectType;

void object_cleanup(const ObjectNc *ptr);

#define Object ObjectNc __attribute__((warn_unused_result, cleanup(object_cleanup)))

// move the object to a new memory location
Object object_move(const Object *src);
// create a reference counted reference of the object
Object object_ref(const Object *src);
Object object_create(bool send, ObjectType type, const void *primitive);
ObjectType object_type(const Object *obj);
int object_send(Object *obj, Channel *channel);

// model: two rbtrees. One sorts based on index, the other based on name. Inserting using the
// map-like function use inserts it into the index based tree in the last position.
// map-like functions

// returns Ok on success, nil on error. Error type is set with the fam_err functionality.
Object object_set_property(Object *obj, const char *name, const Object *value);
// returns object or nil if not found or there is an error. Error type is set with the fam_err
// functionality.
Object object_get_property(const Object *obj, const char *name);
// removes property and returns it if found, nil if not found or error. Error type is set with the
// fam_err functionality.
Object object_remove_property(Object *obj, const char *name);

// array-like functions
Object object_get_property_index(const Object *obj, u32 index);
Object object_remove_property_index(Object *obj, u32 index);
// insert at the specified index
Object object_set_property_index(Object *obj, const Object *value, u64 index);
Object object_insert_property_before_index(Object *obj, const char *name, const Object *value,
										   u64 index);
Object object_insert_property_after_index(Object *obj, const char *name, const Object *value,
										  u64 index);

// from these primitive functions, we can introduce multiple easier to use macros:
// $(x, "myprop", z); -> object_set_property(&x, "myprop", &z);
// let z = $(x, "myprop"); -> z = object_get_property(&x, "myprop");
// let z = $remove(x, "myprop"); -> let z = object_remove_property(&x, "myprop");
// $(x, 0, "myprop", z); -> object_set_property_index(&x, "myprop", &z, 0);

const char *object_as_string(const Object *obj);
u64 object_as_u64(const Object *obj);
void object_cleanup_thread_local();

// TODO: additional primitive types (U8 - U128, I8 - I128, Bool, F32, F64)
// TODO: ObjectTypeArray (this type will expose the indexing capabilities of objects)

#ifdef TEST
i64 get_global_orbtree_alloc_count();
i64 get_thread_local_orbtree_alloc_count();
u64 get_thread_local_orbtree_size();
u64 get_global_orbtree_size();
void object_cleanup_global();
#endif // TEST

#endif // _UTIL_OBJECT__
