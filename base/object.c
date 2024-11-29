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

#include <base/err.h>
#include <base/object.h>
#include <base/print_util.h>
#include <base/slabs.h>

SlabAllocator object_slabs;

typedef struct ObjectImpl {
	union {
		unsigned char bytes[8];
		long long int_value;
		unsigned long long uint_value;
		double float_value;
		int code_value;
		void *ptr_value;
	} value;
	unsigned long long type;
} ObjectImpl;

void __attribute__((constructor)) __setup_object_impl() {
	if (sizeof(ObjectImpl) != sizeof(__int128_t)) {
		panic("sizeof(ObjectImpl) ({}) != sizeof(Object) ({})",
			  sizeof(ObjectImpl), sizeof(Object));
	}

	let res = slab_allocator_init(&object_slabs, 100, 100, 100);
}

Object object_int(long long value) {
	ObjectImpl ret = {.type = Int, .value.int_value = value};
	return *((Object *)&ret);
}

Object object_uint(unsigned long long value) {
	ObjectImpl ret = {.type = UInt, .value.uint_value = value};
	return *((Object *)&ret);
}

Object object_float(double value) {
	ObjectImpl ret = {.type = Float, .value.float_value = value};
	return *((Object *)&ret);
}

Object object_function(void *fn) {
	ObjectImpl ret = {.type = Function, .value.ptr_value = fn};
	return *((Object *)&ret);
}

Object object_err(int code) {
	ObjectImpl ret = {.type = Err, .value.code_value = code};
	return *((Object *)&ret);
}

Object object_string(const char *s) {
	ObjectImpl ret = {.type = Err, .value.code_value = NotYetImplemented};
	return *((Object *)&ret);
}

Object box(long long size) {
	ObjectImpl ret = {.type = Box, .value.code_value = NotYetImplemented};
	return *((Object *)&ret);
}

const void *value_of(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	if (impl->type == Function) return impl->value.ptr_value;
	return &impl->value.bytes;
}

const void *value_of_checked(const Object *obj, ObjectType expect) {
	ObjectType type = object_type(obj);
	if (type != expect)
		panic("Expected Object type {}. Found {}!", (int)expect, (int)type);
	return value_of(obj);
}

ObjectType object_type(const Object *obj) {
	return ((ObjectImpl *)obj)->type;
}

void object_cleanup(const Object *obj) {
}
