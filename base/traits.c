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
#include <base/traits_base.h>

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

bool default_copy(void *dst, void *src) { return clone(dst, src); }

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

bool clone(void *dst, void *src) {
	bool *(*do_clone)(Object *dst, Object *src) =
	    find_fn((Object *)src, "clone");
	if (do_clone == NULL)
		panic("clone not implemented for this type");
	((Object *)dst)->vdata.vtable = ((Object *)src)->vdata.vtable;
	((Object *)dst)->vdata.name = ((Object *)src)->vdata.name;
	bool ret = do_clone(dst, src);
	return ret;
}

usize size(void *obj) {
	usize (*do_size)(Object *obj) = find_fn((Object *)obj, "size");
	if (do_size == NULL)
		panic("size not implemented for this type: %s",
		      ((Object *)obj)->vdata.name);
	return do_size(obj);
}

void cleanup(void *ptr) {
	void (*do_cleanup)(Object *ptr) = find_fn((Object *)ptr, "cleanup");
	if (do_cleanup != NULL)
		do_cleanup(ptr);
}

Result append(void *dst, void *src) {
	ResultPtr (*do_append)(Object *dst, Object *src) =
	    find_fn((Object *)src, "append");
	if (do_append == NULL)
		panic("append not implemented for this type");
	return do_append(dst, src);
}

Result deep_copy(void *dst, void *src) {
	ResultPtr (*do_deep_copy)(Object *dst, Object *src) =
	    find_fn((Object *)src, "deep_copy");
	if (do_deep_copy == NULL)
		panic("append not implemented for this type");
	((Object *)dst)->vdata.vtable = ((Object *)src)->vdata.vtable;
	((Object *)dst)->vdata.name = ((Object *)src)->vdata.name;
	return do_deep_copy(dst, src);
}

u64 len(void *obj) {
	u64 (*do_len)(Object *obj) = find_fn((Object *)obj, "len");
	if (do_len == NULL)
		panic("len not implemented for this type");
	return do_len(obj);
}

Result to_string(void *obj) { return to_string_buf(obj, TO_STRING_BUF_SIZE); }

Result to_string_buf(void *obj, usize buf_size) {
	ResultPtr (*do_fmt)(Object *obj, Formatter *formatter) =
	    find_fn((Object *)obj, "fmt");
	if (do_fmt == NULL) {

		panic("display trait not implemented for this type [%s]",
		      CLASS_NAME(obj));
	}

	char buf[buf_size];
	Formatter fmt = BUILD(Formatter, buf, buf_size, 0);

	Result r = do_fmt(obj, &fmt);
	Try(r);

	return Formatter_to_str_ref(&fmt);
}

Result to_debug(void *obj) { return to_debug_buf(obj, TO_STRING_BUF_SIZE); }

Result to_debug_buf(void *obj, usize buf_size) {
	ResultPtr (*do_dbg)(Object *obj, Formatter *formatter) =
	    find_fn((Object *)obj, "dbg");
	if (do_dbg == NULL) {
		void (*do_to_str_buf)(Object *obj, char *buf, usize max_len) =
		    find_fn((Object *)obj, "to_str_buf");

		if (do_to_str_buf == NULL)
			panic("debug and to_str_buf trait not implemented for "
			      "this "
			      "type [%s]",
			      CLASS_NAME(obj));
		char buf[buf_size];
		do_to_str_buf(obj, buf, buf_size);
		return STRING(buf);
	}

	char buf[buf_size];
	Formatter fmt = BUILD(Formatter, buf, buf_size, 0);
	Result r = do_dbg(obj, &fmt);
	Try(r);

	return Formatter_to_str_ref(&fmt);
}
