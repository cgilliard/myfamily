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

Result append(void *dst, void *src);
Result deep_copy(void *dst, void *src);

#define TRAIT_STRINGREF_BUILD(T) TRAIT_REQUIRED(T, Result, build, char *s)

#define TRAIT_STRING_BUILD(T)                                                  \
	TRAIT_REQUIRED(T, T##Ptr, build_expect, const char *s)                 \
	TRAIT_REQUIRED(T, T##Ptr *, build_ptr_expect, const char *s)           \
	TRAIT_REQUIRED(T, Result, build_ptr_try, char *s)

CLASS(String, FIELD(char *, ptr) FIELD(u64, len))
IMPL(String, TRAIT_STRING_BUILD)
IMPL(String, TRAIT_CLONE)
IMPL(String, TRAIT_EQUAL)
IMPL(String, TRAIT_UNWRAP)
IMPL(String, TRAIT_APPEND)
#define String DEFINE_CLASS(String)
#define STRINGPTRP(s) String_build_ptr_expect(s)
#define STRINGPTR(s) String_build_ptr_try(s)

CLASS(StringRef, FIELD(RcPtr *, ptr))
IMPL(StringRef, TRAIT_STRINGREF_BUILD)
IMPL(StringRef, TRAIT_COPY)
IMPL(StringRef, TRAIT_UNWRAP)
IMPL(StringRef, TRAIT_CLONE)
IMPL(StringRef, TRAIT_EQUAL)
IMPL(StringRef, TRAIT_APPEND)
IMPL(StringRef, TRAIT_DEEP_COPY)
#define StringRef DEFINE_CLASS(StringRef)

StringRef StringRef_buildp(char *s);
Result StringRef_build(char *s);
#define STRING(x) StringRef_build(x)
#define STRINGP(x) StringRef_buildp(x)

#endif // _BASE_STRING__
