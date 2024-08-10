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

#ifndef _CORE_TRAITS_CORE__
#define _CORE_TRAITS_CORE__

#include <core/class.h>

#define TRAIT_EQUAL(T) TRAIT_REQUIRED(T, bool, equal, T##Ptr *dst, T##Ptr *src)
#define TRAIT_CLONE(T)                                                         \
	TRAIT_REQUIRED(T, bool, myclone, T##Ptr *dst, T##Ptr *src)
#define TRAIT_LEN(T) TRAIT_REQUIRED(T, u64, len, T##Ptr *obj)
#define TRAIT_COPY(T)                                                          \
	TRAIT_IMPL(T, copy, default_copy)                                      \
	TRAIT_SUPER(T, TRAIT_CLONE)
#define TRAIT_DEEP_COPY(T)                                                     \
	TRAIT_REQUIRED(T, bool, deep_copy, T##Ptr *dst, T##Ptr *src)

#define TRAIT_TO_STR(T) TRAIT_REQUIRED(T, char *, to_str, T##Ptr *obj)
#define TRAIT_TO_STR_BUF(T)                                                    \
	TRAIT_REQUIRED(T, void, to_str_buf, T##Ptr *obj, char *buf, u64 max_len)

#define TRAIT_PRINT(T) TRAIT_REQUIRED(T, void, print, T##Ptr *obj)

#define TRAIT_UNWRAP(T) TRAIT_REQUIRED(T, void *, unwrap, T##Ptr *obj)

#define TRAIT_UNWRAP_AS(T)                                                     \
	TRAIT_REQUIRED(T, void *, unwrap_as, char *class_name, T##Ptr *obj)

#define TRAIT_TEST(T) TRAIT_IMPL(T, test1, test1_default)

// trait implementations
bool equal(void *obj1, void *obj2);
void *unwrap(void *obj);
void *unwrap_as(char *name, void *obj);
u64 mysize(void *obj);
bool copy(void *dest, void *src);
bool myclone(void *dest, void *src);
void cleanup(void *ptr);
char *to_str(void *s);
void print(void *ptr);
u64 len(void *obj);
bool deep_copy(void *dst, void *src);

// default implementations
bool default_copy(void *dst, void *src);

#endif // _CORE_TRAITS_CORE__
