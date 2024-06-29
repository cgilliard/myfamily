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

#include <base/formatter.h>
#include <base/result.h>
#include <base/string.h>
#include <base/traits_base.h>

#ifndef _BASE_TRAITS__
#define _BASE_TRAITS__

#define TO_STRING_BUF_SIZE 10000

Result append(void *dst, void *src);
Result deep_copy(void *dst, void *src);
u64 len(void *obj);
Result to_string(void *obj);
Result to_string_buf(void *obj, usize buffer_size);
Result to_debug(void *obj);
Result to_debug_buf(void *obj, usize buffer_size);

#define UNWRAP_AS(type, value) *(type *)unwrap_as(#type, &value)

#define TRAIT_APPEND(T)                                                        \
	TRAIT_REQUIRED(T, Result, append, T##Ptr *dst, T##Ptr *src)
#define TRAIT_DEEP_COPY(T)                                                     \
	TRAIT_REQUIRED(T, Result, deep_copy, T##Ptr *dst, T##Ptr *src)

#define TRAIT_LEN(T) TRAIT_REQUIRED(T, u64, len, T##Ptr *obj)

#define TRAIT_FORMATTER(T)                                                     \
	TRAIT_REQUIRED(T, Result, write, T##Ptr *f, char *fmt, ...)

#define TRAIT_TO_STR_REF(T) TRAIT_REQUIRED(T, Result, to_str_ref, T##Ptr *obj)

#define TRAIT_DISPLAY(T)                                                       \
	TRAIT_REQUIRED(T, Result, fmt, T##Ptr *self, Formatter *formatter)
#define TRAIT_DEBUG(T)                                                         \
	TRAIT_REQUIRED(T, Result, dbg, T##Ptr *self, Formatter *formatter)

#endif // _BASE_TRAITS__
