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

#ifndef _CORE_CLASS2__
#define _CORE_CLASS2__

#include <core/macro_utils.h>
#include <core/panic.h>
#include <core/slabs.h>
#include <core/types.h>

#define VTABLE_FLAG_NO_CLEANUP 0x1

typedef struct VtableEntry {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct Vtable {
	u64 len;
	u128 id;
	VtableEntry *entries;
} Vtable;

typedef struct Vdata {
	Vtable *vtable;
	char *name;
	u64 id;
	u8 flags;
} Vdata;

typedef struct Object {
	Vdata vdata;
} Object;

typedef struct TraitRequired {
	char *return_type;
	char *name;
	char *parameters;
} TraitRequired;

typedef struct TraitEntry {
	char *name;
	u64 id;
	TraitRequired *required;
	u64 required_count;
} TraitEntry;

typedef struct TraitTable {
	u64 len;
	TraitEntry *entries;
} TraitTable;

Slab init_generic_slab(char **bindings[]);
void sort_global_trait_lookup_table();
void add_trait_entry(TraitEntry *ent);
static TraitTable __global_trait_lookup_table = {0, NULL};
static char *__current_trait_name = NULL;

#define V1_GENERIC(x, ...) #x
#define V2_GENERIC(x, ...) {__VA_ARGS__ __VA_OPT__(, )((void *)0)}

#define PROC_FIRST(x) V1_##x
#define PROC_OTHERS(x) V2_##x

#define TRAIT_REQUIRED(R, name, ...)                                           \
	static void __attribute__((                                            \
	    constructor)) __add_trait_required_##name##UNIQUE_ID() {           \
		printf("current=%s\n", __current_trait_name);                  \
		TraitEntry next = {__current_trait_name, UNIQUE_ID};           \
		add_trait_entry(&next);                                        \
	}

#define PROCESS_TRAIT_DIRECTIVE(x) x

#define TRAIT(trait_name, ...)                                                 \
	static inline void                                                     \
	    __attribute__((constructor)) __add_trait_##trait_name() {          \
		__global_trait_lookup_table.len += 1;                          \
		if (__global_trait_lookup_table.len == 1) {                    \
			void *tmp = malloc(sizeof(TraitEntry));                \
			if (tmp == NULL)                                       \
				panic("Could not allocate sufficient memory"); \
			__global_trait_lookup_table.entries = tmp;             \
		} else {                                                       \
			void *tmp =                                            \
			    realloc(__global_trait_lookup_table.entries,       \
				    sizeof(TraitEntry) *                       \
					__global_trait_lookup_table.len);      \
			if (tmp == NULL)                                       \
				panic("Could not allocate sufficient memory"); \
			__global_trait_lookup_table.entries = tmp;             \
		}                                                              \
		__global_trait_lookup_table                                    \
		    .entries[__global_trait_lookup_table.len - 1]              \
		    .name = #trait_name;                                       \
		__global_trait_lookup_table                                    \
		    .entries[__global_trait_lookup_table.len - 1]              \
		    .id = UNIQUE_ID;                                           \
		sort_global_trait_lookup_table();                              \
		__current_trait_name = #trait_name;                            \
	}                                                                      \
	FOR_EACH(PROCESS_TRAIT_DIRECTIVE, __VA_ARGS__);

#define CLASS(name, where, ...)                                                \
	typedef struct name##Ptr {                                             \
		Vdata vdata;                                                   \
		Slab generics;                                                 \
		__VA_ARGS__                                                    \
	} name##Ptr;                                                           \
	static Vtable name##Ptr_Vtable__ = {0, UNIQUE_ID, NULL};               \
	static inline void name##_cleanup_impl(name##Ptr *obj) {}              \
	static const char *__##name##_generic_names__[] = {                    \
	    FOR_EACH(PROC_FIRST, where)};                                      \
	static const u64 __##name##_generic_count__ =                          \
	    sizeof(__##name##_generic_names__) /                               \
	    sizeof(__##name##_generic_names__[0]);                             \
	static const char *__##name##_generic_bounds__                         \
	    [][1 + (sizeof(__##name##_generic_names__) /                       \
		    sizeof(__##name##_generic_names__[0]))] = {                \
		FOR_EACH(PROC_OTHERS, where)};                                 \
	static inline void                                                     \
	    __attribute__((constructor)) __init_class_##name() {               \
		printf("init class %s %i\n", #name,                            \
		       __##name##_generic_count__);                            \
	}

#define FIELD(x, y) x y;

#define WHERE(...) __VA_ARGS__
#define PROC_GEN(x) x
#define GENERIC_PARAM(p) Slab

#define DEFINE_CLASS(x)                                                        \
	x##Ptr __attribute__((warn_unused_result, cleanup(x##_cleanup_impl)))

#define BIND(G, V) {#G, #V, NULL}

#define BUILD_SLAB(name, ...)                                                  \
	({                                                                     \
		char *__vals[][4] = {FOR_EACH(EXPAND, __VA_ARGS__), NULL};     \
		Slab ret = init_generic_slab((char ***)__vals);                \
		ret;                                                           \
	})

#define BUILD(name, ...)                                                       \
	{{&name##Ptr_Vtable__, #name, UNIQUE_ID, 0},                           \
	 BUILD_SLAB(name, __VA_ARGS__)}

#endif // _CORE_CLASS2__

