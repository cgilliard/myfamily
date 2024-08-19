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

#ifndef _CORE_CLASS__
#define _CORE_CLASS__

#include <assert.h>
#include <core/global_counter.h>
#include <core/macro_utils.h>
#include <core/panic.h>
#include <core/types.h>

typedef struct GenericsEntry {
	char *name;
	i64 *trait_ids;
	u64 trait_id_count;
} GenericsEntry;

typedef struct GenericClause {
	GenericsEntry *entries;
	u64 entry_count;
} GenericClause;

#define Module(name, ...)                                                      \
	void __attribute((constructor)) __setup_module##name() {               \
		i64 next = UNIQUE_ID();                                        \
		printf("++++++++++++setup mod [id=%lld] %s\n", next, #name);   \
	}                                                                      \
	__VA_ARGS__                                                            \
	void __attribute((constructor)) __exit_module##name() {                \
		printf("++++++++++++exit mod %s\n", #name);                    \
	}

#define Trait(id, ...)                                                         \
	void __attribute__((constructor)) __setup_trait_##id() {               \
		char *x = #__VA_ARGS__;                                        \
		printf("-------------->loaded trait[%s]='%s'\n", #id, x);      \
	}

#define Class(id, ...)                                                         \
	void __attribute__((constructor)) __setup_class_##id() {               \
		char *x = #__VA_ARGS__;                                        \
		printf("-------------->loaded class[%s]='%s'\n", #id, x);      \
	}

#define Use(id, v)                                                             \
	static void __attribute__((constructor)) __setup_function__##id() {    \
		char *x = #v;                                                  \
		printf("-------------->loaded use='%s',c=%d\n", x,             \
		       __COUNTER__);                                           \
	}

#define SaveFam(name, version, author, file)
#define LoadFam(fam)

#endif // _CORE_CLASS__
