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

string string_ref(const string src) {
	if (nil(src)) {
		SetErr(IllegalArgument);
		return NULL;
	}

	int64 *aux = ptr_aux(src);
	int64 refc = (*aux & 0x00FFFFFF00000000) >> 32;
	refc++;
	*aux = (*aux & 0xFF000000FFFFFFFF) | (refc << 32);
	stringNc ret = src;

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
	int64 refc = 0x1LL << 32; // set reference counter to 1
	*aux = flags | len | refc;
	refc = (*aux & 0x00FFFFFF00000000) >> 32;
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
	int64 refc = 0x1LL << 32; // set reference counter to 1
	int64 flags = *aux & 0xFFFFFFFF00000000LL;
	*aux = flags | nlen | refc;
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
	return (*aux & 0xFFFFFFFF);
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
		int64 *aux = ptr_aux(*ptr);
		int64 refc = (*aux & 0x00FFFFFF00000000) >> 32;
		refc--;
		*aux = (*aux & 0xFF000000FFFFFFFF) | (refc << 32);

		if (refc == 0) {
			fam_release(ptr);
		}
	}
}
