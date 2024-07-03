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

#include <base/class.h>
#include <base/result.h>
#include <base/traits.h>

Result char_at(void *s, u64 index);

#define TRAIT_STRINGREF_BUILD(T) TRAIT_REQUIRED(T, Result, build, char *s)

#define TRAIT_STRING_BUILD(T)                                                  \
	TRAIT_REQUIRED(T, T##Ptr, build_expect, const char *s)                 \
	TRAIT_REQUIRED(T, T##Ptr *, build_ptr_expect, const char *s)           \
	TRAIT_REQUIRED(T, Result, build_ptr_try, char *s)

#define TRAIT_STRING_CORE(T)                                                   \
	TRAIT_REQUIRED(T, Result, substring, T##Ptr *s, u64 start, u64 end)    \
	TRAIT_REQUIRED(T, Result, index_of, T##Ptr *s, T##Ptr *n)              \
	TRAIT_REQUIRED(T, Result, last_index_of, T##Ptr *s, T##Ptr *n)         \
	TRAIT_REQUIRED(T, Result, index_of_s, T##Ptr *s, char *n)              \
	TRAIT_REQUIRED(T, Result, last_index_of_s, T##Ptr *s, char *n)         \
	TRAIT_REQUIRED(T, Result, char_at, T##Ptr *s, u64 index)

// have to include this here because Formatter depends on CString/String
typedef struct Formatter Formatter;

CLASS(CString, FIELD(char *, ptr) FIELD(u64, len))
IMPL(CString, TRAIT_STRING_BUILD)
IMPL(CString, TRAIT_CLONE)
IMPL(CString, TRAIT_EQUAL)
IMPL(CString, TRAIT_DEBUG)
IMPL(CString, TRAIT_UNWRAP)
IMPL(CString, TRAIT_APPEND)
IMPL(CString, TRAIT_LEN)
IMPL(CString, TRAIT_TO_STR)
IMPL(CString, TRAIT_STRING_CORE)

#define CString DEFINE_CLASS(CString)
#define STRINGPTRP(s) CString_build_ptr_expect(s)
#define STRINGPTR(s) CString_build_ptr_try(s)

CLASS(String, FIELD(RcPtr *, ptr))
IMPL(String, TRAIT_STRINGREF_BUILD)
IMPL(String, TRAIT_COPY)
IMPL(String, TRAIT_UNWRAP)
IMPL(String, TRAIT_EQUAL)
IMPL(String, TRAIT_APPEND)
IMPL(String, TRAIT_DEBUG)
IMPL(String, TRAIT_DEEP_COPY)
IMPL(String, TRAIT_TO_STR)
IMPL(String, TRAIT_LEN)
IMPL(String, TRAIT_STRING_CORE)
#define String DEFINE_CLASS(String)

String String_buildp(char *s);
Result String_build(char *s);
#define STRING(x) String_build(x)
#define STRINGP(x) String_buildp(x)

#define INDEX_OF(s, n)                                                         \
	_Generic((*s),                                                         \
	    String: _Generic((n),                                           \
	    char *: String_index_of_s,                                      \
	    default: String_index_of),                                      \
	    CString: _Generic((n),                                             \
	    char *: CString_index_of_s,                                        \
	    default: CString_index_of))(s, n)

#define LAST_INDEX_OF(s, n)                                                    \
	_Generic((*s),                                                         \
	    String: _Generic((n),                                           \
	    char *: String_last_index_of_s,                                 \
	    default: String_last_index_of),                                 \
	    CString: _Generic((n),                                             \
	    char *: CString_last_index_of_s,                                   \
	    default: CString_last_index_of))(s, n)

#define SUBSTRING(s, start, end)                                               \
	_Generic((s),                                                          \
	    String: String_substring,                                    \
	    CString: CString_substring)(&s, start, end)

#endif // _BASE_STRING__
