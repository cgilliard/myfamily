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

#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/os.h>
#include <base/osdef.h>
#include <base/slabs.h>
#include <base/string.h>

#include <stdio.h>

int64 cstring_len(const byte *S) {
	if (S == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	int64 ret = 0;
	while (*S != '\0') {
		S++;
		ret++;
	}
	return ret;
}

byte *cstring(const string s) {
	return $(s);
}

string string_create() {
	return string_create_b("", 0);
}
string string_create_cs(const char *s) {
	if (s == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}
	return string_create_b(s, cstring_len(s));
}
string string_create_b(const byte *s, unsigned int len) {
	if (s == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}
	if (len > UINT32_MAX) {
		SetErr(Overflow);
		return NULL;
	}
	Ptr ret = fam_alloc(len + 1, false);
	if (ret == NULL) {
		return NULL;
	}
	int64 *aux = ptr_aux(ret);
	int64 flags = *aux & 0xFF00000000000000LL;
	*aux = flags | len;
	memcpy($(ret), s, len);
	((byte *)$(ret))[len] = '\0';
	return ret;
}
string string_clone(string s) {
	if (nil(s)) {
		SetErr(IllegalArgument);
		return NULL;
	}
	return string_create_b(cstring(s), string_len(s));
}
string string_append_bytes(const string s, const byte *bytes, unsigned int len) {
	if (nil(s) || bytes == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}
	unsigned int slen = string_len(s);
	int64 nlen = (int64)len + (int64)slen;
	if (nlen > UINT32_MAX) {
		SetErr(Overflow);
		return NULL;
	}
	Ptr ret = fam_alloc(nlen + 1, false);
	if (ret == NULL)
		return ret;

	int64 *aux = ptr_aux(ret);
	int64 flags = *aux & 0xFF00000000000000LL;
	*aux = flags | nlen;
	memcpy($(ret), $(s), slen);
	memcpy($(ret) + slen, bytes, len);
	((byte *)$(ret))[nlen] = '\0';

	return ret;
}
string string_append_cstring(const string s, const char *bytes) {
	return string_append_bytes(s, bytes, cstring_len(bytes));
}
string string_append_string(const string s, const string s2) {
	return string_append_bytes(s, cstring(s2), string_len(s2));
}
int64 string_len(const string s) {
	if (nil(s)) {
		SetErr(IllegalArgument);
		return -1;
	}
	int64 *aux = ptr_aux(s);
	return (*aux & 0x00FFFFFFFFFFFFFFLL);
}
int64 string_index_of(const string s1, const string s2) {
	// TODO: look into optimization with Boyer-Moore or KMP
	if (nil(s1) || nil(s2)) {
		SetErr(IllegalArgument);
		return -1;
	}
	const char *si1 = $(s1);
	const char *si2 = $(s2);

	unsigned int s1len = string_len(s1);
	unsigned int s2len = string_len(s2);

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return 0;

	unsigned int max = 1 + (s1len - s2len);
	for (unsigned int i = 0; i < max; i++) {
		if (!memcmp(si1 + i, si2, s2len))
			return i;
	}

	return -1;
}
int64 string_last_index_of(const string s1, const string s2) {
	// TODO: look into optimization with Boyer-Moore or KMP
	if (nil(s1) || nil(s2)) {
		SetErr(IllegalArgument);
		return -1;
	}
	const char *si1 = $(s1);
	const char *si2 = $(s2);

	unsigned int s1len = string_len(s1);
	unsigned int s2len = string_len(s2);

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return 0;

	int64 max = s1len - s2len;
	for (unsigned int i = max; i >= 0; i--) {
		if (!memcmp(si1 + i, si2, s2len))
			return i;
	}
	return -1;
}
string string_substring(const string src, unsigned int start) {
	return string_substring_s(src, start, string_len(src));
}
string string_substring_s(const string src, unsigned int start, unsigned int end) {
	if (nil(src) || end < start) {
		SetErr(IllegalArgument);
		return NULL;
	}

	if (end > string_len(src)) {
		SetErr(IndexOutOfBounds);
		return NULL;
	}

	return string_create_b($(src) + start, end - start);
}
byte string_char_at(const string s, unsigned int index) {
	if (nil(s)) {
		SetErr(IllegalArgument);
		return '\0';
	}

	if (index >= string_len(s)) {
		SetErr(IndexOutOfBounds);
		return '\0';
	}

	return ((char *)$(s))[index];
}
bool string_equal(const string s1, const string s2) {
	if (nil(s1) && nil(s2))
		return true;
	if (nil(s1) || nil(s2))
		return false;

	int64 len = string_len(s1);
	if (len != string_len(s2))
		return false;

	const char *si1 = $(s1);
	const char *si2 = $(s2);

	for (int64 i = 0; i < len; i++) {
		if (si1[i] != si2[i])
			return false;
	}

	return true;
}

int string_compare(const string s1, const string s2) {
	if (nil(s1) && nil(s2))
		return 0;
	if (nil(s1))
		return -1;
	if (nil(s2))
		return 1;
	const char *s1c = $(s1);
	const char *s2c = $(s2);
	unsigned int s1len = string_len(s1);
	unsigned int s2len = string_len(s2);
	unsigned int len;
	if (s1len < s2len)
		len = s1len;
	else
		len = s2len;
	unsigned int i = 0;

	while (i < len) {
		if (s1c[i] != s2c[i]) {
			if (s1c[i] < s2c[i])
				return -1;
			else
				return 1;
		}
		i++;
	}
	if (s2len > s1len)
		return 1;
	else if (s2len < s1len)
		return -1;
	return 0;
}

// Functions that require override of const
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
void string_cleanup(const string *ptr) {
	if (!nil(*ptr)) {
		fam_release(ptr);
	}
}

/*
void string2_cleanup(string2 *ptr) {
	if (ptr == NULL)
		return;
	fam_release(ptr);
	*ptr = NULL;
}
string2 string2_create(const char *s) {
	int64 len = cstring_len(s);
	Ptr ret = fam_alloc(len + 1, false);
	if (ret == NULL) {
		return NULL;
	}
	unsigned int *p = ptr_aux2(ret);
	*p = len;
	memcpy($(ret), s, len);
	((byte *)$(ret))[len] = 0;
	return ret;
}

unsigned int string2_len(const string2 s) {
	if (s == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	return *((unsigned int *)ptr_aux2(s));
}

typedef struct stringImpl {
	int64 len;
	byte data[];
} stringImpl;

int64 cstring_len(const byte *S) {
	int64 ret = 0;
	while (*S != '\0') {
		S++;
		ret++;
	}
	return ret;
}

void string_cleanup(stringNc *ptr) {
	if (ptr == NULL || ptr->impl == NULL)
		return;
	release(ptr->impl);
	ptr->impl = NULL;
}

int64 string_create(string *s) {
	return string_create_cs(s, "");
}

int64 string_create_cs(string *s, const char *s2) {
	if (s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	return string_create_ch(s, s2, cstring_len(s2));
}

int64 string_create_ch(string *s, const byte *s2, int64 len) {
	if (s == NULL || s2 == NULL || len < 0) {
		SetErr(IllegalArgument);
		return -1;
	}

	s->impl = alloc(1 + len + sizeof(stringImpl), false);
	if (s->impl == NULL)
		return -1;
	stringImpl *si = s->impl;
	si->len = len;
	memcpy(si->data, s2, len);
	si->data[len] = 0;

	return 0;
}

int64 string_create_s(string *s, const string *s2) {
	return string_create_cs(s, cstring(s2));
}

int64 string_append_s(string *s, const string *s2) {
	stringImpl *si2 = s2->impl;
	return string_append_ch(s, si2->data, si2->len);
}

int64 string_append_ch(string *s, const byte *s2, int64 len) {
	if (s == NULL || s2 == NULL || len < 0) {
		SetErr(IllegalArgument);
		return -1;
	}

	stringImpl *si = s->impl;
	if (si == NULL) {
		SetErr(IllegalState);
		return -1;
	}
	int64 len_sum = si->len + len;
	void *tmp = resize(si, 1 + len_sum + sizeof(stringImpl));

	if (tmp == NULL) {
		return -1;
	}

	s->impl = si = tmp;

	memcpy(si->data + si->len, s2, len);
	si->data[len_sum] = 0;

	si->len = len_sum;

	return 0;
}

int64 string_len(const string *s) {
	if (s == NULL)
		return 0;
	stringImpl *si = s->impl;
	if (si == NULL)
		return 0;
	return si->len;
}

int64 string_index_of(const string *s1, const string *s2) {
	if (s1 == NULL || s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	stringImpl *si1 = s1->impl;
	stringImpl *si2 = s2->impl;

	if (si1 == NULL || si2 == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	int64 s1len = si1->len;
	int64 s2len = si2->len;

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return 0;

	int64 max = 1 + (s1len - s2len);
	for (int64 i = 0; i < max; i++) {
		if (!memcmp(si1->data + i, si2->data, s2len))
			return i;
	}

	return -1;
}

int64 string_last_index_of(const string *s1, const string *s2) {
	if (s1 == NULL || s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	stringImpl *si1 = s1->impl;
	stringImpl *si2 = s2->impl;

	if (si1 == NULL || si2 == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	int64 s1len = si1->len;
	int64 s2len = si2->len;

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return s1len;

	int64 max = s1len - s2len;
	for (int64 i = max; i >= 0; i--) {
		if (!memcmp(si1->data + i, si2->data, s2len))
			return i;
	}

	return -1;
}

int64 string_substring(string *dst, const string *src, int64 begin) {
	return string_substring_s(dst, src, begin, string_len(src));
}

int64 string_substring_s(string *dst, const string *src, int64 begin, int64 end) {
	byte ret = '\0';
	if (src == NULL || end < begin) {
		SetErr(IllegalArgument);
		return ret;
	}

	stringImpl *si = src->impl;
	if (si == NULL) {
		SetErr(IllegalState);
		return ret;
	}

	if (end > si->len || begin < 0) {
		SetErr(IndexOutOfBounds);
		return ret;
	}

	return string_create_ch(dst, si->data + begin, end - begin);
}
byte string_char_at(const string *s, int64 index) {
	byte ret = '\0';
	if (s == NULL) {
		SetErr(IllegalArgument);
		return ret;
	}

	stringImpl *si = s->impl;
	if (si == NULL) {
		SetErr(IllegalState);
		return ret;
	}

	if (index >= si->len || index < 0) {
		SetErr(IndexOutOfBounds);
		return ret;
	}

	ret = si->data[index];
	return ret;
}

int64 string_equal(const string *s1, const string *s2) {
	if (s1 == NULL && s2 == NULL)
		return true;
	if (s1 == NULL || s2 == NULL)
		return false;

	stringImpl *si1 = s1->impl;
	stringImpl *si2 = s2->impl;
	if (si1 == NULL && si2 == NULL)
		return true;
	if (si1 == NULL || si2 == NULL)
		return false;
	if (si1->len != si2->len)
		return false;

	for (int64 i = 0; i < si1->len; i++) {
		if (si1->data[i] != si2->data[i])
			return false;
	}

	return true;
}

void string_move(string *dst, string *src) {
	string_cleanup(dst);
	dst->impl = src->impl;
	src->impl = NULL;
}

byte *cstring(const string *s) {
	if (s == NULL || s->impl == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}
	stringImpl *si = s->impl;
	return si->data;
}
*/
