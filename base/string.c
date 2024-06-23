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

#include <base/ekinds.h>
#include <base/string.h>
#include <base/unit.h>

GETTER(String, ptr)
SETTER(String, ptr)
GETTER(String, len)
SETTER(String, len)
GETTER(RcString, ptr)
SETTER(RcString, ptr)

void String_cleanup(StringPtr *s) {
	char *ptr = *String_get_ptr(s);
	if (ptr != NULL) {
		tlfree(ptr);
		String_set_ptr(s, NULL);
		String_set_len(s, 0);
	}
}
usize String_size(String *s) { return sizeof(String); }
bool String_clone(String *dst, String *src) {
	u64 len = *String_get_len(src);
	String_set_len(dst, len);
	void *tmp = tlmalloc(sizeof(char) * (1 + len));
	String_set_ptr(dst, tmp);

	char *dst_ptr = *String_get_ptr(dst);
	char *src_ptr = *String_get_ptr(src);
	if (dst_ptr == NULL)
		return false;
	else {
		strcpy(dst_ptr, src_ptr);
		return true;
	}
}
void *String_unwrap(StringPtr *obj) {
	char *ptr = *String_get_ptr(obj);
	return ptr;
}
bool String_equal(String *obj1, String *obj2) {
	char *obj1_ptr = *String_get_ptr(obj1);
	char *obj2_ptr = *String_get_ptr(obj2);
	if (obj1_ptr == NULL || obj2_ptr == NULL)
		return false;
	return !strcmp(obj1_ptr, obj2_ptr);
}

Result String_build_try(const char *s) {
	if (s == NULL) {
		Error e = ERROR(ILLEGAL_ARGUMENT, "char pointer was NULL");
		return Err(e);
	}
	u64 len = strlen(s);
	char *ptr = tlmalloc(sizeof(char) * (1 + len));
	if (ptr == NULL) {
		Error e = ERROR(ALLOC_ERROR, "Could not allocate memory");
		return Err(e);
	}
	strcpy(ptr, s);

	String ret = BUILD(String, ptr, len);
	return Ok(ret);
}

String String_build_expect(const char *s) {
	if (s == NULL)
		panic("char pointer was NULL");
	u64 len = strlen(s);
	char *ptr = tlmalloc(sizeof(char) * (1 + len));
	if (ptr == NULL) {
		panic("Allocation Error: Could not allocate memory");
	}
	strcpy(ptr, s);

	StringPtr ret = BUILD(String, ptr, len);
	return ret;
}

StringPtr *String_build_ptr_expect(const char *s) {
	StringPtr *ret = tlmalloc(sizeof(StringPtr));
	if (ret == NULL)
		panic("Allocation Error: Could not allocate memory");
	*ret = String_build_expect(s);
	return ret;
}

Result String_build_ptr_try(const char *s) {
	StringPtr *ret = tlmalloc(sizeof(StringPtr));
	if (ret == NULL) {
		Error e = ERROR(ALLOC_ERROR, "Could not allocate memory");
		return Err(e);
	}
	return String_build_try(s);
}

Result String_append(String *dst, String *src) {
	char *dst_ptr = *String_get_ptr(dst);
	char *src_ptr = *String_get_ptr(src);
	u64 nlen = strlen(dst_ptr) + strlen(src_ptr);
	char *tmp = tlrealloc(dst_ptr, (nlen + 1) * sizeof(char));

	if (!tmp) {
		Error err = ERROR(ALLOC_ERROR, "could not allocate "
					       "sufficient memory");
		return Err(err);
	}

	strcat(tmp, src_ptr);
	String_set_ptr(dst, tmp);
	String_set_len(dst, nlen);

	return Ok(UNIT);
}

Result append(void *dst, void *src) {
	ResultPtr (*do_append)(Object *dst, Object *src) =
	    find_fn((Object *)src, "append");
	if (do_append == NULL)
		panic("append not implemented for this type");
	return do_append(dst, src);
}

RcString RcString_build(char *s) {
	StringPtr *ptr = STRINGP(s);
	RcPtr *nrc = tlmalloc(sizeof(Rc));
	*nrc = RC(ptr);
	RcStringPtr ret = BUILD(RcString, nrc);
	return ret;
}

bool RcString_copy(RcString *dst, RcString *src) {
	RcPtr *src_rc = src->_ptr;
	RcPtr *nrc = tlmalloc(sizeof(Rc));
	if (nrc == NULL)
		return false;
	if (!clone(nrc, src_rc)) {
		tlfree(nrc);
		return false;
	}
	dst->_ptr = nrc;

	return true;
}
usize RcString_size(RcString *obj) { return sizeof(RcString); }

void *RcString_unwrap(RcString *obj) {
	RcPtr *ptr = *RcString_get_ptr(obj);
	StringPtr *s = unwrap(ptr);
	return unwrap(s);
}

void RcString_cleanup(RcString *ptr) {
	RcPtr *rc = *RcString_get_ptr(ptr);
	if (rc != NULL) {
		cleanup(rc);
		tlfree(rc);
		RcString_set_ptr(ptr, NULL);
	}
}
