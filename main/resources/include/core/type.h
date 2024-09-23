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

#define TypeName(obj) (obj).vtable->name

#define Cleanup Obj __attribute__((warn_unused_result, cleanup(Obj_cleanup)))
#define mut Cleanup
#define let const Cleanup

#define SET_PARAM__(name, value)                                                                   \
	__config_.name = value;                                                                        \
	__config_.name##_is_set__ = true;
#define SET_PARAM_(...) SET_PARAM__(__VA_ARGS__)
#define SET_PARAM(ptr, value) SET_PARAM_(EXPAND_ALL value)
#define With(x, y) (x, y)

// TODO: _fptr__.data may be NULL, need to handle better. Once we have
// 'Result' implement two versions of this macro. One which
// panics and one which returns error.
#define _Impl(name, ...)                                                                           \
	({                                                                                             \
		FatPtr _fptr__ = build_fat_ptr(name##_size());                                             \
		if (_fptr__.data == NULL)                                                                  \
			panic("Could not allocate sufficient memory");                                         \
		Obj _ret__ = {&name##_Vtable__, 0, _fptr__};                                               \
		Obj_build_int(&_ret__);                                                                    \
		name##Config __config_;                                                                    \
		memset(&__config_, 0, sizeof(name##Config));                                               \
		__VA_OPT__(FOR_EACH(SET_PARAM, (&__config_, name##Config), (), __VA_ARGS__))               \
		Obj_build(&_ret__, &__config_);                                                            \
		_ret__;                                                                                    \
	})

#define _(name, ...) _Impl(name, __VA_ARGS__)

#define $Mut(...)                                                                                  \
	__VA_OPT__(((IMPL *)__thread_local_self_Var->ptr.data)->__VA_ARGS__)                           \
	__VA_OPT__(NONE)                                                                               \
	(__thread_local_self_Var)

#define $(...)                                                                                     \
	__VA_OPT__(((const IMPL *)__thread_local_self_Const->ptr.data)->__VA_ARGS__)                   \
	__VA_OPT__(NONE)                                                                               \
	(__thread_local_self_Const)

#define $Alloc(name, size)                                                                         \
	({                                                                                             \
		if (size <= 0) {                                                                           \
			$Free(name);                                                                           \
		} else {                                                                                   \
			if ($Mut(name##_fat_ptr__.data) == NULL) {                                             \
				if (chain_malloc(&$Mut(name##_fat_ptr__), size))                                   \
					panic("Could not allocate sufficient memory");                                 \
			} else {                                                                               \
				void *_tmp__;                                                                      \
				if (chain_realloc(&$Mut(name##_fat_ptr__), &$Mut(name##_fat_ptr__), size))         \
					panic("Could not allocate sufficient memory");                                 \
			}                                                                                      \
			$Mut(name) = $(name##_fat_ptr__).data;                                                 \
		}                                                                                          \
	})

#define $Free(name)                                                                                \
	({                                                                                             \
		if ($(name##_fat_ptr__).data) {                                                            \
			chain_free(&$Mut(name##_fat_ptr__));                                                   \
			$Mut(name##_fat_ptr__).data = NULL;                                                    \
			$Mut(name) = NULL;                                                                     \
		}                                                                                          \
	})

#define $Config(...) (((const IMPLCONFIG *)(__selfconfig__))->__VA_ARGS__)

#define $IsSet(...) (((const IMPLCONFIG *)(__selfconfig__))->__VA_ARGS__##_is_set__)

#define fn__(v, i) Fn_expand_##v##_return CAT(i, Fn_expand_##v##_params)
#define fn_(v) fn__(v, IMPL)
#define fn(v) fn_(v)

#define TYPE_EXPAND_(t) Type_Expand_##t##_
#define TYPE_EXPAND(t) TYPE_EXPAND_(t)
// this is to handle cases where -DIMPL is set to an empty string. We want to do nothing in those
// cases.
#define Type_Expand__

#endif // _CORE_TYPE__
