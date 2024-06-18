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

#define DEFINE_PRIM_IMPLS(prim_type, type)                                     \
	void type##_cleanup(type *ptr) {}                                      \
	size_t type##_size(type *ptr) { return sizeof(type); }                 \
	void *type##_unwrap(type *ptr) { return &ptr->_value; }                \
	bool type##_copy(type *dst, type *src) {                               \
		memcpy(&dst->_value, &src->_value, sizeof(prim_type));         \
		return true;                                                   \
	}

DEFINE_PRIM_IMPLS(i8, I8)
DEFINE_PRIM_IMPLS(i16, I16)
DEFINE_PRIM_IMPLS(i32, I32)
DEFINE_PRIM_IMPLS(i64, I64)
DEFINE_PRIM_IMPLS(i128, I128)

DEFINE_PRIM_IMPLS(u8, U8)
DEFINE_PRIM_IMPLS(u16, U16)
DEFINE_PRIM_IMPLS(u32, U32)
DEFINE_PRIM_IMPLS(U64, U64)
DEFINE_PRIM_IMPLS(u128, U128)

DEFINE_PRIM_IMPLS(bool, Bool)

