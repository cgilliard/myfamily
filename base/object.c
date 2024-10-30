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
#include <stdatomic.h>

#include <stdio.h>

#define OBJECT_FLAG_FAM_ALLOC_RESERVED1 0
#define OBJECT_FLAG_FAM_ALLOC_RESERVED2 1
#define OBJECT_FLAG_TYPE0 2
#define OBJECT_FLAG_TYPE1 3
#define OBJECT_FLAG_TYPE2 4

unsigned int object_get_size(ObjectType type) {
	if (type == ObjectTypeInt64)
		return sizeof(int64);
	else if (type == ObjectTypeByte)
		return sizeof(byte);
	else if (type == ObjectTypeBool)
		return sizeof(bool);
	else if (type == ObjectTypeFloat)
		return sizeof(float64);
	else if (type == ObjectTypeInt32)
		return sizeof(int);
	else if (type == ObjectTypeWeak)
		return sizeof(int64);
	else if (type == ObjectTypeBox)
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
	} else if (type == ObjectTypeBox) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, false);
	} else if (type == ObjectTypeFloat) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, true);
	} else if (type == ObjectTypeBool) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, true);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, false);
	} else if (type == ObjectTypeByte) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, true);
	}
}

Object object_create(ObjectType type, const void *value, bool send) {
	if (value == NULL || type < 0 || type >= __ObjectTypeCount__) {
		SetErr(IllegalArgument);
		return NULL;
	}

	unsigned int size = object_get_size(type);
	Ptr ret = fam_alloc(size, send);
	if (ret == NULL)
		return ret;

	memcpy($(ret), value, size);
	object_set_ptr_type(ret, type);
	int64 *aux = ptr_aux(ret);
	// set strong count to 1
	(*aux)++;

	return ret;
}

Object object_create_box(unsigned int size, bool send) {
	unsigned int box_size = object_get_size(ObjectTypeBox);
	Ptr ret = fam_alloc(box_size, send);
	if (ret == NULL)
		return ret;
	Ptr ptr = fam_alloc(size, send);
	if (ptr == NULL) {
		fam_release(&ret);
		return NULL;
	}
	memcpy($(ret), &ptr, box_size);
	object_set_ptr_type(ret, ObjectTypeBox);
	int64 *aux = ptr_aux(ret);

	// set strong count to 1
	(*aux)++;

	return ret;
}

const void *object_value_of(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	return $(obj);
}

ObjectType object_type(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE0)) {
		if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE1)) {
			if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE2))
				return ObjectTypeInt64;
			return ObjectTypeInt32;
		} else {
			if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE2))
				return ObjectTypeWeak;
			else
				return ObjectTypeBox;
		}
	} else {
		if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE1)) {
			if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE2))
				return ObjectTypeFloat;
			else
				return ObjectTypeBool;
		} else {
			if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE2))
				return ObjectTypeByte;
		}
	}
	return ObjectTypeBool;
}
unsigned int object_size(const Object obj) {
	return object_get_size(object_type(obj));
}

int object_mutate(Object obj, const void *value) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	if (object_type(obj) == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return -1;
	}

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
	bool send = object_get_ptr_flag(obj, PTR_FLAGS_SEND);
	if (send) {
		_Atomic int64 *aux = (_Atomic int64 *)ptr_aux(obj);
		int64 aux_val = atomic_fetch_sub(aux, 1);
		if ((aux_val & 0xFFFFFF) > 1) {
			// return strong count
			return aux_val - 1;
		} else {
			// return weak count
			return (aux_val & 0xFFFFFF000000LL) >> 24;
		}
	} else {
		int64 *aux = ptr_aux(obj);
		int64 count = *aux & 0xFFFFFF;
		int64 flags = *aux & 0xFFFFFFFFFF000000LL;
		count--;
		*aux = count | flags;
		if (count) {
			// return strong count
			return count;
		} else {
			// return the weak count
			return (*aux & 0xFFFFFF000000LL) >> 24;
		}
	}
}

int object_increment_strong(Object obj) {
	bool send = object_get_ptr_flag(obj, PTR_FLAGS_SEND);
	if (send) {
		_Atomic int64 *aux = (_Atomic int64 *)ptr_aux(obj);
		int64 old_count;
		int64 new_count;
		do {
			old_count = atomic_load(aux); // Atomically load the current count
			new_count = old_count + 1;
			if ((new_count & 0xFFFFFF) == 0xFFFFFF) { // Check for overflow
				SetErr(Overflow);
				return -1;
			}
		} while (!atomic_compare_exchange_weak(aux, &old_count, new_count));
		// Try to atomically increment the count if it hasn't changed

	} else {
		int64 *aux = ptr_aux(obj);
		if ((*aux & 0xFFFFFF) == 0xFFFFFF) {
			SetErr(Overflow);
			return -1;
		}

		(*aux)++;
	}

	return 0;
}

int object_decrement_weak(Object obj) {
	bool send = object_get_ptr_flag(obj, PTR_FLAGS_SEND);

	if (send) {
		_Atomic int64 *aux = (_Atomic int64 *)ptr_aux(obj);
		int64 aux_val = atomic_fetch_sub(aux, 0x1 << 24);
		int64 weak_count = (aux_val & 0xFFFFFF000000LL) >> 24;
		if (weak_count > 1) {
			// return weak count
			return weak_count - 1;
		} else {
			// return strong count
			return aux_val & 0xFFFFFF;
		}
	} else {
		int64 *aux = ptr_aux(obj);
		int64 count = *aux & 0xFFFFFF000000LL;
		int64 flags = *aux & 0xFFFF000000FFFFFFLL;
		count -= (0x1 << 24);
		*aux = count | flags;

		if (count) {
			// return weak count
			return count;
		} else {
			// return the strong count
			return *aux & 0xFFFFFF;
		}
	}
}

/*
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
*/

int object_increment_weak(Object obj) {
	bool send = object_get_ptr_flag(obj, PTR_FLAGS_SEND);
	if (send) {
		_Atomic int64 *aux = (_Atomic int64 *)ptr_aux(obj);
		int64 old_count;
		int64 new_count;
		do {
			old_count = atomic_load(aux); // Atomically load the current count
			new_count = old_count + (0x1 << 24);
			if ((new_count & 0xFFFFFF000000LL) == 0xFFFFFF000000LL) { // Check for overflow
				SetErr(Overflow);
				return -1;
			}
		} while (!atomic_compare_exchange_weak(aux, &old_count, new_count));
		// Try to atomically increment the count if it hasn't changed

	} else {
		int64 *aux = ptr_aux(obj);
		if ((*aux & 0xFFFFFF000000LL) == 0xFFFFFF000000LL) {
			SetErr(Overflow);
			return -1;
		}
		(*aux) += (0x1 << 24);
	}

	return 0;
}

Object object_ref_impl(Object src, bool incr) {
	if (nil(src)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	if (object_type(src) == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}

	if (incr) {
		if (object_increment_strong(src))
			return NULL;
	}

	ObjectNc ret = src;
	return ret;
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
	if (object_type(src) == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}
	if (nil(src)) {
		SetErr(ObjectConsumed);
		return NULL;
	}

	bool send = object_get_ptr_flag(src, PTR_FLAGS_SEND);
	ObjectNc ret = object_create(object_type(src), $(src), send);
	Object_cleanup(&src);
	return ret;
}
Object object_ref(const Object src) {
	return object_ref_impl(src, true);
}

Object object_weak(const Object src) {
	if (nil(src)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	if (object_type(src) == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}
	unsigned long long v = (unsigned long long)src;
	bool send = object_get_ptr_flag(src, PTR_FLAGS_SEND);
	ObjectNc weak = object_create(ObjectTypeWeak, &v, send);
	if (object_increment_weak(src))
		return NULL;

	return weak;
}

Object object_upgrade(const Object src) {
	if (nil(src)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	if (object_type(src) != ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}

	unsigned long long *target = object_value_of(src);
	ObjectNc w = (ObjectNc)*target;
	bool send = object_get_ptr_flag(src, PTR_FLAGS_SEND);
	if (send) {
		_Atomic int64 *aux = ptr_aux(w);
		int64 old_count;
		do {
			old_count = atomic_load(aux);
			if ((old_count & 0xFFFFFF) == 0) {
				return NULL; // Object is already deallocated
			}
		} while (!atomic_compare_exchange_weak(aux, &old_count, old_count + 1));

	} else {
		int64 *aux = ptr_aux(w);
		int64 strong_count = *aux & 0xFFFFFF;

		if (!strong_count) {
			return NULL;
		}
		(*aux)++;
	}

	return object_ref_impl(w, false);
}

void Object_cleanup(const Object *obj) {
	if (!nil(*obj)) {
		// deallocate the pointer
		if (object_type(*obj) == ObjectTypeBox) {
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
