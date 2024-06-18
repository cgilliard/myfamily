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

#ifndef _STRING_BASE__
#define _STRING_BASE__

#include <base/class.h>
#include <base/result.h>

#define TRAIT_STRING_BUILD(T)                                                  \
	TRAIT_REQUIRED(T, Result, build, const char *s)                        \
	TRAIT_REQUIRED(T, T##Ptr, build_expect, const char *s)

CLASS(String, FIELD(char *, ptr) FIELD(u64, len))
IMPL(String, TRAIT_STRING_BUILD)
IMPL(String, TRAIT_COPY)
IMPL(String, TRAIT_EQUAL)
IMPL(String, TRAIT_UNWRAP)
#define String DEFINE_CLASS(String)

#define STRINGP(s) String_build_expect(s)
#define STRING(s) String_build(s)

#endif // _STRING_BASE__
