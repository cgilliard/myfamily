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

#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/object.h>
#include <base/string.h>

#include <stdio.h>

#define OBJECT_FLAG_FAM_ALLOC_RESERVED1 0
#define OBJECT_FLAG_FAM_ALLOC_RESERVED2 1
#define OBJECT_FLAG_TYPE0 2
#define OBJECT_FLAG_TYPE1 3
#define OBJECT_FLAG_TYPE2 4

Object object_create(const void *value, ObjectType type) {
	return NULL;
}
const void *object_value_of(const Object obj) {
	return NULL;
}
ObjectType object_type(const Object obj) {
	return ObjectTypeObject;
}
unsigned int object_size(const Object obj) {
	return 0;
}

int object_mutate(Object obj, const void *value) {
	return 0;
}

int object_set_property(Object obj, const char *key, const Object value) {
	return 0;
}
Object object_delete_property(Object obj, const char *key) {
	return NULL;
}
Object object_get_property(const Object obj, const char *key) {
	return NULL;
}

Object object_move(const Object src) {
	return NULL;
}
Object object_ref(const Object src) {
	return NULL;
}
