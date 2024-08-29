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

#define VDATA_FLAGS_NO_CLEANUP (0x1 << 0)

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct {
	char *name;
	u64 len;
	VtableEntry *entries;
} Vtable;

typedef struct Object {
	Vtable *vtable;
	u64 id;
	u8 flags;
	FatPtr ptr;
} Object;

typedef struct SelfCleanupImpl {
	const Object *prev_tl_self_Const;
	Object *prev_tl_self_Var;
} SelfCleanupImpl;

void SelfCleanupImpl_update(SelfCleanupImpl *sc);

#define SelfCleanup                                                            \
	SelfCleanupImpl __attribute__((warn_unused_result,                     \
				       cleanup(SelfCleanupImpl_update)))

extern _Thread_local const Object *__thread_local_self_Const;
extern _Thread_local Object *__thread_local_self_Var;

void Object_cleanup(const Object *ptr);
void Object_build(Object *ptr);
void sort_vtable(Vtable *table);
void vtable_add_entry(Vtable *table, VtableEntry entry);
void *find_fn(const Object *obj, const char *name);
u64 unique_id();
FatPtr build_fat_ptr(u64 size);

#define Cleanup                                                                \
	Object __attribute__((warn_unused_result, cleanup(Object_cleanup)))

#define TypeName(obj) obj.vtable->name

#define var Cleanup
#define let const Cleanup

#define $Var(field_name) ((IMPL *)__thread_local_self_Var->ptr.data)->field_name

#define $(field_name)                                                          \
	((const IMPL *)__thread_local_self_Const->ptr.data)->field_name

#define CALL_BOTH(x, y) BOTH y

#define Type(name, ...)                                                        \
	typedef struct name name;                                              \
	static Vtable name##_Vtable__ = {#name, 0, NULL};                      \
	u64 name##_size();                                                     \
	typedef struct name {                                                  \
		FOR_EACH(CALL_BOTH, , (;), __VA_ARGS__);                       \
	} name;                                                                \
	u64 name##_size() { return sizeof(name); }                             \
	static void __attribute__((constructor))                               \
	__add_impls_##name##_vtable() {                                        \
		VtableEntry size = {"size", name##_size};                      \
		vtable_add_entry(&name##_Vtable__, size);                      \
	}

#define Field(field_type, field_name) (field_type, field_name)
#define With(x, y) (x, y)

// Begin trait

// Define macros to extract the type and name from a tuple
#define EXTRACT_TYPE_NAME(type, name, ...) type name
#define EXTRACT_NAME(type, name, ...) name

#define UNWRAP_PARAM_TYPE_NAME(ignore, type_name) EXTRACT_TYPE_NAME type_name
#define UNWRAP_PARAM_NAME(ignore, type_name) EXTRACT_NAME type_name

// Define PROC_FN_SIGNATURE and PROC_PARAMS to use UNWRAP_PARAMS correctly
#define PROC_FN_SIGNATURE(...)                                                 \
	FOR_EACH(UNWRAP_PARAM_TYPE_NAME, , (, ), __VA_ARGS__)
#define PROC_PARAMS(...) FOR_EACH(UNWRAP_PARAM_NAME, , (, ), __VA_ARGS__)

#define Param(type) (type, CAT(name_, __COUNTER__))

#define RequiredOld(T, is_var, R, name, ...)                                   \
	R T##_##name(PROC_FN_SIGNATURE(__VA_ARGS__));                          \
	static void __attribute__((constructor))                               \
	CAT(__required_add_##T##_, UNIQUE_ID)() {                              \
		VtableEntry next = {#name, T##_##name};                        \
		vtable_add_entry(&T##_Vtable__, next);                         \
	}
#define Var()
#define Const() const

#define CALL_SECOND(x, y) SECOND y
#define PROCESS_FN_CALL(...)                                                   \
	FOR_EACH_INNER(CALL_SECOND, none, (, ), __VA_ARGS__)

#define PROCESS_FN_SIG(...) FOR_EACH_INNER(CALL_BOTH, none, (, ), __VA_ARGS__)

#define PROC_TRAIT_IMPL_FN(mutability, return_type, name, ...)                 \
	static return_type name(mutability() Object *self __VA_OPT__(, )       \
				    __VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__))) { \
		return_type (*impl)(__VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__))) = \
		    find_fn(self, #name);                                      \
		if (!impl)                                                     \
			panic("Runtime error: Trait bound violation! "         \
			      "Type "                                          \
			      "'%s' does "                                     \
			      "not implement the "                             \
			      "required function [%s]",                        \
			      TypeName((*self)), #name);                       \
		SelfCleanup sc = {__thread_local_self_Const,                   \
				  __thread_local_self_Var};                    \
		__thread_local_self_Const = self;                              \
		__thread_local_self_Var = NULL;                                \
		__thread_local_self_##mutability = self;                       \
		return impl(__VA_OPT__(PROCESS_FN_CALL(__VA_ARGS__)));         \
	}
#define PROC_TRAIT_IMPL(arg, x) PROC_TRAIT_IMPL_FN x
#define TraitImpl(trait) FOR_EACH(PROC_TRAIT_IMPL, none, (), trait)

/*
#define TraitImpl(trait, is_var, return_type, name, ...)                       \
	static return_type name(is_var() Object *self __VA_OPT__(, )           \
				    PROC_FN_SIGNATURE(__VA_ARGS__)) {          \
		return_type (*impl)(PROC_FN_SIGNATURE(__VA_ARGS__)) =          \
		    find_fn(self, #name);                                      \
		if (!impl)                                                     \
			panic("Runtime error: Trait bound violation! "         \
			      "Type "                                          \
			      "'%s' does "                                     \
			      "not implement the "                             \
			      "required function [%s]",                        \
			      TypeName((*self)));                              \
		SelfCleanup sc = {__thread_local_self_Const,                   \
				  __thread_local_self_Var};                    \
		__thread_local_self_Const = self;                              \
		__thread_local_self_Var = NULL;                                \
		__thread_local_self_##is_var = self;                           \
		return impl(PROC_PARAMS(__VA_ARGS__));                         \
	}
	*/

#define PROC_TRAIT_STATEMENT_IMPL_EXP(impl_type, mutability, return_type,      \
				      fn_name, ...)                            \
	return_type impl_type##_##fn_name(                                     \
	    __VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__)));                          \
	static void __attribute__((constructor))                               \
	__required_add__##impl_type##_##fn_name() {                            \
		VtableEntry next = {#fn_name, impl_type##_##fn_name};          \
		vtable_add_entry(&impl_type##_Vtable__, next);                 \
	}
#define PROC_TRAIT_STATEMENT_IMPL(...)                                         \
	PROC_TRAIT_STATEMENT_IMPL_EXP(__VA_ARGS__)
#define PROC_TRAIT_STATEMENT(arg, x)                                           \
	PROC_TRAIT_STATEMENT_IMPL(arg, EXPAND_ALL x)
// #define PROC_TRAIT_STATEMENT(arg, x) [ arg, EXPAND_ALL x ]
#define PROC_IMPL(name, ...)                                                   \
	FOR_EACH(PROC_TRAIT_STATEMENT, name, (), __VA_ARGS__)
#define DefineTrait(...) __VA_ARGS__
#define Required(...) (__VA_ARGS__)
#define Impl(name, trait) PROC_IMPL(name, trait)

#define SET_OFFSET_OF_IMPL(ptr, structure, name, value)                        \
	*((typeof(((structure *)0)->name) *)(ptr + offsetof(structure,         \
							    name))) = value;

#define SET_OFFSET_OF(...) SET_OFFSET_OF_IMPL(__VA_ARGS__)
#define SET_PARAM(ptr, value) SET_OFFSET_OF(EXPAND_ALL ptr, EXPAND_ALL value)

// TODO: _fptr__.data may be NULL, need to handle. Once we have
// 'Result' implement two versions of this macro. One which
// panics and one which returns error. Also, implement a trait
// 'validate' which is called here to validate the parameter
// settings.
// clang-format off
#define new(name, ...)({ \
		FatPtr _fptr__ = build_fat_ptr(name##_size());                 \
		FOR_EACH(SET_PARAM, (_fptr__.data, name), (), __VA_ARGS__)     \
		Object _ret__ = {&name##_Vtable__, unique_id(), 0, _fptr__};   \
		Object_build(&_ret__);                                         \
		_ret__;                                                        \
	})
// clang-format on

#endif // _CORE_TYPE__
