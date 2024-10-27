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

typedef struct stringImpl {
	num len;
	ch data[];
} stringImpl;

num cstring_len(const ch *S) {
	num ret = 0;
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

num string_create(string *s) {
	return string_create_cs(s, "");
}

num string_create_cs(string *s, const char *s2) {
	if (s2 == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	return string_create_ch(s, s2, cstring_len(s2));
}

num string_create_ch(string *s, const ch *s2, num len) {
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

num string_create_s(string *s, const string *s2) {
	return string_create_cs(s, cstring(s2));
}

num string_append_s(string *s, const string *s2) {
	stringImpl *si2 = s2->impl;
	return string_append_ch(s, si2->data, si2->len);
}

num string_append_ch(string *s, const ch *s2, num len) {
	if (s == NULL || s2 == NULL || len < 0) {
		SetErr(IllegalArgument);
		return -1;
	}

	stringImpl *si = s->impl;
	if (si == NULL) {
		SetErr(IllegalState);
		return -1;
	}
	num len_sum = si->len + len;
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

num string_len(const string *s) {
	if (s == NULL)
		return 0;
	stringImpl *si = s->impl;
	if (si == NULL)
		return 0;
	return si->len;
}

num string_index_of(const string *s1, const string *s2) {
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

	num s1len = si1->len;
	num s2len = si2->len;

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return 0;

	num max = 1 + (s1len - s2len);
	for (num i = 0; i < max; i++) {
		if (!memcmp(si1->data + i, si2->data, s2len))
			return i;
	}

	return -1;
}

num string_last_index_of(const string *s1, const string *s2) {
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

	num s1len = si1->len;
	num s2len = si2->len;

	if (s2len > s1len)
		return -1;

	if (s2len == 0)
		return s1len;

	num max = s1len - s2len;
	for (num i = max; i >= 0; i--) {
		if (!memcmp(si1->data + i, si2->data, s2len))
			return i;
	}

	return -1;
}

num string_substring(string *dst, const string *src, num begin) {
	return string_substring_s(dst, src, begin, string_len(src));
}

num string_substring_s(string *dst, const string *src, num begin, num end) {
	ch ret = '\0';
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
ch string_char_at(const string *s, num index) {
	ch ret = '\0';
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

num string_equal(const string *s1, const string *s2) {
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

	for (num i = 0; i < si1->len; i++) {
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

ch *cstring(const string *s) {
	if (s == NULL || s->impl == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}
	stringImpl *si = s->impl;
	return si->data;
}
