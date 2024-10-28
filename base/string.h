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

#ifndef _BASE_STRING__
#define _BASE_STRING__

#include <base/macro_util.h>
#include <base/macros.h>
#include <base/types.h>

int64 cstring_len(const byte *s);

Type(string);
#define string DefineType(string)

string string_create();
string string_create_cs(const char *s);
string string_create_b(const byte *s, unsigned int len);
string string_clone(string *s);
int string_append_bytes(string *s, const byte *bytes, unsigned int len);
int string_append_cstring(string *s, const char *bytes);
int string_append_string(string *s1, const string *s2);
unsigned int string_len(const string *s);
unsigned int string_index_of(const string *s, const string *s2);
unsigned int string_last_index_of(const string *s, const string *s2);
unsigned int string_substring(string *dst, const string *src, unsigned int start);
unsigned int string_substring_s(string *dst, const string *src, unsigned int start,
								unsigned int end);
byte string_char_at(const string *s, unsigned int index);
int string_compare(const string *s1, const string *s2);
bool string_equal(const string *s1, const string *s2);

/*
string2 string2_create(const char *s);
unsigned int string2_len(const string2 s);

typedef struct stringNc {
	void *impl;
} stringNc;

void string_cleanup(stringNc *ptr);
int64 string_create(string *s);
int64 string_create_cs(string *s, const char *s2);
int64 string_create_ch(string *s, const byte *s2, int64 len);
int64 string_create_s(string *s, const string *s2);
int64 string_append_ch(string *s, const byte *s2, int64 len);
int64 string_append_s(string *s, const string *s2);
int64 string_len(const string *s);
int64 string_index_of(const string *s1, const string *s2);
int64 string_last_index_of(const string *s1, const string *s2);
int64 string_substring(string *dst, const string *src, int64 begin);
int64 string_substring_s(string *dst, const string *src, int64 begin, int64 end);
byte string_char_at(const string *s, int64 index);
int64 string_equal(const string *s1, const string *s2);
void string_move(string *s1, string *s2);
int64 cstring_len(const byte *S);

byte *cstring(const string *s);
*/

#endif // _BASE_STRING__
