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

#include <base/macros.h>
#include <base/types.h>

Type(Object);
#define Object DefineType(Object)

typedef enum ObjectType {
	ObjectTypeInt,
	ObjectTypeFloat,
	ObjectTypeByte,
	ObjectTypeBox,
	ObjectTypeBool,
	ObjectTypeWeak,
	__ObjectTypeCount__,
} ObjectType;

Object object_create_box(unsigned int size, bool send);
Object object_create(ObjectType type, const void *value, bool send);
int object_value_of(const Object obj, void *buffer, unsigned int limit);
ObjectType object_type(const Object obj);

int object_resize(Object obj, unsigned int size);
int object_set_property(Object obj, const char *key, const Object value);
Object object_delete_property(Object obj, const char *key);
Object object_get_property(const Object obj, const char *key);

// moving/referencing functions
Object object_move(const Object src);
Object object_ref(const Object src);
Object object_weak(const Object src);
Object object_upgrade(const Object src);
