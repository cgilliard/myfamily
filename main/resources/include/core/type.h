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

#ifndef _CORE_TYPE__
#define _CORE_TYPE__

#include <assert.h>
#include <core/chain_allocator.h>
#include <core/heap.h>
#include <core/macro_utils.h>
#include <core/panic.h>
#include <core/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRAIT_NAME_LEN 128

#define OBJECT_FLAGS_NO_CLEANUP (0x1 << 0)
#define OBJECT_FLAGS_CONSUMED (0x1 << 1)
#define OBJECT_FLAG_HAS_MULTI_VARIANT (0x1 << 2)
#define OBJECT_FLAGS_RESERVED5 (0x1 << 3)
#define OBJECT_FLAGS_RESERVED4 (0x1 << 4)
#define OBJECT_FLAGS_RESERVED3 (0x1 << 5)
#define OBJECT_FLAGS_RESERVED2 (0x1 << 6)
#define OBJECT_FLAGS_RESERVED1 (0x1 << 7)

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct VtableTraitEntry {
	char trait_name[MAX_TRAIT_NAME_LEN + 1];

} VtableTraitEntry;

typedef struct {
	char *name;
	u64 len;
	VtableEntry *entries;
	u64 trait_len;
	VtableTraitEntry *trait_entries;
	bool is_trait_specifier;
} Vtable;

typedef struct Obj {
	Vtable *vtable;
	u8 flags;
	FatPtr ptr;
} Obj;

typedef struct SelfCleanupImpl {
	const Obj *prev_tl_self_Const;
	Obj *prev_tl_self_Var;
} SelfCleanupImpl;

void SelfCleanupImpl_update(SelfCleanupImpl *sc);

extern _Thread_local const Obj *__thread_local_self_Const;
extern _Thread_local Obj *__thread_local_self_Var;

void Obj_cleanup(const Obj *ptr);
void Obj_mark_consumed(const Obj *ptr);
void Obj_build(Obj *ptr, const void *config);
bool Obj_is_type(const Obj *ptr, char *ref);
void Obj_build_int(Obj *ptr);
void Obj_check_param(const Obj *obj);
void sort_vtable(Vtable *table);
void vtable_override(Vtable *table, VtableEntry entry);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void vtable_add_trait(Vtable *table, char *trait);
bool vtable_check_impl_trait(const Vtable *table, const char *trait);
void *find_fn(const Obj *obj, const char *name);
u64 unique_id();
FatPtr build_fat_ptr(u64 size);

#define SelfCleanup                                                                                \
	SelfCleanupImpl __attribute__((warn_unused_result, cleanup(SelfCleanupImpl_update)))

#define Cleanup Obj __attribute__((warn_unused_result, cleanup(Obj_cleanup)))

#define TypeName(obj) (obj).vtable->name

#endif // _CORE_TYPE__
