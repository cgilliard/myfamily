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

#include <core/tuple.h>
#include <core/unit.h>

GETTER(Tuple, count)
SETTER(Tuple, count)
GETTER(Tuple, elements)
SETTER(Tuple, elements)

void Tuple_cleanup(Tuple *tuple) {
	u64 count = *Tuple_get_count(tuple);
	void **elements = *Tuple_get_elements(tuple);
	for (u64 i = 0; i < count; i++) {
		cleanup(elements[i]);
		myfree(elements[i]);
	}

	myfree(elements);
}

bool Tuple_clone(Tuple *dst, Tuple *src) {
	u64 count_src = *Tuple_get_count(src);
	void **elements_src = *Tuple_get_elements(src);

	void **elements_dst = mymalloc(sizeof(void *) * count_src);
	if (elements_dst == NULL) {
		Tuple_set_count(dst, 0);
		return false;
	}
	for (u64 i = 0; i < count_src; i++) {
		elements_dst[i] = mymalloc(size(elements_src[i]));
		if (elements_dst[i] == NULL) {
			for (u64 j = i - 1; j >= 0; j--) {
				myfree(elements_dst[i]);
			}
			myfree(elements_dst);
			Tuple_set_count(dst, 0);
			return false;
		}
		copy(elements_dst[i], elements_src[i]);
	}

	Tuple_set_elements(dst, elements_dst);
	Tuple_set_count(dst, count_src);
	return true;
}

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
		memcpy(dst, ref, sizeof(u64));
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
			if (!copy(dst, ref))
				panic("Could not copy object");
		}
	}
}

void *Tuple_add_value(void *value) {
	if (!implements(value, "copy")) {
		panic("copy is required. RC can "
		      "be used to wrap other types");
	}
	void *ret = mymalloc(size(value));
	if (!ret)
		return NULL;
	if (!copy(ret, value)) {
		myfree(ret);
		return NULL;
	}
	return ret;
}

void *Tuple_add_value_i128(void *value) {
	I128Ptr *ret = mymalloc(sizeof(I128));
	if (!ret)
		return NULL;
	BUILDPTR(ret, I128);
	ret->_value = *(i128 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i64(void *value) {
	I64Ptr *ret = mymalloc(sizeof(I64));
	if (!ret)
		return NULL;
	BUILDPTR(ret, I64);
	ret->_value = *(i64 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i32(void *value) {
	I32Ptr *ret = mymalloc(sizeof(I32));
	if (!ret)
		return NULL;
	BUILDPTR(ret, I32);
	ret->_value = *(i32 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i16(void *value) {
	I16Ptr *ret = mymalloc(sizeof(I16));
	if (!ret)
		return NULL;
	BUILDPTR(ret, I16);
	ret->_value = *(i16 *)value;
	return (void *)ret;
}

void *Tuple_add_value_i8(void *value) {
	I8Ptr *ret = mymalloc(sizeof(I8));
	if (!ret)
		return NULL;
	BUILDPTR(ret, I8);
	ret->_value = *(i8 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u128(void *value) {
	U128Ptr *ret = mymalloc(sizeof(U128));
	if (!ret)
		return NULL;
	BUILDPTR(ret, U128);
	ret->_value = *(u128 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u64(void *value) {
	U64Ptr *ret = mymalloc(sizeof(U64));
	if (!ret)
		return NULL;
	BUILDPTR(ret, U64);
	ret->_value = *(u64 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u32(void *value) {
	U32Ptr *ret = mymalloc(sizeof(U32));
	if (!ret)
		return NULL;
	BUILDPTR(ret, U32);
	ret->_value = *(u32 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u16(void *value) {
	U16Ptr *ret = mymalloc(sizeof(U16));
	if (!ret)
		return NULL;
	BUILDPTR(ret, U16);
	ret->_value = *(u16 *)value;
	return (void *)ret;
}

void *Tuple_add_value_u8(void *value) {
	U8Ptr *ret = mymalloc(sizeof(U8));
	if (!ret)
		return NULL;
	BUILDPTR(ret, U8);
	ret->_value = *(u8 *)value;
	return (void *)ret;
}

void *Tuple_add_value_bool(void *value) {
	BoolPtr *ret = mymalloc(sizeof(Bool));
	if (!ret)
		return NULL;
	BUILDPTR(ret, Bool);
	ret->_value = *(bool *)value;
	return (void *)ret;
}

void *Tuple_add_value_f64(void *value) {
	F64Ptr *ret = mymalloc(sizeof(F64));
	if (!ret)
		return NULL;
	BUILDPTR(ret, F64);
	ret->_value = *(f64 *)value;
	return (void *)ret;
}

void *Tuple_add_value_f32(void *value) {
	F32Ptr *ret = mymalloc(sizeof(F32));
	if (!ret)
		return NULL;
	BUILDPTR(ret, F32);
	ret->_value = *(f32 *)value;
	return (void *)ret;
}
