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

#include <core/enum.h>

#define DEFINE_ENUM_VALUE_IMPL(sign, bits)                                     \
	sign##bits enum_value_##sign##bits(void *value) {                      \
		sign##bits r;                                                  \
		memcpy(&r, value, sizeof(sign##bits));                         \
		return r;                                                      \
	}

DEFINE_ENUM_VALUE_IMPL(u, size)
DEFINE_ENUM_VALUE_IMPL(i, size)
DEFINE_ENUM_VALUE_IMPL(u, 8)
DEFINE_ENUM_VALUE_IMPL(u, 16)
DEFINE_ENUM_VALUE_IMPL(u, 32)
DEFINE_ENUM_VALUE_IMPL(u, 64)
DEFINE_ENUM_VALUE_IMPL(u, 128)
DEFINE_ENUM_VALUE_IMPL(i, 8)
DEFINE_ENUM_VALUE_IMPL(i, 16)
DEFINE_ENUM_VALUE_IMPL(i, 32)
DEFINE_ENUM_VALUE_IMPL(i, 64)
DEFINE_ENUM_VALUE_IMPL(i, 128)
DEFINE_ENUM_VALUE_IMPL(f, 32)
DEFINE_ENUM_VALUE_IMPL(f, 64)

bool enum_value_bool(void *value) {
	bool r;
	memcpy(&r, value, sizeof(bool));
	return r;
}

#define DEFINE_BUILD_ENUM_VALUE_IMPL(sign, bits)                               \
	void *build_enum_value_##sign##bits(Slab *slab, sign##bits *v,         \
					    char *type_str) {                  \
		char type_compare[20];                                         \
		snprintf(type_compare, 20, "%s%s", #sign, #bits);              \
		if (strcmp(type_str, type_compare)) {                          \
			panic("Attempt to build an enum with the "             \
			      "wrong value. Expected [%s], Found [%s]",        \
			      type_str, type_compare);                         \
		}                                                              \
		if (mymalloc(slab, sizeof(sign##bits)))                        \
			return NULL;                                           \
		void *value = slab->data;                                      \
		memcpy(value, v, sizeof(sign##bits));                          \
		return value;                                                  \
	}

DEFINE_BUILD_ENUM_VALUE_IMPL(u, size)
DEFINE_BUILD_ENUM_VALUE_IMPL(i, size)
DEFINE_BUILD_ENUM_VALUE_IMPL(u, 128)
DEFINE_BUILD_ENUM_VALUE_IMPL(u, 64)
DEFINE_BUILD_ENUM_VALUE_IMPL(u, 32)
DEFINE_BUILD_ENUM_VALUE_IMPL(u, 16)
DEFINE_BUILD_ENUM_VALUE_IMPL(u, 8)
DEFINE_BUILD_ENUM_VALUE_IMPL(i, 128)
DEFINE_BUILD_ENUM_VALUE_IMPL(i, 64)
DEFINE_BUILD_ENUM_VALUE_IMPL(i, 32)
DEFINE_BUILD_ENUM_VALUE_IMPL(i, 16)
DEFINE_BUILD_ENUM_VALUE_IMPL(i, 8)
DEFINE_BUILD_ENUM_VALUE_IMPL(f, 64)
DEFINE_BUILD_ENUM_VALUE_IMPL(f, 32)

void *build_enum_value_bool(Slab *slab, bool *v, char *type_str) {
	if (strcmp(type_str, "bool")) {
		panic("Attempt to build an enum with the "
		      "wrong value. Expected [%s], Found [%s]",
		      type_str, "bool");
	}
	if (mymalloc(slab, sizeof(bool)))
		return NULL;
	void *value = slab->data;
	memcpy(value, v, sizeof(bool));
	return value;
}

void *build_enum_value(Slab *slab, void *v, char *type_str) {
	if (!strcmp(CLASS_NAME(v), "Rc") && strcmp(type_str, "Rc")) {
		// rc, check inner type instead
		Rc rc_myclone;
		myclone(&rc_myclone, v);
		Object *obj = unwrap(&rc_myclone);
		if (strcmp(CLASS_NAME(obj), type_str))
			panic("Attempt to build an enum with the "
			      "wrong value. Expected [%s], Found "
			      "[%s]",
			      type_str, CLASS_NAME(obj));
	} else if (strcmp(type_str, CLASS_NAME(v))) {
		panic("Attempt to build an enum with the "
		      "wrong value. Expected [%s], Found [%s]",
		      type_str, CLASS_NAME(v));
	}
	if (!implements(v, "copy")) {
		panic("copy is required. RC can "
		      "be used to wrap other types");
	}

	if (mymalloc(slab, size((Object *)v)))
		return NULL;
	void *value = slab->data;
	if (!copy(value, v)) {
		myfree(slab);
		return NULL;
	}
	return value;
}
