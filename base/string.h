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
#include <base/types.h>

typedef struct CStringNc {
	void *impl;
} CStringNc;

void string_cleanup(CStringNc *ptr);

#define INIT_STRING {.impl = NULL}
#define EMPTY_STRING                                                                               \
	({                                                                                             \
		CStringNc _empty__ = INIT_STRING;                                                          \
		_empty__;                                                                                  \
	})
#define CString CStringNc __attribute__((warn_unused_result, cleanup(string_cleanup)))
#define String_(v)                                                                                 \
	({                                                                                             \
		CStringNc _ret__ = INIT_STRING;                                                            \
		_Generic((v),                                                                              \
			char *: ({                                                                             \
					 char *val = _Generic((v), char *: v, default: NULL);                          \
					 string_create_cs(&_ret__, (char *)val);                                       \
				 }),                                                                               \
			CString: ({                                                                            \
					 CStringNc val = _Generic((v), CString: v, default: EMPTY_STRING);             \
					 string_create_s(&_ret__, &val);                                               \
				 }),                                                                               \
			default: string_create(&_ret__));                                                      \
		_ret__;                                                                                    \
	})
#define String(...) String_(__VA_OPT__(__VA_ARGS__) __VA_OPT__(NONE)(0ULL))

#define nil_string(s) (s.impl == NULL)
#define nil(s) nil_string(s)

i32 string_create(CString *s);
i32 string_create_cs(CString *s, const char *s2);
i32 string_create_u8(CString *s, const u8 *s2, u64 len);
i32 string_create_s(CString *s, const CString *s2);
i32 string_append_cs(CString *s, const char *s2);
i32 string_append_u8(CString *s, const u8 *s2, u64 len);
i32 string_append_s(CString *s, const CString *s2);
u64 string_len(const CString *s);
i64 string_index_of(const CString *s1, const CString *s2);
i64 string_last_index_of(const CString *s1, const CString *s2);
i32 string_substring(CString *dst, const CString *src, u64 begin);
i32 string_substring_s(CString *dst, const CString *src, u64 begin, u64 end);
u8 string_char_at(const CString *s, u64 index);
bool string_equal(const CString *s1, const CString *s2);

u8 *cstring(const CString *s);

#endif // _BASE_STRING__
