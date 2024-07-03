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

#include <base/string.h>
#include <base/tuple.h>
#include <base/unit.h>

GETTER(Tuple, count)
SETTER(Tuple, count)
GETTER(Tuple, elements)
SETTER(Tuple, elements)

void Tuple_cleanup(Tuple *tuple) {
	u64 count = *Tuple_get_count(tuple);
	void **elements = *Tuple_get_elements(tuple);
	for (u64 i = 0; i < count; i++) {
		cleanup(elements[i]);
		tlfree(elements[i]);
	}

	tlfree(elements);
}

bool Tuple_clone(Tuple *dst, Tuple *src) {
	u64 count_src = *Tuple_get_count(src);
	void **elements_src = *Tuple_get_elements(src);

	void **elements_dst = tlmalloc(sizeof(void *) * count_src);
	if (elements_dst == NULL) {
		Tuple_set_count(dst, 0);
		return false;
	}
	for (u64 i = 0; i < count_src; i++) {
		elements_dst[i] = tlmalloc(size(elements_src[i]));
		if (elements_dst[i] == NULL) {
			for (u64 j = i - 1; j >= 0; j--) {
				tlfree(elements_dst[i]);
			}
			tlfree(elements_dst);
			Tuple_set_count(dst, 0);
			return false;
		}
		copy(elements_dst[i], elements_src[i]);
	}

	Tuple_set_elements(dst, elements_dst);
	Tuple_set_count(dst, count_src);
	return true;
}
Result Tuple_dbg(Tuple *obj, Formatter *f) {
	u64 count = *Tuple_get_count(obj);
	void **elements = *Tuple_get_elements(obj);
	Result r0 = WRITE(f, "Tuple(");
	Try(Unit, r0);
	for (u64 i = 0; i < count; i++) {
		if (implements(elements[i], "dbg") ||
		    implements(elements[i], "to_str_buf")) {
			Result r = to_debug(elements[i]);
			Try(Unit, r);
			StringRef item = Try(StringRef, r);
			Result r2 = WRITE(f, "%s", to_str(&item));
			Try(Unit, r2);
		} else {
			Result r3 = WRITE(f, "%s", CLASS_NAME(elements[i]));
			Try(Unit, r3);
		}
		if (i != count - 1) {
			Result r4 = WRITE(f, ", ");
			Try(Unit, r4);
		}
	}
	Result r5 = WRITE(f, ")");
	Try(Unit, r5);
	return Ok(UNIT);
}
bool Tuple_equal(Tuple *obj1, Tuple *obj2) { return false; }

u64 Tuple_len(Tuple *tuple) {
	u64 count = *Tuple_get_count(tuple);
	return count;
}

void Tuple_element_at(Tuple *tuple, u64 index, void *dst) {
	u64 count = *Tuple_get_count(tuple);
	if (index >= count)
		panic("Attempt to access element index %llu on a tuple of size "
		      "%llu",
		      index, count);
	void **elements = *Tuple_get_elements(tuple);
	void *ref = elements[index];

	char *cn = CLASS_NAME(ref);

	if (!strcmp(cn, "Bool")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(bool));
	} else if (!strcmp(cn, "U8")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(u8));
	} else if (!strcmp(cn, "U16")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(u16));
	} else if (!strcmp(cn, "U32")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(u32));
	} else if (!strcmp(cn, "U64")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(u64));
	} else if (!strcmp(cn, "U128")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(u128));
	} else if (!strcmp(cn, "I128")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(i128));
	} else if (!strcmp(cn, "I64")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(i64));
	} else if (!strcmp(cn, "I32")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(i32));
	} else if (!strcmp(cn, "I16")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(i16));
	} else if (!strcmp(cn, "I8")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(i8));
	} else if (!strcmp(cn, "USize")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(usize));
	} else if (!strcmp(cn, "F64")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(f64));
	} else if (!strcmp(cn, "F32")) {
		ref = unwrap(ref);
		memcpy(dst, ref, sizeof(f32));
	} else {
		// it's an object, we'll call copy which must be implemented
		if (!strcmp(cn, "Rc")) {
			void *tmp = unwrap(ref);
			memcpy(dst, tmp, size(tmp));
		} else {
			copy(dst, ref);
		}
	}
}

void *Tuple_add_value(void *value) {
	if (!implements(value, "copy")) {
		panic("copy is required. RC can "
		      "be used to wrap other types");
	}
	void *ret = tlmalloc(size(value));
	copy(ret, value);
	return ret;
}

void *Tuple_add_value_i128(void *value) {
	I128Ptr *ret = tlmalloc(sizeof(I128));
	BUILDPTR(ret, I128);
	ret->_value = *(i128 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i64(void *value) {
	I64Ptr *ret = tlmalloc(sizeof(I64));
	BUILDPTR(ret, I64);
	ret->_value = *(i64 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i32(void *value) {
	I32Ptr *ret = tlmalloc(sizeof(I32));
	BUILDPTR(ret, I32);
	ret->_value = *(i32 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i16(void *value) {
	I16Ptr *ret = tlmalloc(sizeof(I16));
	BUILDPTR(ret, I16);
	ret->_value = *(i16 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i8(void *value) {
	I8Ptr *ret = tlmalloc(sizeof(I8));
	BUILDPTR(ret, I8);
	ret->_value = *(i8 *)value;
	return (void *)ret;
}

void *Tuple_add_value_usize(void *value) {
	USizePtr *ret = tlmalloc(sizeof(USize));
	BUILDPTR(ret, USize);
	ret->_value = *(usize *)value;
	return (void *)ret;
}

void *Tuple_add_value_u128(void *value) {
	U128Ptr *ret = tlmalloc(sizeof(U128));
	BUILDPTR(ret, U128);
	ret->_value = *(u128 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u64(void *value) {
	U64Ptr *ret = tlmalloc(sizeof(U64));
	BUILDPTR(ret, U64);
	ret->_value = *(u64 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u32(void *value) {
	U32Ptr *ret = tlmalloc(sizeof(U32));
	BUILDPTR(ret, U32);
	ret->_value = *(u32 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u16(void *value) {
	U16Ptr *ret = tlmalloc(sizeof(U16));
	BUILDPTR(ret, U16);
	ret->_value = *(u16 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u8(void *value) {
	U8Ptr *ret = tlmalloc(sizeof(U8));
	BUILDPTR(ret, U8);
	ret->_value = *(u8 *)value;
	return (void *)ret;
}

void *Tuple_add_value_bool(void *value) {
	BoolPtr *ret = tlmalloc(sizeof(Bool));
	BUILDPTR(ret, Bool);
	ret->_value = *(bool *)value;
	return (void *)ret;
}

void *Tuple_add_value_f64(void *value) {
	F64Ptr *ret = tlmalloc(sizeof(F64));
	BUILDPTR(ret, F64);
	ret->_value = *(f64 *)value;
	return (void *)ret;
}

void *Tuple_add_value_f32(void *value) {
	F32Ptr *ret = tlmalloc(sizeof(F32));
	BUILDPTR(ret, F32);
	ret->_value = *(f32 *)value;
	return (void *)ret;
}
