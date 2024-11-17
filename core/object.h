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

#ifndef _CORE_OBJECT__
#define _CORE_OBJECT__

#include <base/types.h>

typedef enum ObjectType {
	ObjectTypeUnit,
	ObjectTypeInt,
	ObjectTypeFloat,
	ObjectTypeByte,
	ObjectTypeBool,
	ObjectTypeBox,
	ObjectTypeErr,
	ObjectTypeFunction,
} ObjectType;

#define OBJECT_IMPL_SIZE 16
typedef struct ObjectNc {
	byte impl[OBJECT_IMPL_SIZE];
} ObjectNc;

void object_cleanup(const ObjectNc *obj);

#define Object \
	ObjectNc __attribute((warn_unused_result, cleanup(object_cleanup)))

Object object_create_int(int value);
Object object_create_float(float value);
Object object_create_byte(byte value);
Object object_create_bool(bool value);
Object object_create_err(int value);
Object object_create_function(void *fn);
Object object_create_box(unsigned int size);
Object object_create_unit();

int object_value_of(const Object *obj);
ObjectType object_type(const Object *obj);
void *object_box_sso(const Object *obj);
void *object_box_extended(const Object *obj);
unsigned int object_box_size(const Object *obj);
Object object_resize_box(Object *obj, unsigned int size);
void *object_value_function(const Object *obj);

Object object_set_property(Object *obj, const char *key, const Object *value);
Object object_delete_property(Object *obj, const char *key);
Object object_get_property(const Object *obj, const char *key);

// moving/referencing functions
Object object_move(const Object src);
Object object_ref(const Object src);
Object object_weak(const Object src);
Object object_upgrade(const Object src);

#endif	// _CORE_OBJECT__
