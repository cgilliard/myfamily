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

#include <core/ekinds.h>
#include <core/formatter.h>
#include <core/resources.h>
#include <core/string.h>
#include <core/unit.h>

GETTER(String, ptr)
SETTER(String, ptr)
GETTER(String, len)
SETTER(String, len)

void String_cleanup(String *s) {
	char *p = GET(String, s, ptr);
	if (p) {
		myfree(p);
		SET(String, s, ptr, NULL);
	}
}

bool String_myclone(String *dst, String *src) {
	char *src_ptr = GET(String, src, ptr);
	u64 slen = GET(String, src, len);

	if (src_ptr == NULL)
		return false;

	char *dst_ptr = mymalloc(sizeof(char) * (1 + slen));
	if (dst_ptr == NULL)
		return false;
	strcpy(dst_ptr, src_ptr);

	SET(String, dst, ptr, dst_ptr);
	SET(String, dst, len, slen);

	return true;
}

bool String_equal(String *s1, String *s2) {
	if (s1 == NULL || s2 == NULL)
		return false;

	char *s1_ptr = GET(String, s1, ptr);
	char *s2_ptr = GET(String, s2, ptr);

	if (s1_ptr == NULL && s2_ptr == NULL)
		return true;
	else if (s1_ptr == NULL || s2_ptr == NULL)
		return false;

	return !strcmp(s1_ptr, s2_ptr);
}

Result String_append_s(String *s1, Object *s2) {
	if (strcmp(CLASS_NAME(s2), "String")) {
		Error e = ERR(UNEXPECTED_CLASS_ERROR,
			      "Trying to append class '%s', expected String\n",
			      CLASS_NAME(s2));
		return Err(e);
	}

	StringPtr *s2ptr = (StringPtr *)s2;
	return String_append(s1, unwrap(s2));
}

Result String_append(String *s1, char *s2_ptr) {

	if (s1 == NULL || s2_ptr == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "input must not be NULL");
		return Err(e);
	}

	char *s1_ptr = GET(String, s1, ptr);
	if (s1_ptr == NULL) {
		Error e = ERR(ILLEGAL_STATE, "ptr must not be NULL");
		return Err(e);
	}

	u64 s1len = GET(String, s1, len);
	u64 s2len = strlen(s2_ptr);
	u64 s1_new_len = s1len + s2len;
	void *tmp = myrealloc(s1_ptr, sizeof(char) * (s1_new_len + 1));
	if (tmp == NULL) {
		return STATIC_ALLOC_RESULT;
	}

	strcat(tmp, s2_ptr);
	SET(String, s1, ptr, tmp);
	SET(String, s1, len, s1_new_len);

	return Ok(_());
}

Result String_build(char *s) {
	if (s == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "s must not be NULL");
		return Err(e);
	}
	u64 len = strlen(s);
	char *ptr = mymalloc(sizeof(char) * (len + 1));
	if (ptr == NULL) {
		return STATIC_ALLOC_RESULT;
	}
	strcpy(ptr, s);
	StringPtr ret = BUILD(String, ptr, len);
	return Ok(ret);
}

void *String_unwrap(String *s) { return GET(String, s, ptr); }

u64 String_len(String *s) { return GET(String, s, len); }

void String_print(String *s) { printf("%s\n", GET(String, s, ptr)); }

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

Result String_index_of(String *s, String *n, u64 start) {
	if (s == NULL || n == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "input must not be NULL");
		return Err(e);
	}
	char *sptr = GET(String, s, ptr);
	char *nptr = GET(String, n, ptr);
	if (sptr == NULL || nptr == NULL) {
		Error e = ERR(ILLEGAL_STATE, "ptr must not be NULL");
		return Err(e);
	}
	char *res = strstr(sptr + start, nptr);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_last_index_of(String *s, String *n) {
	if (s == NULL || n == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "input must not be NULL");
		return Err(e);
	}
	char *sptr = GET(String, s, ptr);
	char *nptr = GET(String, n, ptr);
	if (sptr == NULL || nptr == NULL) {
		Error e = ERR(ILLEGAL_STATE, "ptr must not be NULL");
		return Err(e);
	}
	char *res = rstrstr(sptr, nptr);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_substring(String *s, u64 start, u64 end) {
	if (start > end) {
		Error err =
		    ERR(ILLEGAL_ARGUMENT,
			"start (%llu) must be equal to or less than end (%llu)",
			start, end);
		return Err(err);
	}
	u64 slen = GET(String, s, len);
	if (end > slen) {
		Error err = ERR(ILLEGAL_ARGUMENT,
				"end (%llu) must be equal to or less than "
				"the length of s (%llu)",
				end, slen);
		return Err(err);
	}
	char *sptr = GET(String, s, ptr);
	char nstr[(end - start) + 1];
	strncpy(nstr, sptr + start, (end - start));
	nstr[end - start] = 0;

	return String_build(nstr);
}

Result String_index_of_s(String *s, char *n, u64 start) {
	if (s == NULL || n == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "input must not be NULL");
		return Err(e);
	}
	char *sptr = GET(String, s, ptr);
	if (sptr == NULL) {
		Error e = ERR(ILLEGAL_STATE, "ptr must not be NULL");
		return Err(e);
	}
	char *res = strstr(sptr + start, n);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_last_index_of_s(String *s, char *n) {
	if (s == NULL || n == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "input must not be NULL");
		return Err(e);
	}
	char *sptr = GET(String, s, ptr);
	if (sptr == NULL) {
		Error e = ERR(ILLEGAL_STATE, "ptr must not be NULL");
		return Err(e);
	}
	char *res = rstrstr(sptr, n);
	i64 ret;
	if (res == NULL)
		ret = -1;
	else
		ret = res - sptr;
	return Ok(ret);
}

Result String_char_at(String *s, u64 index) {
	char *sptr = GET(String, s, ptr);
	u64 slen = GET(String, s, len);
	if (index >= slen) {
		Error err =
		    ERR(STRING_INDEX_OUT_OF_BOUNDS,
			"index (%llu) is greater than or equal to len (%llu)",
			index, slen);
		return Err(err);
	}
	i8 ret = sptr[index];
	return Ok(ret);
}

Result String_fmt(String *s, Formatter *f) { return WRITE(f, unwrap(s)); }

Result String_from_slice(Slice *s, u64 len) {
	char buf[len + 1];
	void *ref = GET(Slice, s, ref);
	memcpy(buf, ref, len);
	buf[len] = 0;
	return String_build(buf);
}
