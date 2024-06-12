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

#include <base/panic.h>
#include <base/tlmalloc.h>
#include <base/vtable.h>
#include <string.h>

#define CLASS(name, ...)                                                       \
	typedef struct {                                                       \
		Vtable *vtable;                                                \
		__VA_ARGS__                                                    \
	} name;                                                                \
	static VtableEntry *name##VtableEntries = NULL;                        \
	static u64 name##VtableEntriesSize = 0;

#define EXPAND(x) x
#define CATI(x, y) x##y
#define CAT(x, y) CATI(x, y)
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

#define TRAIT_COPY(T) TRAIT_FN(T, bool, copy, T *dst, T *src)
#define TRAIT_CLEANUP(T) TRAIT_FN(T, void, cleanup, T *obj)
#define TRAIT_SIZE(T) TRAIT_FN(T, size_t, size, T *obj)

#endif //_CLASS_BASE__
