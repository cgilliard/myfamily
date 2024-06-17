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

#include <base/traits.h>

bool equal(void *obj1, void *obj2) {
	if (((Object *)obj1)->vdata.vtable->id !=
	    ((Object *)obj2)->vdata.vtable->id) {
		return false;
	}
	bool (*do_equal)(Object *obj1, Object *obj2) =
	    find_fn((Object *)obj1, "equal");

	if (do_equal == NULL)
		panic("equal not implemented for this type");
	return do_equal(obj1, obj2);
}

void *unwrap(void *obj) {
	void *(*do_unwrap)(Object *obj) = find_fn((Object *)obj, "unwrap");
	if (do_unwrap == NULL)
		panic("unwrap not implemented for this type");
	return do_unwrap(obj);
}

char *to_str(void *obj) {
	char *(*do_to_str)(Object *obj) = find_fn((Object *)obj, "to_str");
	if (do_to_str == NULL)
		panic("to_str not implemented for this type");
	return do_to_str(obj);
}

void print(void *obj) {
	char *(*do_print)(Object *obj) = find_fn((Object *)obj, "print");
	if (do_print == NULL)
		panic("print not implemented for this type");
	do_print(obj);
}

void *unwrap_err(void *obj) {
	void *(*do_unwrap)(Object *obj) = find_fn((Object *)obj, "unwrap_err");
	if (do_unwrap == NULL)
		panic("unwrap_err not implemented for this type");
	return do_unwrap(obj);
}

#define IMPL_COPY_IMPL(type)                                                   \
	bool copy_impl_##type(type *dst, type *src) {                          \
		memcpy(dst, src, sizeof(type));                                \
		return true;                                                   \
	}                                                                      \
	size_t size_impl_##type(type *s) { return sizeof(type); }              \
	void cleanup_impl_##type(type *s) {}

IMPL_COPY_IMPL(f64);
IMPL_COPY_IMPL(f32);
IMPL_COPY_IMPL(i128);
IMPL_COPY_IMPL(i64);
IMPL_COPY_IMPL(i32);
IMPL_COPY_IMPL(i16);
IMPL_COPY_IMPL(i8);
IMPL_COPY_IMPL(u128);
IMPL_COPY_IMPL(u64);
IMPL_COPY_IMPL(u32);
IMPL_COPY_IMPL(u16);
IMPL_COPY_IMPL(u8);
IMPL_COPY_IMPL(bool);

bool copy_impl(void *dst, void *src) {
	bool *(*do_copy)(Object *dst, Object *src) =
	    find_fn((Object *)src, "copy");
	if (do_copy == NULL)
		panic("copy not implemented for this type");
	((Object *)dst)->vdata.vtable = ((Object *)src)->vdata.vtable;
	return do_copy(dst, src);
}

size_t size_impl(void *obj) {
	size_t (*do_size)(Object *obj) = find_fn((Object *)obj, "size");
	if (do_size == NULL)
		panic("size not implemented for this type");
	return do_size(obj);
}

void no_cleanup(void *ptr) {}

void cleanup(void *ptr) {
	bool (*do_cleanup)(Object *ptr) = find_fn((Object *)ptr, "cleanup");

	if (do_cleanup != NULL)
		do_cleanup(ptr);
}
