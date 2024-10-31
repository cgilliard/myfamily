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

#include <stdio.h>

#define OBJECT_FLAG_FAM_ALLOC_RESERVED1 0
#define OBJECT_FLAG_FAM_ALLOC_RESERVED2 1
#define OBJECT_FLAG_TYPE0 2
#define OBJECT_FLAG_TYPE1 3
#define OBJECT_FLAG_TYPE2 4

#define PTR_SIZE 16

const void *object_box_value_of(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	return $(obj);
}

unsigned int object_get_size(ObjectType type) {
	if (type == ObjectTypeInt)
		return 0;
	else if (type == ObjectTypeByte)
		return 0;
	else if (type == ObjectTypeBool)
		return 0;
	else if (type == ObjectTypeFloat)
		return 0;
	else if (type == ObjectTypeWeak)
		return sizeof(int64);
	else if (type == ObjectTypeBox)
		return sizeof(int64);

	return -1;
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
	if (type == ObjectTypeWeak) {
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
	} else if (type == ObjectTypeInt) {
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE0, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE1, false);
		object_set_ptr_flag(ptr, OBJECT_FLAG_TYPE2, false);
	}
}

Object object_create_impl(ObjectType type, const void *value, bool send) {

	unsigned int size = object_get_size(type);
	Ptr ret = fam_alloc(size, send);
	if (ret == NULL)
		return ret;

	if (size)
		memcpy($(ret), value, size);
	int64 *aux = ptr_aux(ret);
	object_set_ptr_type(ret, type);
	// set strong count to 1
	(*aux) |= 0x0000000000000001L;

	if (type == ObjectTypeInt || type == ObjectTypeFloat) {
		int v;
		memcpy(&v, value, 4);
		int64 count = ((unsigned long long)v << 24) & 0x00FFFFFFFFFFFFFFULL;
		*aux |= count;
	} else if (type == ObjectTypeBool || type == ObjectTypeByte) {
		int v = 0;
		memcpy(&v, value, 1);
		int64 count = ((int64)v << 24);
		*aux |= count;
	}

	return ret;
}

Object object_create(ObjectType type, const void *value, bool send) {
	if (value == NULL || type < 0 || type >= __ObjectTypeCount__ || type == ObjectTypeWeak ||
		type == ObjectTypeBox) {
		SetErr(IllegalArgument);
		return NULL;
	}

	return object_create_impl(type, value, send);
}

Object object_create_box(unsigned int size, bool send) {
	if (size == 0) {
		SetErr(IllegalArgument);
		return NULL;
	}
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
	int64 *aux = ptr_aux(ret);
	object_set_ptr_type(ret, ObjectTypeBox);

	// set strong count to 1
	(*aux) |= 0x0000000000000001L;

	return ret;
}

int object_mutate(Object obj, const void *value) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	ObjectType type = object_type(obj);
	if (type == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return -1;
	}

	unsigned int size = object_get_size(type);
	if (size)
		memcpy($(obj), value, size);
	return 0;
}

int object_resize(Object obj, unsigned int size) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	ObjectType type = object_type(obj);
	if (type != ObjectTypeBox) {
		SetErr(UnsupportedOperation);
		return -1;
	}

	Ptr ptr = NULL;
	object_value_of(obj, &ptr, 8);
	if (nil(ptr)) {
		SetErr(IllegalState);
		return -1;
	}

	Ptr updated = fam_resize(ptr, size);
	if (nil(updated)) {
		SetErr(AllocErr);
		return -1;
	}
	return object_mutate(obj, &updated);
}

int object_value_of(const Object obj, void *buffer, unsigned int limit) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	if (buffer == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	int max = limit;
	ObjectType type = object_type(obj);
	if (type == ObjectTypeInt || type == ObjectTypeFloat) {

		if (limit > 4)
			max = 4;
	} else if (type == ObjectTypeByte || type == ObjectTypeBool) {
		if (limit > 1)
			max = 1;
	} else if (type == ObjectTypeBox) {
		if (limit > 8)
			max = 8;
	}

	int ret = 0;
	if (type == ObjectTypeInt || type == ObjectTypeFloat || type == ObjectTypeByte ||
		type == ObjectTypeBool) {
		int64 *aux = ptr_aux(obj);
		ret = max;
		int count = ((*aux) & 0xFFFFFFFF000000LL) >> 24;
		memcpy((byte *)buffer, &count, ret);
	} else if (type == ObjectTypeBox) {
		ret = max;
		memcpy((byte *)buffer, $(obj), ret);
	}
	return ret;
}

ObjectType object_type(const Object obj) {
	if (nil(obj)) {
		SetErr(ObjectConsumed);
		return -1;
	}
	if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE0)) {
		if (object_get_ptr_flag(obj, OBJECT_FLAG_TYPE1)) {
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
			else
				return ObjectTypeInt;
		}
	}
	return ObjectTypeBool;
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
	ObjectType type = object_type(obj);
	if (send) {
		int64 *aux = ptr_aux(obj);
		int64 aux_val = __sync_fetch_and_sub(aux, 1);
		if ((aux_val & 0xFFFFFF) > 1) {
			// return strong count
			return aux_val - 1;
		} else {
			// return weak count
			if (type == ObjectTypeInt || type == ObjectTypeFloat || type == ObjectTypeByte ||
				type == ObjectTypeBool)
				return 0;
			else
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
			if (type == ObjectTypeInt || type == ObjectTypeFloat || type == ObjectTypeByte ||
				type == ObjectTypeBool)
				return 0;
			else
				return (*aux & 0xFFFFFF000000LL) >> 24;
		}
	}
}

int object_increment_strong(Object obj) {
	bool send = object_get_ptr_flag(obj, PTR_FLAGS_SEND);
	if (send) {
		int64 *aux = ptr_aux(obj);
		int64 old_count;
		int64 new_count;
		do {
			old_count = __sync_fetch_and_add(aux, 0); // Atomically load the current count
			new_count = old_count + 1;
			if ((new_count & 0xFFFFFF) == 0xFFFFFF) { // Check for overflow
				SetErr(Overflow);
				return -1;
			}
		} while (!__sync_val_compare_and_swap(aux, old_count, new_count));
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
		int64 *aux = ptr_aux(obj);
		int64 aux_val = __sync_fetch_and_sub(aux, 0x1 << 24);
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
			return ((*aux) & 0xFFFFFF);
		}
	}
}

int object_increment_weak(Object obj) {
	bool send = object_get_ptr_flag(obj, PTR_FLAGS_SEND);
	if (send) {
		int64 *aux = ptr_aux(obj);
		int64 old_count;
		int64 new_count;
		do {
			old_count = __sync_fetch_and_add(aux, 0); // Atomically load the current count
			new_count = old_count + (0x1 << 24);
			if ((new_count & 0xFFFFFF000000LL) == 0xFFFFFF000000LL) { // Check for overflow
				SetErr(Overflow);
				return -1;
			}
		} while (!__sync_val_compare_and_swap(aux, old_count, new_count));
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
	ObjectType type = object_type(src);
	if (type == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}

	bool send = object_get_ptr_flag(src, PTR_FLAGS_SEND);
	ObjectNc ret;
	if (type == ObjectTypeInt || type == ObjectTypeFloat || type == ObjectTypeByte ||
		type == ObjectTypeBool) {
		int64 *aux = ptr_aux(src);
		int value = ((*aux) & 0xFFFFFFFF000000LL) >> 24;
		ret = object_create_impl(type, &value, send);

	} else {
		ret = object_ref(src);
		// ret = object_create_impl(type, $(src), send);
	}
	Object_cleanup(&src);
	return ret;
}
Object object_ref(const Object src) {
	ObjectType type = object_type(src);
	if (type == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}
	return object_ref_impl(src, true);
}

Object object_weak(const Object src) {
	if (nil(src)) {
		SetErr(ObjectConsumed);
		return NULL;
	}
	ObjectType type = object_type(src);
	if (type == ObjectTypeInt || type == ObjectTypeFloat || type == ObjectTypeByte ||
		type == ObjectTypeBool || type == ObjectTypeWeak) {
		SetErr(IllegalArgument);
		return NULL;
	}
	unsigned long long v = (unsigned long long)src;
	bool send = object_get_ptr_flag(src, PTR_FLAGS_SEND);
	ObjectNc weak = object_create_impl(ObjectTypeWeak, &v, send);
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

	unsigned long long *target = object_box_value_of(src);
	ObjectNc w = (ObjectNc)*target;
	bool send = object_get_ptr_flag(src, PTR_FLAGS_SEND);
	if (send) {
		int64 *aux = ptr_aux(w);
		int64 old_count;
		do {
			old_count = __sync_fetch_and_add(aux, 0);
			if ((old_count & 0xFFFFFF) == 0) {
				return NULL; // Object is already deallocated
			}
		} while (!__sync_val_compare_and_swap(aux, old_count, old_count + 1));

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
	if (initialized(*obj)) {
		if (object_type(*obj) == ObjectTypeWeak) {
			unsigned long long *target = object_box_value_of(*obj);
			ObjectNc w = (ObjectNc)*target;

			int odwval = object_decrement_weak(w);
			if (!odwval) {
				// deallocate the pointer
				if (object_type(w) == ObjectTypeBox) {
					Ptr inner = *(Ptr *)object_box_value_of(w);
					fam_release(&inner);
				}
				fam_release(&w);
			}
			fam_release(obj);
		} else {
			int odsval = object_decrement_strong(*obj);
			if (!odsval) {
				// deallocate the pointer
				if (object_type(*obj) == ObjectTypeBox) {
					Ptr inner = *(Ptr *)object_box_value_of(*obj);
					fam_release(&inner);
				}
				fam_release(obj);
			}
		}
	}
}
