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

void default_print_hi(void *obj);

#define TRAIT_COPY(T)                                                          \
	TRAIT_REQUIRED(T, bool, copy, T##Ptr *dst, T##Ptr *src)                \
	TRAIT_IMPL(T, print_hi, default_print_hi)
#define TRAIT_SIZE(T) TRAIT_REQUIRED(T, size_t, size, T##Ptr *obj)

void print_hi(void *obj);
bool equal(void *obj1, void *obj2);
void *unwrap(void *obj);
void *unwrap_err(void *obj);
size_t size(void *obj);
bool copy(void *dest, void *src);
void cleanup(void *ptr);
char *to_str(void *s);

#endif // _TRAITS_BASE__
