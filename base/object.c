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

#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/object.h>
#include <base/string.h>

#include <stdio.h>

// types:
// int64: 64 bit signed
// int: 32 bit signed
// string: string (binary string)
// byte: unsigned 8 bit byte
// object: contains other primitives and objects as properties

#define OBJECT_FLAG_FAM_ALLOC_RESERVED1 0
#define OBJECT_FLAG_FAM_ALLOC_RESERVED2 1
#define OBJECT_FLAG_TYPE0 2
#define OBJECT_FLAG_TYPE1 3
#define OBJECT_FLAG_TYPE2 4
// for objects this should always be 0, we use this to ensure we're not operating on a string
// directly
#define OBJECT_FLAG_STRING 5

typedef struct ObjectString {
	string s;
} ObjectString;

Object object_create_string(string s) {

	Ptr ret = fam_alloc(sizeof(ObjectString), false);
	if (nil(ret))
		return ret;
	ObjectString os;
	os.s = string_ref(s);
	*(ObjectString *)$(ret) = os;

	// set type flags for int64
	int64 *aux = ptr_aux(ret);
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56;
	*aux &= ~(0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56;
	*aux &= ~(0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56;

	return ret;
}

Object object_create_int64(int64 v) {
	Ptr ret = fam_alloc(sizeof(int64), false);
	if (nil(ret))
		return NULL;

	int64 *value = $(ret);
	*value = v;

	// set type flags for int64
	int64 *aux = ptr_aux(ret);
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56;
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56;
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56;

	return ret;
}

Object object_create_int(int v) {
	Ptr ret = fam_alloc(sizeof(int), false);
	if (nil(ret))
		return NULL;

	int *value = $(ret);
	*value = v;

	// set type flags for int
	int64 *aux = ptr_aux(ret);
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56;
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56;
	*aux &= ~((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56);

	return ret;
}

Object object_create_byte(byte v) {
	Ptr ret = fam_alloc(sizeof(byte), false);
	if (nil(ret))
		return NULL;

	byte *value = $(ret);
	*value = v;

	// set type flags for byte
	int64 *aux = ptr_aux(ret);
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56;
	*aux &= ~((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56);
	*aux |= (0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56;

	return ret;
}

int64 object_as_int64(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	// check flags
	int64 *aux = ptr_aux(obj);
	bool type0 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56) & *aux;
	bool type1 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56) & *aux;
	bool type2 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56) & *aux;
	if (!type0 || !type1 || !type2) {
		SetErr(TypeMismatch);
		return -1;
	}

	int64 *value = $(obj);
	return *value;
}

int object_as_int(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	// check flags
	int64 *aux = ptr_aux(obj);
	bool type0 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56) & *aux;
	bool type1 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56) & *aux;
	bool type2 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56) & *aux;
	if (!type0 || !type1 || type2) {
		SetErr(TypeMismatch);
		return -1;
	}

	int *value = $(obj);
	return *value;
}

byte object_as_byte(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	// check flags
	int64 *aux = ptr_aux(obj);
	bool type0 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56) & *aux;
	bool type1 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56) & *aux;
	bool type2 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56) & *aux;
	if (!type0 || type1 || !type2) {
		SetErr(TypeMismatch);
		return BYTE_MAX;
	}

	int *value = $(obj);
	return *value;
}

string object_as_string(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	int64 *aux = ptr_aux(obj);
	// check flags
	bool type0 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56) & *aux;
	bool type1 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56) & *aux;
	bool type2 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56) & *aux;

	if (!type0 || type1 || type2) {
		SetErr(TypeMismatch);
		return NULL;
	}

	ObjectString *os = $(obj);

	return string_ref(os->s);
}

// Functions that require override of const
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

Object object_move(Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	// check flags
	int64 *aux = ptr_aux(obj);
	bool type0 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56) & *aux;
	bool type1 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56) & *aux;
	bool type2 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56) & *aux;

	if (type0 && type1 && type2) {
		// int64 type
		int *value = $(obj);
		ObjectNc ret = object_create_int64(*value);
		Object_cleanup(&obj);
		return ret;
	} else if (type0 && type1 && !type2) {
		// int type
		int *value = $(obj);
		ObjectNc ret = object_create_int(*value);
		Object_cleanup(&obj);
		return ret;
	} else if (type0 && !type1 && type2) {
		// byte type
		byte *value = $(obj);
		ObjectNc ret = object_create_byte(*value);
		Object_cleanup(&obj);
		return ret;
	} else if (type0 && !type1 && !type2) {
		// string type
		ObjectString *os = $(obj);
		ObjectNc ret = object_create_string(os->s);
		return ret;
	}

	return NULL;
}

Object object_ref(Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return NULL;
	}

	int64 *aux = ptr_aux(obj);
	(*aux)++;
	ObjectNc ret = obj;

	return ret;
}

void Object_cleanup(const Object *ptr) {
	if (!nil(*ptr)) {
		int64 *aux = ptr_aux(*ptr);
		int64 count = *aux & 0x00FFFFFFFFFFFFFFLL;
		int64 flags = *aux & 0xFF00000000000000LL;
		if (count == 0) {
			// cleanup string
			bool type0 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE0) << 56) & *aux;
			bool type1 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE1) << 56) & *aux;
			bool type2 = ((0x1ULL << (unsigned long long)OBJECT_FLAG_TYPE2) << 56) & *aux;
			if (type0 && !type1 && !type2) {
				ObjectString *os = $(*ptr);
				string_cleanup(&os->s);
			}
			fam_release(ptr);
		} else {
			count--;
			*aux = count | flags;
		}
	}
}
