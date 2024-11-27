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

#include <base/object.h>
#include <base/print_util.h>

typedef struct ObjectImpl {
	unsigned long long type;
	union {
		unsigned char bytes[8];
		long long int_value;
		unsigned long long uint_value;
		double float_value;
		int code_value;
		void *ptr_value;
	} value;
} ObjectImpl;

void __attribute__((constructor)) __confirm_object_impl() {
	if (sizeof(ObjectImpl) != sizeof(__int128_t)) {
		panic("sizeof(ObjectImpl) (%lu) != sizeof(Object) (%lu)",
			  sizeof(ObjectImpl), sizeof(Object));
	}
}

Object object_int(long long value) {
	ObjectImpl ret = {.type = Int};
	ret.value.int_value = value;
	return *((Object *)&ret);
}

Object object_uint(unsigned long long value) {
	ObjectImpl ret = {.type = UInt};
	ret.value.uint_value = value;
	return *((Object *)&ret);
}

Object object_float(double value) {
	ObjectImpl ret = {.type = Float};
	ret.value.float_value = value;
	return *((Object *)&ret);
}

Object object_function(void *fn) {
	ObjectImpl ret = {.type = Function};
	ret.value.ptr_value = fn;
	return *((Object *)&ret);
}

Object object_err(int code) {
	ObjectImpl ret = {.type = Err};
	ret.value.code_value = code;
	return *((Object *)&ret);
}

Object box(long long size) {
	return 0;
}

const void *value_of(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	if (impl->type == Function) return impl->value.ptr_value;
	return &impl->value.bytes;
}

ObjectType object_type(const Object *obj) {
	return ((ObjectImpl *)obj)->type;
}
