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

#include <base/fam_err.h>
#include <base/os.h>
#include <base/osdef.h>
#include <base/string.h>

typedef struct CStringImpl {
	u64 len;
	u8 data[];
} CStringImpl;

u64 cstring_len(const u8 *S) {
	u64 ret = 0;
	while (*S != '\0') {
		S++;
		ret++;
	}
	return ret;
}

void string_cleanup(CStringNc *ptr) {
	if (ptr == NULL || ptr->impl == NULL)
		return;
	release(ptr->impl);
	ptr->impl = NULL;
}

i32 string_create(CString *s) {
	return string_create_cs(s, "");
}

i32 string_create_cs(CString *s, const char *s2) {
	if (s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	return string_create_u8(s, s2, cstring_len(s2));
}

i32 string_create_u8(CString *s, const u8 *s2, u64 len) {
	if (s == NULL || s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	s->impl = alloc(1 + len + sizeof(CStringImpl), false);
	if (s->impl == NULL)
		return -1;
	CStringImpl *si = s->impl;
	si->len = len;
	memcpy(si->data, s2, len);
	si->data[len] = 0;

	return 0;
}

i32 string_create_s(CString *s, const CString *s2) {
	return string_create_cs(s, cstring(s2));
}

i32 string_append_s(CString *s, const CString *s2) {
	CStringImpl *si2 = s2->impl;
	return string_append_u8(s, si2->data, si2->len);
}

i32 string_append_u8(CString *s, const u8 *s2, u64 len) {
	if (s == NULL || s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	CStringImpl *si = s->impl;
	if (si == NULL) {
		SetErr(IllegalState);
		return -1;
	}
	u64 len_sum = si->len + len;
	void *tmp = resize(si, 1 + len_sum + sizeof(CStringImpl));

	if (tmp == NULL) {
		return -1;
	}

	s->impl = si = tmp;

	memcpy(si->data + si->len, s2, len);
	si->data[len_sum] = 0;

	si->len = len_sum;

	return 0;
}

i32 string_append_cs(CString *s, const char *s2) {
	return string_append_u8(s, s2, cstring_len(s2));
}

u64 string_len(const CString *s) {
	if (s == NULL)
		return 0;
	CStringImpl *si = s->impl;
	if (si == NULL)
		return 0;
	return si->len;
}

i64 string_index_of(const CString *s1, const CString *s2) {
	if (s1 == NULL || s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	CStringImpl *si1 = s1->impl;
	CStringImpl *si2 = s2->impl;

	if (si1 == NULL || si2 == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	u64 s1len = si1->len;
	u64 s2len = si2->len;

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return 0;

	u64 max = 1 + (s1len - s2len);
	for (u64 i = 0; i < max; i++) {
		if (!memcmp(si1->data + i, si2->data, s2len))
			return i;
	}

	return -1;
}

i64 string_last_index_of(const CString *s1, const CString *s2) {
	if (s1 == NULL || s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	CStringImpl *si1 = s1->impl;
	CStringImpl *si2 = s2->impl;

	if (si1 == NULL || si2 == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	u64 s1len = si1->len;
	u64 s2len = si2->len;

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return s1len;

	u64 max = s1len - s2len;
	for (i64 i = max; i >= 0; i--) {
		if (!memcmp(si1->data + i, si2->data, s2len))
			return i;
	}

	return -1;
}

i32 string_substring(CString *dst, const CString *src, u64 begin) {
	i32 ret = 0;
	return ret;
}

i32 string_substring_s(CString *dst, const CString *src, u64 begin, u64 end) {
	i32 ret = 0;
	return ret;
}
u8 string_char_at(const CString *s, u64 index) {
	u8 ret = 0;
	return ret;
}

bool string_equal(const CString *s1, const CString *s2) {
	if (s1 == NULL && s2 == NULL)
		return true;
	if (s1 == NULL || s2 == NULL)
		return false;

	CStringImpl *si1 = s1->impl;
	CStringImpl *si2 = s2->impl;
	if (si1 == NULL && si2 == NULL)
		return true;
	if (si1 == NULL || si2 == NULL)
		return false;
	if (si1->len != si2->len)
		return false;

	for (u64 i = 0; i < si1->len; i++) {
		if (si1->data[i] != si2->data[i])
			return false;
	}

	return true;
}

u8 *cstring(const CString *s) {
	if (s == NULL || s->impl == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}
	CStringImpl *si = s->impl;
	return si->data;
}
