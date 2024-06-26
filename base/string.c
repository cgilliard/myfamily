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

#include <base/class.h>
#include <base/ekinds.h>
#include <base/string.h>
#include <base/unit.h>

GETTER(String, ptr)
SETTER(String, ptr)
GETTER(String, len)
SETTER(String, len)
GETTER(StringRef, ptr)
SETTER(StringRef, ptr)

Result char_at(void *s, u64 index) {
	ResultPtr (*do_char_at)(Object *s, u64 index) =
	    find_fn((Object *)s, "char_at");
	if (do_char_at == NULL)
		panic("char_at not implemented for this type");
	return do_char_at(s, index);
}

char *rstrstr(char *s1, char *s2) {
	size_t s1len = strlen(s1);
	size_t s2len = strlen(s2);
	char *s;

	if (s2len > s1len)
		return NULL;
	for (s = s1 + s1len - s2len; s >= s1; --s)
		if (strncmp(s, s2, s2len) == 0)
			return s;
	return NULL;
}

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

u64 String_len(String *obj) {
	u64 len = *String_get_len(obj);
	return len;
}

void *String_unwrap(String *obj) {
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

Result String_build_ptr_try(char *s) {
	StringPtr rs = BUILD(String, s, strlen(s));
	StringPtr *rsc = tlmalloc(sizeof(String));
	clone(rsc, &rs);
	Rc ret = RC(rsc);
	Rc_set_flags(&ret, RC_FLAGS_NO_UNWRAP);
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

Result String_index_of(StringPtr *s, StringPtr *n) {
	char *sptr = *String_get_ptr(s);
	char *nptr = *String_get_ptr(n);
	char *res = strstr(sptr, nptr);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_last_index_of(StringPtr *s, StringPtr *n) {
	char *sptr = *String_get_ptr(s);
	char *nptr = *String_get_ptr(n);
	char *res = rstrstr(sptr, nptr);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_substring(StringPtr *s, u64 start, u64 end) {
	if (start > end) {
		Error err = ERROR(
		    ILLEGAL_ARGUMENT,
		    "start (%llu) must be equal to or less than end (%llu)",
		    start, end);
		return Err(err);
	}
	u64 len = *String_get_len(s);
	if (end > len) {
		Error err = ERROR(ILLEGAL_ARGUMENT,
				  "end (%llu) must be equal to or less than "
				  "the length of s (%llu)",
				  end, len);
		return Err(err);
	}
	char *sptr = *String_get_ptr(s);
	char nstr[(end - start) + 1];
	strncpy(nstr, sptr + start, (end - start));
	nstr[end - start] = 0;

	return STRINGPTR(nstr);
}

char *String_to_str(String *s) {
	char *sptr = *String_get_ptr(s);
	return sptr;
}

Result String_index_of_s(String *s, char *n) {
	char *sptr = *String_get_ptr(s);
	char *res = strstr(sptr, n);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_last_index_of_s(String *s, char *n) {
	char *sptr = *String_get_ptr(s);
	char *res = rstrstr(sptr, n);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_char_at(String *s, u64 index) {
	char *sptr = *String_get_ptr(s);
	u64 len = *(u64 *)String_get_len(s);
	if (index >= len) {
		Error err =
		    ERROR(STRING_INDEX_OUT_OF_BOUNDS,
			  "index (%llu) is greater than or equal to len (%llu)",
			  index, len);
		return Err(err);
	}
	i8 ret = sptr[index];
	return Ok(ret);
}

StringRef StringRef_buildp(char *s) {
	StringPtr *ptr = STRINGPTRP(s);
	RcPtr *nrc = tlmalloc(sizeof(Rc));
	if (nrc == NULL)
		panic("could not allocate memory");
	*nrc = RC(ptr);
	StringRefPtr ret = BUILD(StringRef, nrc);
	return ret;
}

u64 StringRef_len(StringRef *obj) {
	RcPtr *rc = *StringRef_get_ptr(obj);
	StringPtr *ptr = unwrap(rc);
	u64 len = *String_get_len(ptr);
	return len;
}

Result StringRef_build(char *s) {
	if (s == NULL) {
		Error err = ERROR(ILLEGAL_ARGUMENT, "s cannot be NULL");
		return Err(err);
	}
	StringPtr *sptr = tlmalloc(sizeof(String));
	if (sptr == NULL) {
		Error err =
		    ERROR(ALLOC_ERROR, "could not allocate sufficient memory");
		return Err(err);
	}
	StringPtr scpy = BUILD(String, s, strlen(s));
	if (!clone(sptr, &scpy)) {
		Error err = ERROR(COPY_ERROR, "could not clone the string");
		return Err(err);
	}
	RcPtr *nrc = tlmalloc(sizeof(Rc));
	if (nrc == NULL) {
		Error err =
		    ERROR(ALLOC_ERROR, "could not allocate sufficient memory");
		return Err(err);
	}
	*nrc = RC(sptr);
	StringRef ret = BUILD(StringRef, nrc);
	return Ok(ret);
}

bool StringRef_equal(StringRef *dst, StringRef *src) {
	RcPtr *rc_src = *StringRef_get_ptr(src);
	RcPtr *rc_dst = *StringRef_get_ptr(dst);

	return equal(unwrap(rc_src), unwrap(rc_dst));
}

bool StringRef_clone(StringRef *dst, StringRef *src) {
	RcPtr *src_rc = src->_ptr;

	dst->_ptr = tlmalloc(sizeof(Rc));
	if (dst->_ptr == NULL)
		return false;
	if (!clone(dst->_ptr, src_rc)) {
		tlfree(dst->_ptr);
		return false;
	}

	return true;
}

Result StringRef_deep_copy(StringRef *dst, StringRef *src) {
	RcPtr *src_rc = *StringRef_get_ptr(src);
	StringPtr *src_str = unwrap(src_rc);
	dst->_ptr = tlmalloc(sizeof(Rc));
	StringPtr *dst_str = tlmalloc(sizeof(String));
	if (dst_str == NULL) {
		Error err =
		    ERROR(ALLOC_ERROR, "Could not allocate sufficient memory");
		return Err(err);
	}
	clone(dst_str, src_str);
	Rc x = RC(dst_str);
	copy(dst->_ptr, &x);
	return Ok(UNIT);
}

usize StringRef_size(StringRef *obj) { return sizeof(StringRef); }

void *StringRef_unwrap(StringRef *obj) {
	RcPtr *ptr = *StringRef_get_ptr(obj);
	StringPtr *s = unwrap(ptr);
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

Result StringRef_append(StringRef *dst, StringRef *src) {
	RcPtr *dst_rc = *StringRef_get_ptr(dst);
	RcPtr *src_rc = *StringRef_get_ptr(src);
	StringPtr *dst_ptr = unwrap(dst_rc);
	StringPtr *src_ptr = unwrap(src_rc);
	return append(dst_ptr, src_ptr);
}

Result StringRef_index_of(StringRef *s, StringRef *n) {
	RcPtr *sptr = *StringRef_get_ptr(s);
	RcPtr *nptr = *StringRef_get_ptr(n);
	StringPtr *sstrptr = unwrap(sptr);
	StringPtr *nstrptr = unwrap(nptr);
	return String_index_of(sstrptr, nstrptr);
}

Result StringRef_last_index_of(StringRef *s, StringRef *n) {

	RcPtr *sptr = *StringRef_get_ptr(s);
	RcPtr *nptr = *StringRef_get_ptr(n);
	StringPtr *sstrptr = unwrap(sptr);
	StringPtr *nstrptr = unwrap(nptr);
	return String_last_index_of(sstrptr, nstrptr);
}

Result StringRef_substring(StringRef *s, u64 start, u64 end) {

	RcPtr *sptr = *StringRef_get_ptr(s);
	StringPtr *sstrptr = unwrap(sptr);
	return String_substring(sstrptr, start, end);
}

char *StringRef_to_str(StringRef *s) {
	RcPtr *sptr = *StringRef_get_ptr(s);
	StringPtr *sstrptr = unwrap(sptr);
	return to_str(sstrptr);
}

Result StringRef_index_of_s(StringRef *s, char *n) {
	RcPtr *sptr = *StringRef_get_ptr(s);
	StringPtr *sstrptr = unwrap(sptr);
	return String_index_of_s(sstrptr, n);
}

Result StringRef_last_index_of_s(StringRef *s, char *n) {
	RcPtr *sptr = *StringRef_get_ptr(s);
	StringPtr *sstrptr = unwrap(sptr);
	return String_last_index_of_s(sstrptr, n);
}

Result StringRef_char_at(StringRef *s, u64 index) {
	RcPtr *sptr = *StringRef_get_ptr(s);
	StringPtr *sstrptr = unwrap(sptr);
	return String_char_at(sstrptr, index);
}
