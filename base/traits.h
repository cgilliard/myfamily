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

#ifndef _TRAITS_BASE__
#define _TRAITS_BASE__

#include <base/class.h>

#define TRAIT_EQUAL(T) TRAIT_REQUIRED(T, bool, equal, T##Ptr *dst, T##Ptr *src)
#define TRAIT_CLONE(T) TRAIT_REQUIRED(T, bool, clone, T##Ptr *dst, T##Ptr *src)
#define TRAIT_COPY(T)                                                          \
	TRAIT_IMPL(T, copy, default_copy)                                      \
	TRAIT_SUPER(T, TRAIT_SIZE)                                             \
	TRAIT_SUPER(T, TRAIT_CLONE)

#define TRAIT_SIZE(T) TRAIT_REQUIRED(T, usize, size, T##Ptr *obj)

#define TRAIT_TO_STR(T) TRAIT_REQUIRED(T, char *, to_str, T##Ptr *obj)

#define TRAIT_PRINT(T) TRAIT_REQUIRED(T, void, print, T##Ptr *obj)

#define TRAIT_UNWRAP(T) TRAIT_REQUIRED(T, void *, unwrap, T##Ptr *obj)

#define TRAIT_TEST(T) TRAIT_IMPL(T, test1, test1_default)

// trait implementations
bool equal(void *obj1, void *obj2);
void *unwrap(void *obj);
usize size(void *obj);
bool copy(void *dest, void *src);
bool clone(void *dest, void *src);
void cleanup(void *ptr);
char *to_str(void *s);
void print(void *ptr);

// default implementations
bool default_copy(void *dst, void *src);

#endif // _TRAITS_BASE__
