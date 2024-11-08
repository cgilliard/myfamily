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

#include <base/types.h>

typedef enum ObjectType {
	ObjectTypeInt,
	ObjectTypeFloat,
	ObjectTypeByte,
	ObjectTypeBool,
	ObjectTypeBox,
	ObjectTypeErr,
	ObjectTypeFunction,
} ObjectType;

#define OBJECT_IMPL_SIZE 64
typedef struct Object {
	byte impl[OBJECT_IMPL_SIZE];
} Object;

Object object_create_int(int value);
Object object_create_float(float value);
Object object_create_byte(byte value);
Object object_create_bool(bool value);
Object object_create_err(int value);
Object object_create_function(void *fn);
Object object_create_box(unsigned long long size);

int object_value_of_primitive(const Object *obj);
void *object_value_of_box(const Object *obj);

Object object_set_property(Object obj, const char *key, const Object value);
Object object_delete_property(Object obj, const char *key);
Object object_get_property(const Object obj, const char *key);

// moving/referencing functions
Object object_move(const Object src);
Object object_ref(const Object src);
Object object_weak(const Object src);
Object object_upgrade(const Object src);

#endif	// _BASE_OBJECT__
