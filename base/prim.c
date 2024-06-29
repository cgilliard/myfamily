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

#include <base/prim.h>
#include <base/result.h>
#include <base/unit.h>
#include <stdio.h>

#define DEFINE_PRIM_IMPLS(prim_type, type, format)                             \
	void type##_cleanup(type *ptr) {}                                      \
	usize type##_size(type *ptr) { return sizeof(type); }                  \
	void type##_to_str_buf(type *ptr, char *buf, usize max_len) {          \
		snprintf(buf, max_len, "%" format, ptr->_value);               \
	}                                                                      \
	void *type##_unwrap(type *ptr) { return &ptr->_value; }                \
	bool type##_clone(type *dst, type *src) {                              \
		memcpy(&dst->_value, &src->_value, sizeof(prim_type));         \
		return true;                                                   \
	}

DEFINE_PRIM_IMPLS(i8, I8, PRId8)
DEFINE_PRIM_IMPLS(i16, I16, PRId16)
DEFINE_PRIM_IMPLS(i32, I32, PRId32)
DEFINE_PRIM_IMPLS(i64, I64, PRId64)
// TODO: Need to make a custom implementation to display 128 bit correctly
DEFINE_PRIM_IMPLS(i128, I128, PRId64)

DEFINE_PRIM_IMPLS(u8, U8, PRIu8)
DEFINE_PRIM_IMPLS(u16, U16, PRIu16)
DEFINE_PRIM_IMPLS(u32, U32, PRIu32)
DEFINE_PRIM_IMPLS(U64, U64, PRIu64)
// TODO: Need to make a custom implementation to display 128 bit correctly
DEFINE_PRIM_IMPLS(u128, U128, PRIu64)

DEFINE_PRIM_IMPLS(bool, Bool, PRIu32)

