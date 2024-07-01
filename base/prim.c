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

DEFINE_PRIM_IMPLS(u8, U8, PRIu8)
DEFINE_PRIM_IMPLS(u16, U16, PRIu16)
DEFINE_PRIM_IMPLS(u32, U32, PRIu32)
DEFINE_PRIM_IMPLS(U64, U64, PRIu64)

void U128_cleanup(U128 *ptr) {}
usize U128_size(U128 *ptr) { return sizeof(U128); }

void U128_to_str_buf(U128 *ptr, char *buf, usize max_len) {
	u128 value = ptr->_value;
	if (value < UINT64_MAX)
		snprintf(buf, max_len, "%" PRIu64, value);
	else
		snprintf(buf, max_len, "U128");
}

void *U128_unwrap(U128 *ptr) { return &ptr->_value; }
bool U128_clone(U128 *dst, U128 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(u128));
	return true;
}

void I128_cleanup(I128 *ptr) {}
usize I128_size(I128 *ptr) { return sizeof(I128); }
void I128_to_str_buf(I128 *ptr, char *buf, usize max_len) {
	i128 value = ptr->_value;
	if (value < INT64_MAX && value > INT64_MIN)
		snprintf(buf, max_len, "%" PRId64, value);
	else
		snprintf(buf, max_len, "I128");
}
void *I128_unwrap(I128 *ptr) { return &ptr->_value; }
bool I128_clone(I128 *dst, I128 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(i128));
	return true;
}

void Bool_cleanup(Bool *ptr) {}
usize Bool_size(Bool *ptr) { return sizeof(Bool); }
void Bool_to_str_buf(Bool *ptr, char *buf, usize max_len) {
	bool value = ptr->_value;
	if (value)
		snprintf(buf, max_len, "true");
	else
		snprintf(buf, max_len, "false");
}
void *Bool_unwrap(Bool *ptr) { return &ptr->_value; }
bool Bool_clone(Bool *dst, Bool *src) {
	memcpy(&dst->_value, &src->_value, sizeof(bool));
	return true;
}

void USize_cleanup(USize *ptr) {}
usize USize_size(USize *ptr) { return sizeof(USize); }
void USize_to_str_buf(USize *ptr, char *buf, usize max_len) {
	usize value = ptr->_value;
	snprintf(buf, max_len, "%zu", value);
}
void *USize_unwrap(USize *ptr) { return &ptr->_value; }
bool USize_clone(USize *dst, USize *src) {
	memcpy(&dst->_value, &src->_value, sizeof(usize));
	return true;
}
