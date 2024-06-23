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

GETTER(StringImpl, ptr)
SETTER(StringImpl, ptr)
GETTER(StringImpl, len)
SETTER(StringImpl, len)
GETTER(StringRef, ptr)
SETTER(StringRef, ptr)

void StringImpl_cleanup(StringImplPtr *s) {
	char *ptr = *StringImpl_get_ptr(s);
	if (ptr != NULL) {
		tlfree(ptr);
		StringImpl_set_ptr(s, NULL);
		StringImpl_set_len(s, 0);
	}
}
usize StringImpl_size(StringImpl *s) { return sizeof(StringImpl); }
bool StringImpl_clone(StringImpl *dst, StringImpl *src) {
	u64 len = *StringImpl_get_len(src);
	StringImpl_set_len(dst, len);
	void *tmp = tlmalloc(sizeof(char) * (1 + len));
	StringImpl_set_ptr(dst, tmp);

	char *dst_ptr = *StringImpl_get_ptr(dst);
	char *src_ptr = *StringImpl_get_ptr(src);
	if (dst_ptr == NULL)
		return false;
	else {
		strcpy(dst_ptr, src_ptr);
		return true;
	}
}
void *StringImpl_unwrap(StringImpl *obj) {
	char *ptr = *StringImpl_get_ptr(obj);
	return ptr;
}
bool StringImpl_equal(StringImpl *obj1, StringImpl *obj2) {
	char *obj1_ptr = *StringImpl_get_ptr(obj1);
	char *obj2_ptr = *StringImpl_get_ptr(obj2);
	if (obj1_ptr == NULL || obj2_ptr == NULL)
		return false;
	return !strcmp(obj1_ptr, obj2_ptr);
}

Result StringImpl_build_try(const char *s) {
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

	StringImpl ret = BUILD(StringImpl, ptr, len);
	return Ok(ret);
}

StringImpl StringImpl_build_expect(const char *s) {
	if (s == NULL)
		panic("char pointer was NULL");
	u64 len = strlen(s);
	char *ptr = tlmalloc(sizeof(char) * (1 + len));
	if (ptr == NULL) {
		panic("Allocation Error: Could not allocate memory");
	}
	strcpy(ptr, s);

	StringImplPtr ret = BUILD(StringImpl, ptr, len);
	return ret;
}

StringImplPtr *StringImpl_build_ptr_expect(const char *s) {
	StringImplPtr *ret = tlmalloc(sizeof(StringImplPtr));
	if (ret == NULL)
		panic("Allocation Error: Could not allocate memory");
	*ret = StringImpl_build_expect(s);
	return ret;
}

Result StringImpl_build_ptr_try(const char *s) {
	StringImplPtr *ret = tlmalloc(sizeof(StringImplPtr));
	if (ret == NULL) {
		Error e = ERROR(ALLOC_ERROR, "Could not allocate memory");
		return Err(e);
	}
	return StringImpl_build_try(s);
}

Result StringImpl_append(StringImpl *dst, StringImpl *src) {
	char *dst_ptr = *StringImpl_get_ptr(dst);
	char *src_ptr = *StringImpl_get_ptr(src);
	u64 nlen = strlen(dst_ptr) + strlen(src_ptr);
	char *tmp = tlrealloc(dst_ptr, (nlen + 1) * sizeof(char));

	if (!tmp) {
		Error err = ERROR(ALLOC_ERROR, "could not allocate "
					       "sufficient memory");
		return Err(err);
	}

	strcat(tmp, src_ptr);
	StringImpl_set_ptr(dst, tmp);
	StringImpl_set_len(dst, nlen);

	return Ok(UNIT);
}

Result append(void *dst, void *src) {
	ResultPtr (*do_append)(Object *dst, Object *src) =
	    find_fn((Object *)src, "append");
	if (do_append == NULL)
		panic("append not implemented for this type");
	return do_append(dst, src);
}

StringRef StringRef_buildp(char *s) {
	StringImplPtr *ptr = STRINGIMPLP(s);
	RcPtr *nrc = tlmalloc(sizeof(Rc));
	if (nrc == NULL)
		panic("could not allocate memory");
	*nrc = RC(ptr);
	StringRefPtr ret = BUILD(StringRef, nrc);
	return ret;
}

Result StringRef_build(char *s) {
	StringImplPtr *ptr = STRINGIMPLP(s);
	RcPtr *nrc = tlmalloc(sizeof(Rc));
	if (nrc == NULL) {
		Error err =
		    ERROR(ALLOC_ERROR, "Could not allocate sufficient memory");
		return Err(err);
	}
	*nrc = RC(ptr);
	StringRef ret = BUILD(StringRef, nrc);
	return Ok(ret);
}

bool StringRef_copy(StringRef *dst, StringRef *src) {
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
usize StringRef_size(StringRef *obj) { return sizeof(StringRef); }

void *StringRef_unwrap(StringRef *obj) {
	RcPtr *ptr = *StringRef_get_ptr(obj);
	StringImplPtr *s = unwrap(ptr);
	return unwrap(s);
}

void StringRef_cleanup(StringRef *ptr) {
	RcPtr *rc = *StringRef_get_ptr(ptr);
	if (rc != NULL) {
		cleanup(rc);
		tlfree(rc);
		StringRef_set_ptr(ptr, NULL);
	}
}
