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

typedef enum ObjectType {
	ObjectType64Bit,
	ObjectType32Bit,
	ObjectType8Bit,
	ObjectTypePointer,
	ObjectTypeObject,
} ObjectType;

// Example usage:
// Object x = object_create(&(int) { 1 }, ObjectType32);
// Object y = object_create(&(u64) {1LL}, ObjectType64);
// a $() macro could autobox these: let x = $(1); or let y = $(1LL);
// int x_int = *(int *)object_value_of(x);
// int64 y_int = *(int64)object_value_of(y);
// a $int() macro or $int64() macro could do this automatically and do a type check.
// int x_int = $int(x); or int64 y_int = $int64(y); int z_int = $int64(x); would be an error,
// possibly print a warning, or maybe panic
Object object_create(const void *value, ObjectType type);
const void *object_value_of(const Object obj);
ObjectType object_type(const Object obj);
unsigned int object_size(const Object obj);

int object_mutate(Object obj, const void *value);

// A vector is created like this:
// Ptr x = fam_alloc(100, false);
// let v = object_create(x, sizeof(Ptr), ObjectTypePointer);
// // resize
// Ptr vptr = object_value_of(x);
// Ptr nvptr = fam_resize(vptr, 200);
// // check if nvptr is NULL (error condition)
// int res = object_mutate(v, nvptr);
// // check for errors
// // if successful, v now has 200 bytes allocated.
// higher level types can use this functionality to build Vectors, Slices, Strings, etc.

// property functions
int object_set_property(Object obj, const char *key, const Object value);
Object object_delete_property(Object obj, const char *key);
Object object_get_property(const Object obj, const char *key);

// moving/referencing functions
Object object_move(const Object src);
Object object_ref(const Object src);
