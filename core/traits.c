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

// #include <core/traits.h>
#include <core/traits_base.h>
// #include <core/unit.h>
#include <stdlib.h>

bool default_copy(void *dst, void *src) { return myclone(dst, src); }

void cleanup(void *ptr) {
	printf("cleanup %p cl = %s\n", ptr, CLASS_NAME(ptr));
	Object *objptr = ptr;
	void (*do_cleanup)(Object *ptr) = find_fn(objptr, "cleanup");
	printf("found fn\n");
	if (do_cleanup != NULL)
		do_cleanup(ptr);
}

bool myclone(void *dst, void *src) {
	bool *(*do_myclone)(Object *dst, Object *src) =
	    find_fn((Object *)src, "myclone");
	if (do_myclone == NULL)
		panic("myclone not implemented for this type");
	((Object *)dst)->vdata.vtable = ((Object *)src)->vdata.vtable;
	((Object *)dst)->vdata.name = ((Object *)src)->vdata.name;
	bool ret = do_myclone(dst, src);
	return ret;
}

void *unwrap(void *obj) {
	void *(*do_unwrap)(Object *obj) = find_fn((Object *)obj, "unwrap");
	if (do_unwrap == NULL)
		panic("unwrap not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_unwrap(obj);
}

bool copy(void *dst, void *src) {
	bool *(*do_copy)(Object *dst, Object *src) =
	    find_fn((Object *)src, "copy");
	if (do_copy == NULL)
		panic("copy not implemented for this type: %s",
		      ((Object *)src)->vdata.name);
	((Object *)dst)->vdata.vtable = ((Object *)src)->vdata.vtable;
	((Object *)dst)->vdata.name = ((Object *)src)->vdata.name;
	bool ret = do_copy(dst, src);
	return ret;
}

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

void print(void *obj) {
	char *(*do_print)(Object *obj) = find_fn((Object *)obj, "print");
	if (do_print == NULL)
		panic("print not implemented for this type");
	do_print(obj);
}

u64 size(void *obj) {
	u64 (*do_size)(Object *obj) = find_fn((Object *)obj, "size");
	if (do_size == NULL)
		panic("size not implemented for this type: %s",
		      ((Object *)obj)->vdata.name);
	return do_size(obj);
}

/*
Rc into_iter_impl(void *obj) {
	RcPtr (*do_into_iter)(Object *obj) =
	    find_fn((Object *)obj, "into_iter");
	if (do_into_iter == NULL)
		panic("into_iter not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_into_iter(obj);
}

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
		panic("unwrap not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_unwrap(obj);
}

void *unwrap_as(char *name, void *obj) {
	void *(*do_unwrap_as)(char *name, Object *obj) =
	    find_fn((Object *)obj, "unwrap_as");
	if (do_unwrap_as == NULL)
		panic("unwrap_as not implemented by type %s", CLASS_NAME(obj));
	return do_unwrap_as(name, obj);
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

bool copy(void *dst, void *src) {
	bool *(*do_copy)(Object *dst, Object *src) =
	    find_fn((Object *)src, "copy");
	if (do_copy == NULL)
		panic("copy not implemented for this type: %s",
		      ((Object *)src)->vdata.name);
	((Object *)dst)->vdata.vtable = ((Object *)src)->vdata.vtable;
	((Object *)dst)->vdata.name = ((Object *)src)->vdata.name;
	return do_copy(dst, src);
}

bool myclone(void *dst, void *src) {
	bool *(*do_myclone)(Object *dst, Object *src) =
	    find_fn((Object *)src, "myclone");
	if (do_myclone == NULL)
		panic("myclone not implemented for this type");
	((Object *)dst)->vdata.vtable = ((Object *)src)->vdata.vtable;
	((Object *)dst)->vdata.name = ((Object *)src)->vdata.name;
	bool ret = do_myclone(dst, src);
	return ret;
}

u64 size(void *obj) {
	u64 (*do_size)(Object *obj) = find_fn((Object *)obj, "size");
	if (do_size == NULL)
		panic("size not implemented for this type: %s",
		      ((Object *)obj)->vdata.name);
	return do_size(obj);
}

u64 len(void *obj) {
	u64 (*do_len)(Object *obj) = find_fn((Object *)obj, "len");
	if (do_len == NULL)
		panic("len not implemented for this type");
	return do_len(obj);
}

Result append_impl(void *dst, void *src) {
	ResultPtr (*do_append)(Object *dst, Object *src) =
	    find_fn((Object *)dst, "append");
	if (do_append == NULL)
		panic("append not implemented for this type");
	return do_append(dst, src);
}

Result append_s(void *dst, Object *src) {
	ResultPtr (*do_append_s)(void *dst, Object *src) =
	    find_fn((Object *)dst, "append_s");
	if (do_append_s == NULL)
		panic("append_s not implemented for this type");
	return do_append_s(dst, src);
}

int myqsort_compare(const void *a, const void *b) {
	i32 (*do_cmp)(const void *a, const void *b) =
	    find_fn((Object *)a, "cmp");
	if (do_cmp == NULL)
		panic("cmp not implemented for this type");
	return do_cmp(a, b);
}

void myqsort(void *arr, u64 len) {
	Object **objarr = arr;
	qsort(arr, len, size(&objarr[0]), myqsort_compare);
}

Result binsearch(void *arr, u64 len, const Object *value) {
	Option ret = None;
	if (len == 0)
		return Ok(ret);
	u64 left = 0;
	u64 right = len - 1;
	OrdOptions (*do_cmp)(const void *a, const void *b) =
	    find_fn(value, "cmp");
	while (left <= right) {
		u64 mid = left + (right - left) / 2;
		OrdOptions cmp = do_cmp(arr + size((void *)value) * mid, value);
		if (cmp == EqualTo) {
			ret = Some(mid);
			break;
		} else if (cmp == GreaterThan)
			right = mid - 1;
		else
			left = mid + 1;
	}

	return Ok(ret);
}
*/