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
#include <base/osdef.h>
#include <base/string.h>

#include <stdio.h>

#define OBJECT_FLAG_FAM_ALLOC_RESERVED1 0
#define OBJECT_FLAG_FAM_ALLOC_RESERVED2 1
#define OBJECT_FLAG_TYPE0 2
#define OBJECT_FLAG_TYPE1 3
#define OBJECT_FLAG_TYPE2 4

unsigned int object_get_size(ObjectType type) {
	if (type == ObjectTypeInt64)
		return sizeof(int64);
	else if (type == ObjectTypeInt32)
		return sizeof(int);
	else if (type == ObjectTypeWeak)
		return sizeof(int64);
	else if (type == ObjectTypePointer)
		return sizeof(int64);

	return 0;
}

void object_set_ptr_flag(Ptr ptr, unsigned long long flag, bool value) {
	int64 *aux = ptr_aux(ptr);
	if (value)
		*aux |= (0x1ULL << (unsigned long long)flag) << 56;
	else
		*aux &= ~((0x1ULL << (unsigned long long)flag) << 56);
}

bool object_get_ptr_flag(Ptr ptr, unsigned long long flag) {
	int64 *aux = ptr_aux(ptr);
	return ((0x1ULL << (unsigned long long)flag) << 56) & *aux;
}

void object_set_ptr_type(Ptr ptr, ObjectType type) {
	if (type == ObjectTypeInt64) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, true);
	} else if (type == ObjectTypeInt32) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, false);
	} else if (type == ObjectTypeWeak) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, true);
	} else if (type == ObjectTypePointer) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, false);
	}
}

Object object_create(const void *value, ObjectType type) {
	unsigned int size = object_get_size(type);
	Ptr ret = fam_alloc(size, false);
	if (ret == NULL)
		return ret;

	memcpy($(ret), value, size);
	object_set_ptr_type(ret, type);
	int64 *aux = ptr_aux(ret);
	// set strong count to 1
	(*aux)++;

	return ret;
}
const void *object_value_of(const Object obj) {
	return $(obj);
}

ObjectType object_type(const Object obj) {
	if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE0)) {
		if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE1)) {
			if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE2))
				return ObjectTypeInt64;
			return ObjectTypeInt32;
		} else {
			if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE2))
				return ObjectTypeWeak;
			else
				return ObjectTypePointer;
		}
	}
	return ObjectTypeBool;
}
unsigned int object_size(const Object obj) {
	return object_get_size(object_type(obj));
}

int object_mutate(Object obj, const void *value) {
	unsigned int size = object_size(obj);
	if (size)
		memcpy($(obj), value, size);
	return 0;
}

int object_set_property(Object obj, const char *key, const Object value) {
	return 0;
}
Object object_delete_property(Object obj, const char *key) {
	return NULL;
}
Object object_get_property(const Object obj, const char *key) {
	return NULL;
}

int object_decrement_strong(Object obj) {
	int64 *aux = ptr_aux(obj);
	int64 count = *aux & 0xFFFFFF;
	int64 flags = *aux & 0xFFFFFFFFFF000000LL;
	count--;

	if (count) {
		*aux = count | flags;
		return count;

	} else {
		// set strong count to 0
		*aux = flags;

		// return the weak count
		return *aux & 0xFFFFFF000000LL;
	}
}

int object_increment_strong(Object obj) {
	// strong count is first three bytes. Mask it and ensure we're not overflowing
	int64 *aux = ptr_aux(obj);
	if ((*aux & 0xFFFFFF) == 0xFFFFFF) {
		SetErr(Overflow);
		return -1;
	}

	(*aux)++;
	return 0;
}

int object_decrement_weak(Object obj) {
	int64 *aux = ptr_aux(obj);
	int64 count = *aux & 0xFFFFFF000000LL;
	int64 flags = *aux & 0xFFFF000000FFFFFFLL;
	count -= (0x1 << 24);
	if (count) {
		*aux = count | flags;
		return count;

	} else {
		*aux = flags;
		// return the strong count
		return *aux & 0xFFFFFF;
	}
}

int object_increment_weak(Object obj) {
	// weak count is second three bytes. Mask it and ensure we're not overflowing
	int64 *aux = ptr_aux(obj);
	if ((*aux & 0xFFFFFF000000LL) == 0xFFFFFF000000LL) {
		SetErr(Overflow);
		return -1;
	}

	(*aux) += (0x1 << 24);

	return 0;
}

// Functions that require override of const
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

Object object_move(const Object src) {
	if (nil(src)) {
		SetErr(ObjectConsumed);
		return NULL;
	}

	ObjectNc ret = object_create($(src), object_type(src));
	Object_cleanup(&src);
	return ret;
}
Object object_ref(const Object src) {
	if (object_increment_strong(src))
		return NULL;

	ObjectNc ret = src;
	return ret;
}

Object object_weak(const Object src) {
	unsigned long long v = (unsigned long long)src;
	ObjectNc weak = object_create(&v, ObjectTypeWeak);
	if (object_increment_weak(src))
		return NULL;

	return weak;
}

Object object_upgrade(const Object src) {
	if (object_type(src) != ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}
	unsigned long long *target = object_value_of(src);
	ObjectNc w = (ObjectNc)*target;
	int64 *aux = ptr_aux(w);
	int64 strong_count = *aux & 0xFFFFFF;
	if (!strong_count)
		return NULL;

	// there are remaining strong references we can upgrade
	return object_ref(w);
}

void Object_cleanup(const Object *obj) {
	if (!nil(*obj)) {
		// deallocate the pointer
		if (object_type(*obj) == ObjectTypePointer) {
			Ptr inner = *(Ptr *)object_value_of(*obj);
			fam_release(&inner);
		}
		if (object_type(*obj) == ObjectTypeWeak) {
			unsigned long long *target = object_value_of(*obj);
			ObjectNc w = (ObjectNc)*target;

			int odwval = object_decrement_weak(w);
			if (!odwval) {
				fam_release(&w);
			}
			fam_release(obj);
		} else {
			int odsval = object_decrement_strong(*obj);
			if (!odsval) {
				fam_release(obj);
			}
		}
	}
}
