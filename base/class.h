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

#ifndef _CLASS_BASE__
#define _CLASS_BASE__

#include <base/cleanup.h>
#include <base/foreach.h>
#include <base/panic.h>
#include <base/tlmalloc.h>
#include <string.h>

#define UNIQUE_ID __COUNTER__

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct {
	u64 len;
	u128 id;
	VtableEntry *entries;
} Vtable;

typedef struct {
	Vtable *vtable;
	// reserved for other data as needed
} Vdata;

typedef struct {
	Vdata vdata;
} Object;

void *find_fn(Object *obj, const char *name);
void vtable_add_entry(Vtable *table, VtableEntry entry);

#define END_CLASS(name)                                                        \
	static Vtable name##Vtable = {0, UNIQUE_ID, NULL};                     \
	static void __init_vtable_##name() {                                   \
		if (name##Vtable.len == 0) {                                   \
			for (int i = 0; i < name##VtableEntriesSize; i++) {    \
				vtable_add_entry(&name##Vtable,                \
						 name##VtableEntries[i]);      \
			}                                                      \
		}                                                              \
	}                                                                      \
	void __attribute__((constructor)) __init_vtable_##name();

#define EXPAND(x) x
#define CATI(x, y) x##y
#define CAT(x, y) CATI(x, y)
#define DEFINE_CLASS(x) x##Ptr Cleanup(x##_cleanup)

#define BUILD(name, ...) {{&name##Vtable}, __VA_ARGS__}

#define CLASS_IMPL(name, ...)

// start FOR_EACH code

struct a {
	u128 a;
	u8 bx[3];
	int c[10];
};

#define PRN_STRUCT_OFFSETS_(structure, field)                                  \
	printf(STRINGIZE(structure)":" STRINGIZE(field)" - offset = %d\n",     \
						 offsetof(structure, field));
#define PRN_STRUCT_OFFSETS(field) PRN_STRUCT_OFFSETS_(struct a, field)
// end FOR_EACH code

#define EXTERNAL_GETTERS_SETTERS(x) ;

#define MEMBER_TYPE(type, member) typeof(((type *)0)->member)

#define FIELD(field_type, field_name) field_type CAT(_, field_name);
#define GETTER(name, field_name)                                               \
	static MEMBER_TYPE(name, CAT(_, field_name)) *                         \
	    CAT(name##_, CAT(get_, field_name))(name##Ptr * self) {            \
		return &self->CAT(_, field_name);                              \
	}
#define SETTER(name, field_name)                                               \
	static void CAT(name##_, CAT(set_, field_name))(                       \
	    name##Ptr * self,                                                  \
	    MEMBER_TYPE(name, CAT(_, field_name)) CAT(_, field_name)) {        \
		self->CAT(_, field_name) = CAT(_, field_name);                 \
	}

#define CLASS(name, ...)                                                       \
	typedef struct {                                                       \
		Vdata vdata;                                                   \
		__VA_ARGS__                                                    \
	} name##Ptr;                                                           \
	static VtableEntry *name##VtableEntries = NULL;                        \
	static u64 name##VtableEntriesSize = 0;                                \
	void name##_cleanup(name##Ptr *obj);                                   \
	static void                                                            \
	    __attribute__((constructor)) add_cleanup_##name##_vtable() {       \
		if (name##VtableEntries == NULL) {                             \
			name##VtableEntries = tlmalloc(sizeof(VtableEntry));   \
			if (!name##VtableEntries)                              \
				panic("couldn't allocate memory for vtable");  \
			name##VtableEntriesSize += 1;                          \
		} else {                                                       \
			name##VtableEntriesSize += 1;                          \
			name##VtableEntries = tlrealloc(                       \
			    name##VtableEntries,                               \
			    sizeof(VtableEntry) * name##VtableEntriesSize);    \
			if (!name##VtableEntries)                              \
				panic("couldn't allocate memory for vtable");  \
		}                                                              \
		char *str;                                                     \
		asprintf(&str, "cleanup");                                     \
		VtableEntry next = {str, name##_cleanup};                      \
		memcpy(&name##VtableEntries[name##VtableEntriesSize - 1],      \
		       &next, sizeof(VtableEntry));                            \
		name##VtableEntries[name##VtableEntriesSize - 1] = next;       \
	}

#define IMPL(name, trait) EXPAND(trait(name))

#define TRAIT_FN(T, R, name, ...)                                              \
	R T##_##name(__VA_ARGS__);                                             \
	static void __attribute__((constructor)) add_##name##_##T##_vtable() { \
		if (T##VtableEntries == NULL) {                                \
			T##VtableEntries = tlmalloc(sizeof(VtableEntry));      \
			if (!T##VtableEntries)                                 \
				panic("couldn't allocate memory for vtable");  \
			T##VtableEntriesSize += 1;                             \
		} else {                                                       \
			T##VtableEntriesSize += 1;                             \
			T##VtableEntries = tlrealloc(                          \
			    T##VtableEntries,                                  \
			    sizeof(VtableEntry) * T##VtableEntriesSize);       \
			if (!T##VtableEntries)                                 \
				panic("couldn't allocate memory for vtable");  \
		}                                                              \
		char *str;                                                     \
		asprintf(&str, "%s", #name);                                   \
		VtableEntry next = {str, T##_##name};                          \
		memcpy(&T##VtableEntries[T##VtableEntriesSize - 1], &next,     \
		       sizeof(VtableEntry));                                   \
		T##VtableEntries[T##VtableEntriesSize - 1] = next;             \
	}

#define TRAIT_COPY(T) TRAIT_FN(T, bool, copy, T##Ptr *dst, T##Ptr *src)
#define TRAIT_SIZE(T) TRAIT_FN(T, size_t, size, T##Ptr *obj)

#endif //_CLASS_BASE__
