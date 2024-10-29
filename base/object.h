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
#include <base/string.h>
#include <base/types.h>

Type(Object);
#define Object DefineType(Object)

/*

typedef enum ObjectType {
	ObjectTypeInt32,
	ObjectTypeInt64,
	ObjectTypeFloat,
	ObjectTypeByte,
	ObjectTypeBool,
	ObjectTypeVector,
	ObjectTypeMap,
	ObjectTypeSlice
} ObjectType;

// Example usage:
// Object x = object_create(ObjectTypeInt32, 1);
// Object x = object_create(ObjectTypeInt64, 1LL);
// Object x = object_create(ObjectTypeBool, false);
// Object x = object_create(ObjectTypeFloat, 1.2);
// Object x = object_create(ObjectTypeByte, 'a');
Object object_create(ObjectType type, const void *value);
// allowed functions for the primitive types:
int object_as_int(const Object obj);
int64 object_as_int64(const Object obj);
bool object_as_bool(const Object obj);
byte object_as_byte(const Object obj);
float64 object_as_float(const Object obj);

// Example usage:
// unsigned char buf[100];
// // populate buf with data...
// Object x = object_create_vector(buf, ObjectTypeByte, 100);
Object object_create_vector(const void *value, ObjectType element_type, int64 len);
// allowed functions
int object_append(Object vec, const Object element);
int object_append_raw(Object vec, const byte *value);
int object_resize(Object vec, int64 nlen);
int object_set(Object vec, int64 element, Object value);
int object_set_raw(Object vec, int64 element, const byte *value);
int object_set_raw_range(Object vec, int64 start, int64 len, const byte *value);
int object_delete_element(Object vec, int64 index);
int object_delete_range(Object vec, int64 start, int64 len);
// Object y = object_create_slice(x, 10, 10); // slice of x[10..20].
Object object_create_slice(Object vector, int64 offset, int64 len);
// allowed functions:
const void *object_slice_ref(Object slice);
// Creates a map. To create an immutable map, you can
Object object_create_map();
int object_set_property(Object obj, const Object key, const Object value);
Object object_delete_property(Object obj, const Object key);
Object object_get_property(const Object obj, const Object key);
Object object_move(Object src);
Object object_ref(Object src);
*/

Object object_create_int64(int64 value);
Object object_create_int(int value);
Object object_create_byte(byte value);
Object object_create_string(string s);
Object object_move(Object src);
Object object_ref(Object src);
int64 object_as_int64(const Object obj);
int object_as_int(const Object obj);
byte object_as_byte(const Object obj);
string object_as_string(const Object obj);
