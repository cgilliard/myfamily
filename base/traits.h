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
#define TRAIT_COPY(T)                                                          \
	TRAIT_REQUIRED(T, bool, copy, T##Ptr *dst, T##Ptr *src)                \
	TRAIT_SUPER(T, TRAIT_SIZE)
#define TRAIT_SIZE(T) TRAIT_REQUIRED(T, size_t, size, T##Ptr *obj)

#define TRAIT_TO_STR(T) TRAIT_REQUIRED(T, char *, to_str, T##Ptr *obj)

#define TRAIT_PRINT(T) TRAIT_REQUIRED(T, void, print, T##Ptr *obj)

// trait implementations
bool equal(void *obj1, void *obj2);
void *unwrap(void *obj);
void *unwrap_err(void *obj);

#define size(s)                                                                \
	_Generic((*s),                                                         \
	    u128: size_impl_u128,                                              \
	    u64: size_impl_u64,                                                \
	    u32: size_impl_u32,                                                \
	    u16: size_impl_u16,                                                \
	    u8: size_impl_u8,                                                  \
	    i128: size_impl_i128,                                              \
	    i64: size_impl_i64,                                                \
	    i32: size_impl_i32,                                                \
	    i16: size_impl_i16,                                                \
	    i8: size_impl_i8,                                                  \
	    f64: size_impl_f64,                                                \
	    f32: size_impl_f32,                                                \
	    default: size_impl)(s)

size_t size_impl(void *obj);
size_t size_impl_u64(u64 *obj);

#define copy(d, s)                                                             \
	_Generic((*s),                                                         \
	    f64: copy_impl_f64,                                                \
	    f32: copy_impl_f32,                                                \
	    i128: copy_impl_i128,                                              \
	    i64: copy_impl_i64,                                                \
	    i32: copy_impl_i32,                                                \
	    i16: copy_impl_i16,                                                \
	    i8: copy_impl_i8,                                                  \
	    u128: copy_impl_u128,                                              \
	    u64: copy_impl_u64,                                                \
	    u32: copy_impl_u32,                                                \
	    u16: copy_impl_u16,                                                \
	    u8: copy_impl_u8,                                                  \
	    bool: copy_impl_bool,                                              \
	    default: copy_impl)(d, s)

bool copy_impl(void *dest, void *src);
#define DEFINE_COPY_IMPL(type)                                                 \
	bool copy_impl_##type(type *dst, type *src);                           \
	size_t size_impl_##type(type *ptr);
DEFINE_COPY_IMPL(f64)
DEFINE_COPY_IMPL(f32)
DEFINE_COPY_IMPL(i128)
DEFINE_COPY_IMPL(i64)
DEFINE_COPY_IMPL(i32)
DEFINE_COPY_IMPL(i16)
DEFINE_COPY_IMPL(i8)
DEFINE_COPY_IMPL(u128)
DEFINE_COPY_IMPL(u64)
DEFINE_COPY_IMPL(u32)
DEFINE_COPY_IMPL(u16)
DEFINE_COPY_IMPL(u8)
DEFINE_COPY_IMPL(bool)

#define dispose(x)                                                             \
	_Generic((*x),                                                         \
	    u8: no_cleanup,                                                    \
	    u16: no_cleanup,                                                   \
	    u32: no_cleanup,                                                   \
	    u64: no_cleanup,                                                   \
	    u128: no_cleanup,                                                  \
	    i8: no_cleanup,                                                    \
	    i16: no_cleanup,                                                   \
	    i32: no_cleanup,                                                   \
	    i64: no_cleanup,                                                   \
	    i128: no_cleanup,                                                  \
	    f32: no_cleanup,                                                   \
	    f64: no_cleanup,                                                   \
	    bool: no_cleanup,                                                  \
	    default: cleanup)(x)

void no_cleanup(void *ptr);
void cleanup(void *ptr);
char *to_str(void *s);
void print(void *ptr);

#endif // _TRAITS_BASE__
