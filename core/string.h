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

#ifndef _CORE_STRING__
#define _CORE_STRING__

#include <core/class.h>
#include <core/result.h>
#include <core/slice.h>
#include <core/traits.h>

#define TRAIT_STRING_BUILD(T) TRAIT_REQUIRED(T, Result, build, char *s)

#define TRAIT_STRING_CORE(T)                                                   \
	TRAIT_REQUIRED(T, Result, substring, T##Ptr *s, u64 start, u64 end)    \
	TRAIT_REQUIRED(T, Result, index_of, T##Ptr *s, T##Ptr *n, u64 start)   \
	TRAIT_REQUIRED(T, Result, last_index_of, T##Ptr *s, T##Ptr *n)         \
	TRAIT_REQUIRED(T, Result, index_of_s, T##Ptr *s, char *n, u64 start)   \
	TRAIT_REQUIRED(T, Result, last_index_of_s, T##Ptr *s, char *n)         \
	TRAIT_REQUIRED(T, Result, char_at, T##Ptr *s, u64 index)               \
	TRAIT_REQUIRED(T, Result, from_slice, Slice *s, u64 len)

// have to include this here because Formatter depends on String
typedef struct Formatter Formatter;

CLASS(String, FIELD(char *, ptr) FIELD(u64, len))
IMPL(String, TRAIT_CLONE)
IMPL(String, TRAIT_EQUAL)
IMPL(String, TRAIT_STRING_BUILD)
IMPL(String, TRAIT_UNWRAP)
IMPL(String, TRAIT_DISPLAY)
IMPL(String, TRAIT_LEN)
IMPL(String, TRAIT_APPEND)
IMPL(String, TRAIT_PRINT)
IMPL(String, TRAIT_STRING_CORE)
#define String DEFINE_CLASS(String)

#define STRING(s)                                                              \
	({                                                                     \
		Result _rr33__ = String_build(s);                              \
		StringPtr _rr44__ = TRY(_rr33__, _rr44__);                     \
		_rr44__;                                                       \
	})

#define STRINGP(s)                                                             \
	({                                                                     \
		Result _rr33__ = String_build(s);                              \
		StringPtr _rr44__ = EXPECT(_rr33__, _rr44__);                  \
		_rr44__;                                                       \
	})

#define CHAR_AT(s, index)                                                      \
	({                                                                     \
		Result _r88__ = String_char_at(s, index);                      \
		if (IS_ERR(_r88__)) {                                          \
			Error _err__ = UNWRAP_ERR(_r88__);                     \
			return Err(_err__);                                    \
		}                                                              \
		i8 _rr99__ = UNWRAP_VALUE(_r88__, _rr99__);                     \
		_rr99__;                                                       \
	})

#define CHAR_ATP(s, index)                                                     \
	({                                                                     \
		Result _r88__ = String_char_at(s, index);                      \
		if (IS_ERR(_r88__)) {                                          \
			Error _err__ = UNWRAP_ERR(_r88__);                     \
			return ErrP(_err__);                                   \
		}                                                              \
		i8 _rr99__ = UNWRAP_VALUE(_r88__, _rr99__);                     \
		_rr99__;                                                       \
	})

#define INDEX_OF(s, n)                                                         \
	_Generic((n),                                                          \
	    char *: ({                                                         \
			 Result _r111__ = String_index_of_s(s, (char *)n, 0);  \
			 i64 _r222__ = TRY(_r111__, _r222__);                  \
			 _r222__;                                              \
		 }),                                                           \
	    default: ({                                                        \
			 Result _r111__ = String_index_of(s, (String *)n, 0);  \
			 i64 _r222__ = TRY(_r111__, _r222__);                  \
			 _r222__;                                              \
		 }))

#define LAST_INDEX_OF(s, n)                                                    \
	_Generic((n),                                                          \
	    char *: ({                                                         \
			 Result _r111__ =                                      \
			     String_last_index_of_s(s, (char *)n);             \
			 i64 _r222__ = TRY(_r111__, _r222__);                  \
			 _r222__;                                              \
		 }),                                                           \
	    default: ({                                                        \
			 Result _r111__ =                                      \
			     String_last_index_of(s, (String *)n);             \
			 i64 _r222__ = TRY(_r111__, _r222__);                  \
			 _r222__;                                              \
		 }))

#define SUBSTRING(s, start, end)                                               \
	({                                                                     \
		Result _r333__ = String_substring(s, start, end);              \
		StringPtr _r444__ = TRY(_r333__, _r444__);                     \
		_r444__;                                                       \
	})

#endif // _CORE_STRING__
