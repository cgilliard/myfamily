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

#include <core/prim.h>
#include <core/result.h>
#include <core/unit.h>
#include <stdio.h>

#include <core/formatter.h>

#define DEFINE_PRIM_IMPLS(prim_type, type, format)                             \
	void type##_cleanup(type *ptr) {}                                      \
	void type##_to_str_buf(type *ptr, char *buf, u64 max_len) {            \
		snprintf(buf, max_len, "%" format, ptr->_value);               \
	}                                                                      \
	void *type##_unwrap(type *ptr) { return &ptr->_value; }                \
	bool type##_myclone(type *dst, type *src) {                            \
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
DEFINE_PRIM_IMPLS(u64, U64, PRIu64)

void U128_cleanup(U128 *ptr) {}

void U128_to_str_buf(U128 *ptr, char *buf, u64 max_len) {
	u128 value = ptr->_value;
	if (value < UINT64_MAX)
		snprintf(buf, max_len, "%" PRIu64, value);
	else
		snprintf(buf, max_len, "U128");
}

void *U128_unwrap(U128 *ptr) { return &ptr->_value; }
bool U128_myclone(U128 *dst, U128 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(u128));
	return true;
}

void I128_cleanup(I128 *ptr) {}
void I128_to_str_buf(I128 *ptr, char *buf, u64 max_len) {
	i128 value = ptr->_value;
	if (value < INT64_MAX && value > INT64_MIN)
		snprintf(buf, max_len, "%" PRId64, value);
	else
		snprintf(buf, max_len, "I128");
}
void *I128_unwrap(I128 *ptr) { return &ptr->_value; }
bool I128_myclone(I128 *dst, I128 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(i128));
	return true;
}

void Bool_cleanup(Bool *ptr) {}
void Bool_to_str_buf(Bool *ptr, char *buf, u64 max_len) {
	bool value = ptr->_value;
	if (value)
		snprintf(buf, max_len, "true");
	else
		snprintf(buf, max_len, "false");
}
void *Bool_unwrap(Bool *ptr) { return &ptr->_value; }
bool Bool_myclone(Bool *dst, Bool *src) {
	memcpy(&dst->_value, &src->_value, sizeof(bool));
	return true;
}

void USize_cleanup(USize *ptr) {}
void USize_to_str_buf(USize *ptr, char *buf, u64 max_len) {
	u64 value = ptr->_value;
	snprintf(buf, max_len, "%zu", value);
}
void *USize_unwrap(USize *ptr) { return &ptr->_value; }
bool USize_myclone(USize *dst, USize *src) {
	memcpy(&dst->_value, &src->_value, sizeof(u64));
	return true;
}

void ISize_cleanup(ISize *ptr) {}
void ISize_to_str_buf(ISize *ptr, char *buf, u64 max_len) {
	u64 value = ptr->_value;
	snprintf(buf, max_len, "%zu", value);
}
void *ISize_unwrap(ISize *ptr) { return &ptr->_value; }
bool ISize_myclone(ISize *dst, ISize *src) {
	memcpy(&dst->_value, &src->_value, sizeof(u64));
	return true;
}

void F32_cleanup(F32 *ptr) {}
void F32_to_str_buf(F32 *ptr, char *buf, u64 max_len) {
	f32 value = ptr->_value;
	snprintf(buf, max_len, "%f", value);
}
void *F32_unwrap(F32 *ptr) { return &ptr->_value; }
bool F32_myclone(F32 *dst, F32 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(f32));
	return true;
}

void F64_cleanup(F64 *ptr) {}
void F64_to_str_buf(F64 *ptr, char *buf, u64 max_len) {
	f64 value = ptr->_value;
	snprintf(buf, max_len, "%e", value);
}
void *F64_unwrap(F64 *ptr) { return &ptr->_value; }
bool F64_myclone(F64 *dst, F64 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(f64));
	return true;
}
