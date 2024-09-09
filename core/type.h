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

typedef struct
{
	char* name;
	void* fn_ptr;
} VtableEntry;

typedef struct VtableTraitEntry
{
	char trait_name[MAX_TRAIT_NAME_LEN + 1];

} VtableTraitEntry;

typedef struct
{
	char* name;
	u64 len;
	VtableEntry* entries;
	u64 trait_len;
	VtableTraitEntry* trait_entries;
	bool is_trait_specifier;
} Vtable;

typedef struct Obj
{
	Vtable* vtable;
	u8 flags;
	FatPtr ptr;
} Obj;

typedef struct SelfCleanupImpl
{
	const Obj* prev_tl_self_Const;
	Obj* prev_tl_self_Var;
} SelfCleanupImpl;

void SelfCleanupImpl_update(SelfCleanupImpl* sc);

extern _Thread_local const Obj* __thread_local_self_Const;
extern _Thread_local Obj* __thread_local_self_Var;

void Obj_cleanup(const Obj* ptr);
void Obj_mark_consumed(const Obj* ptr);
void Obj_build(Obj* ptr, const void* config);
bool Obj_is_type(const Obj* ptr, char* ref);
void Obj_build_int(Obj* ptr);
void Obj_check_param(const Obj* obj);
void sort_vtable(Vtable* table);
void vtable_override(Vtable* table, VtableEntry entry);
void vtable_add_entry(Vtable* table, VtableEntry entry);
void vtable_add_trait(Vtable* table, char* trait);
bool vtable_check_impl_trait(const Vtable* table, const char* trait);
void* find_fn(const Obj* obj, const char* name);
u64 unique_id();
FatPtr build_fat_ptr(u64 size);

#define SelfCleanup                                        \
	SelfCleanupImpl __attribute__((warn_unused_result, \
				       cleanup(SelfCleanupImpl_update)))

#define Cleanup \
	Obj __attribute__((warn_unused_result, cleanup(Obj_cleanup)))

#define ReturnObj(obj) ({ Obj _ret__ = obj; Obj_mark_consumed(&obj); return _ret__; })
#define ReturnObjAndConsumeSelf(obj) ({ Obj _ret__ = obj; Obj_mark_consumed(&obj); Obj_cleanup($()); return _ret__; })

#define TypeName(obj) obj.vtable->name
#define Implements(obj, trait) ({                                              \
	bool _ret__ = false;                                                   \
	for (u64 _i__ = 0; _i__ < obj.vtable->trait_len; _i__++)               \
	{                                                                      \
		char* trait_name = obj.vtable->trait_entries[_i__].trait_name; \
		if (!strcmp(trait, trait_name))                                \
		{                                                              \
			_ret__ = true;                                         \
			break;                                                 \
		}                                                              \
	}                                                                      \
	_ret__;                                                                \
})

#define var Cleanup
#define let const Cleanup

#define $Var(...)                                                           \
	__VA_OPT__(((IMPL*)__thread_local_self_Var->ptr.data)->__VA_ARGS__) \
	__VA_OPT__(NONE)                                                    \
	(__thread_local_self_Var)

#define $(...)                                                               \
	__VA_OPT__(                                                          \
	    ((const IMPL*)__thread_local_self_Const->ptr.data)->__VA_ARGS__) \
	__VA_OPT__(NONE)                                                     \
	(__thread_local_self_Const)
#define $Context(value, type, ...)                         \
	__VA_OPT__(                                        \
	    ((const type*)(value)->ptr.data)->__VA_ARGS__) \
	__VA_OPT__(NONE)                                   \
	(value)

#define $ContextVar(value, type, ...)                \
	__VA_OPT__(                                  \
	    ((type*)(value)->ptr.data)->__VA_ARGS__) \
	__VA_OPT__(NONE)                             \
	(value)

#define FIRST_TWO(x, ...) MULTI_SWITCH(NONE, FIRST_TWO_, x, __VA_ARGS__)
#define FIRST_TWO_(x, y, ...) x y
#define CALL_FIRST_TWO(x, y) FIRST_TWO y

#define DROP_OBJECTS__(name, struct_type, inner, ...) \
	__VA_OPT__(Obj_cleanup(&((name*)(ptr->ptr.data))->inner);)
#define DROP_OBJECTS_(x, y, ...) __VA_OPT__(DROP_OBJECTS__(x, y, __VA_ARGS__))
#define DROP_OBJECTS_EXPAND(...) DROP_OBJECTS_(__VA_ARGS__)
#define DROP_OBJECTS(name, inner) DROP_OBJECTS_EXPAND(name, EXPAND_ALL inner)

#define PROCESS_WHERE_INNER__(...) \
	__VA_OPT__(FIRST_STRINGIFY __VA_ARGS__)
#define PROCESS_WHERE_INNER_(...) PROCESS_WHERE_INNER__(__VA_ARGS__)
#define PROCESS_WHERE_INNER(ignore, value) PROCESS_WHERE_INNER_(value)
#define PROCESS_WHERE_________(name, value, ...)                                                      \
	Vtable name##_##value##_Vtable__ = {#value, 0, NULL, 0, NULL, true};                          \
	void __attribute__((constructor)) __add_where_##name##_##value##_vtable()                     \
	{                                                                                             \
		char* arr[] = {__VA_OPT__(FOR_EACH_INNER(PROCESS_WHERE_INNER, , (, ), __VA_ARGS__))}; \
		int size = sizeof(arr) / sizeof(arr[0]);                                              \
		name##_generic_name_arr = malloc((1 + size) * sizeof(char*));                         \
		for (u64 i = 0; i < size; i++)                                                        \
		{                                                                                     \
			name##_generic_name_arr[i] = #value;                                          \
			vtable_add_trait(&name##_##value##_Vtable__, arr[i]);                         \
		}                                                                                     \
		name##_generic_name_arr[size] = NULL;                                                 \
	}
#define PROCESS_WHERE________(name, value, ...) PROCESS_WHERE_________(name, value, __VA_ARGS__)
#define PROCESS_WHERE_______(name, value, ...) PROCESS_WHERE________(name, EXPAND_ALL value)
#define PROCESS_WHERE______(name, ...) PROCESS_WHERE_______(name, __VA_ARGS__)
#define PROCESS_WHERE_____(value) PROCESS_WHERE______(EXPAND_ALL value)
#define PROCESS_WHERE____(...) __VA_OPT__(PROCESS_WHERE_____(__VA_ARGS__))
#define PROCESS_WHERE___(name, value, ...) PROCESS_WHERE____(__VA_OPT__(NONE)(name, value))
#define PROCESS_WHERE__(...) PROCESS_WHERE___(__VA_ARGS__)
#define PROCESS_WHERE_(name, value, ...) PROCESS_WHERE__(name, value)
#define PROCESS_WHERE(name, where) PROCESS_WHERE_(name, EXPAND_ALL where)

#define BUILD_OBJECTS_IMPL_(name, struct_type, inner, vtable_name, ...) __VA_OPT__(NONE)(BUILD_OBJECTS_IMPL__(name, struct_type, inner, vtable_name)) __VA_OPT__(BUILD_OBJECT_GENERIC(name, struct_type, inner, vtable_name))
#define BUILD_OBJECTS_IMPL(name, struct_type, inner, ...) BUILD_OBJECTS_IMPL_(name, struct_type, inner, __VA_ARGS__)
#define BUILD_OBJECTS_IMPL__(name, struct_type, inner, vtable_name) ({                              \
	((name*)(ptr->ptr.data))->inner = OBJECT_INIT;                                              \
	bool _is_generic__ = false;                                                                 \
	for (u64 i = 0; name##_generic_name_arr != NULL && name##_generic_name_arr[i] != NULL; i++) \
	{                                                                                           \
		if (!strcmp(name##_generic_name_arr[i], STRINGIFY(vtable_name)))                    \
		{                                                                                   \
			_is_generic__ = true;                                                       \
			break;                                                                      \
		}                                                                                   \
	}                                                                                           \
	((name*)(ptr->ptr.data))->inner.vtable = &vtable_name##_Vtable__;                           \
})
#define BUILD_OBJECT_GENERIC(name, struct_type, inner, vtable_name) ({                              \
	((name*)(ptr->ptr.data))->inner = OBJECT_INIT;                                              \
	bool _is_generic__ = false;                                                                 \
	for (u64 i = 0; name##_generic_name_arr != NULL && name##_generic_name_arr[i] != NULL; i++) \
	{                                                                                           \
		if (!strcmp(name##_generic_name_arr[i], STRINGIFY(vtable_name)))                    \
		{                                                                                   \
			_is_generic__ = true;                                                       \
			break;                                                                      \
		}                                                                                   \
	}                                                                                           \
	((name*)(ptr->ptr.data))->inner.vtable = &name##_##vtable_name##_Vtable__;                  \
})

#define BUILD_OBJECTS____(name, struct_type, inner, ...) __VA_OPT__(BUILD_OBJECTS_IMPL(name, struct_type, inner, __VA_ARGS__))
#define BUILD_OBJECTS__(name, v1, ...) __VA_OPT__(BUILD_OBJECTS____(name, v1, __VA_ARGS__))
#define BUILD_OBJECTS_(name, v1, ...) BUILD_OBJECTS__(name, v1 __VA_OPT__(, ) __VA_ARGS__)
#define BUILD_OBJECTS(name, inner) BUILD_OBJECTS_(name, EXPAND_ALL inner)

#define Where(generic, ...) ((generic, __VA_ARGS__))
#define TraitBound(t) (t)

#define Builder(name, ...)                                                \
	typedef struct name name;                                         \
	u64 name##_size();                                                \
	extern Vtable name##_Vtable__;                                    \
	typedef struct name##Config                                       \
	{                                                                 \
		__VA_OPT__(FOR_EACH(CALL_FIRST_TWO, , (;), __VA_ARGS__)); \
	} name##Config;

#define Type(name, ...) TypeImpl(name, , __VA_ARGS__)
#define StaticType(name, ...) TypeImpl(name, static, __VA_ARGS__)

#define TypeImpl(name, static_value, ...)                                                                   \
	typedef struct name name;                                                                           \
	static_value Vtable name##_Vtable__ = {#name, 0, NULL, 0, NULL, false};                             \
	static char** name##_generic_name_arr = NULL;                                                       \
	static_value u64 name##_size();                                                                     \
	typedef struct name                                                                                 \
	{                                                                                                   \
		EXPAND(EXPAND_ALL __VA_OPT__(PAREN) __VA_OPT__(PAREN_END) __VA_OPT__(NONE)(char _dummy__;)) \
		__VA_OPT__(FOR_EACH(CALL_FIRST_TWO, , (;), __VA_ARGS__);)                                   \
	} name;                                                                                             \
	static_value u64 name##_size() { return sizeof(name); }                                             \
	__VA_OPT__(FOR_EACH(PROCESS_WHERE, name, (;), __VA_ARGS__));                                        \
	static_value void name##_build_internal(Obj* ptr)                                                   \
	{                                                                                                   \
		u64 size = name##_size();                                                                   \
		memset(ptr->ptr.data, 0, size);                                                             \
		__VA_OPT__(FOR_EACH(BUILD_OBJECTS, name, (;), __VA_ARGS__));                                \
	}                                                                                                   \
	static_value void name##_drop_internal(Obj* ptr)                                                    \
	{                                                                                                   \
		__VA_OPT__(FOR_EACH(DROP_OBJECTS, name, (;), __VA_ARGS__));                                 \
	}                                                                                                   \
	static_value void __attribute__((constructor)) __add_impls_##name##_vtable()                        \
	{                                                                                                   \
		VtableEntry size = {"size", name##_size};                                                   \
		vtable_add_entry(&name##_Vtable__, size);                                                   \
		VtableEntry build_internal = {"build_internal",                                             \
					      name##_build_internal};                                       \
		vtable_add_entry(&name##_Vtable__, build_internal);                                         \
		VtableEntry drop_internal = {"drop_internal",                                               \
					     name##_drop_internal};                                         \
		vtable_add_entry(&name##_Vtable__, drop_internal);                                          \
	}

#define Field(field_type, field_name) (field_type, field_name)
#define Config(field_type, field_name) (field_type, field_name)
#define Object(obj_type, obj_name) (Obj, obj_name, obj_type)
#define Generic(obj_type, obj_name) (Obj, obj_name, obj_type, ignore)
#define With(x, y) (x, y, ignore)
#define WithObject(x, y) (x, y)

#define OBJECT_INIT                                                        \
	({                                                                 \
		FatPtr _fptr__ = FAT_PTR_INIT;                             \
		Obj _ret__ = {                                             \
		    NULL, OBJECT_FLAGS_NO_CLEANUP | OBJECT_FLAGS_CONSUMED, \
		    _fptr__};                                              \
		_ret__;                                                    \
	})

#define Move(dst, src)                                                                                                       \
	({                                                                                                                   \
		if (((*((Obj*)src)).flags & OBJECT_FLAGS_CONSUMED) != 0)                                                     \
			panic("src object has already been consumed\n");                                                     \
		if (((*((Obj*)dst)).vtable != NULL && ((*((Obj*)dst)).vtable != ((*((Obj*)src)).vtable))))                   \
		{                                                                                                            \
			/* This is a generic */                                                                              \
			if ((*((Obj*)dst)).vtable->is_trait_specifier)                                                       \
			{                                                                                                    \
				/* Check trait bounds */                                                                     \
				for (u64 i = 0; i < (*((Obj*)dst)).vtable->trait_len; i++)                                   \
				{                                                                                            \
					if (!Implements((*((Obj*)src)), (*((Obj*)dst)).vtable->trait_entries[i].trait_name)) \
						panic("required trait [%s] not implemented in type [%s].",                   \
						      (*((Obj*)dst)).vtable->trait_entries[i],                               \
						      (*((Obj*)src)).vtable->name);                                          \
				}                                                                                            \
			}                                                                                                    \
			/* This is a concrete type */                                                                        \
			else                                                                                                 \
				panic("Vtable mismatch! Trying to set type %s to "                                           \
				      "type %s!",                                                                            \
				      ((*((Obj*)dst)).vtable)->name, ((*((Obj*)src)).vtable)->name);                         \
		}                                                                                                            \
		if (((*((Obj*)dst)).flags & OBJECT_FLAGS_CONSUMED) == 0)                                                     \
			Obj_cleanup(dst);                                                                                    \
		/* We copy the flags and vtable from src and the Obj data (ptr).*/                                           \
		(*((Obj*)dst)).vtable = (*((Obj*)src)).vtable;                                                               \
		(*((Obj*)dst)).flags = (*((Obj*)src)).flags;                                                                 \
		(*((Obj*)dst)).ptr = (*((Obj*)src)).ptr;                                                                     \
		/* Set src's flags to consumed and no_cleanup as the ownership is moved. */                                  \
		(*((Obj*)src)).flags |= OBJECT_FLAGS_CONSUMED | OBJECT_FLAGS_NO_CLEANUP;                                     \
	})

// Define macros to extract the type and name from a tuple
#define EXTRACT_TYPE_NAME(type, name, ...) type name
#define EXTRACT_NAME(type, name, ...) name

#define UNWRAP_PARAM_TYPE_NAME(ignore, type_name) EXTRACT_TYPE_NAME type_name
#define UNWRAP_PARAM_NAME(ignore, type_name) EXTRACT_NAME type_name

// Define PROC_FN_SIGNATURE and PROC_PARAMS to use UNWRAP_PARAMS correctly
#define PROC_FN_SIGNATURE(...) \
	__VA_OPT__(FOR_EACH(UNWRAP_PARAM_TYPE_NAME, , (, ), __VA_ARGS__))
#define PROC_PARAMS(...) __VA_OPT__(FOR_EACH(UNWRAP_PARAM_NAME, , (, ), __VA_ARGS__))

#define Param(type) (type, CAT(name_, __COUNTER__))
#define SelfConfig() (const void*, config, ignore, ignore)
#define Self() (const Obj*, CAT(self_ptr_, UNIQUE_ID), ignore)
#define VarSelf() (Obj*, CAT(self_ptr_, UNIQUE_ID))

#define Var()
#define Const() const

#define SECOND_ALL(x, y, ...) y
#define CALL_SECOND(x, y) SECOND_ALL y
#define PROCESS_FN_CALL(...) \
	__VA_OPT__(FOR_EACH_INNER(CALL_SECOND, none, (, ), __VA_ARGS__))

#define OBJECT_ONLY(i, value, ...)                     \
	_Generic((value),                              \
	    Obj *: Obj_check_param((const Obj*)value), \
	    default: Obj_check_param(NULL))
#define CALL_SECOND_OBJECT(x, y) OBJECT_ONLY y
#define PROCESS_FN_CHECK_OBJECTS(...) \
	__VA_OPT__(FOR_EACH_INNER(CALL_SECOND_OBJECT, none, (;), __VA_ARGS__))

#define CALL_COUNT_OBJS(x, v) __count_obj_args__++
#define COUNT_OBJS(...) __VA_OPT__(FOR_EACH_INNER(CALL_COUNT_OBJS, none, (;), __VA_ARGS__))

#define BOTH_ALL(x, y, ...) x y
#define CALL_BOTH(x, y) BOTH_ALL y
#define PROCESS_FN_SIG(...) __VA_OPT__(FOR_EACH_INNER(CALL_BOTH, none, (, ), __VA_ARGS__))

#define EXPAND3(x) x
#define EXPAND2(x) x
#define PROC_BOTH_PROTO__(impl_type, param_type, param_name, ignore1, ...) __VA_OPT__(const impl_type##Config*) EXPAND3 __VA_OPT__(PAREN) __VA_OPT__(PAREN_END) __VA_OPT__(NONE)(const Obj*)
#define PROC_BOTH_PROTO_(impl_type, param_type, param_name, ...) __VA_OPT__(PROC_BOTH_PROTO__(impl_type, param_type, param_name, __VA_ARGS__)) EXPAND2 __VA_OPT__(NONE)(param_type param_name)
#define PROC_BOTH_PROTO(...) PROC_BOTH_PROTO_(__VA_ARGS__)
#define CALL_BOTH_PROTO(x, y) PROC_BOTH_PROTO(x, EXPAND_ALL y)
#define PROCESS_FN_SIG_PROTO(impl_type, ...) __VA_OPT__(FOR_EACH_INNER(CALL_BOTH_PROTO, impl_type, (, ), __VA_ARGS__))

#define EXPAND_ALL2(...) __VA_ARGS__
#define CHECK_SELF_IMPL___(self, type, var_name, ignore1, ...) \
	if (!Obj_is_type(var_name, self->vtable->name))        \
		panic("Type mismatch. Expected %s!", self->vtable->name);
#define CHECK_SELF_IMPL__(self, type, var_name, ...) __VA_OPT__(CHECK_SELF_IMPL___(self, type, var_name, __VA_ARGS__))
#define CHECK_SELF_IMPL_(...) CHECK_SELF_IMPL__(__VA_ARGS__)
#define CHECK_SELF_IMPL(self, param) CHECK_SELF_IMPL_(self, EXPAND_ALL2 param)
#define CHECK_SELF_PARAMS(self, ...) __VA_OPT__(FOR_EACH_INNER(CHECK_SELF_IMPL, self, (;), __VA_ARGS__))

// clang-format off
#define UNEXPECTED_FLAGS(f) if (      \
	f & OBJECT_FLAGS_RESERVED1 || \
	f & OBJECT_FLAGS_RESERVED2 || \
	f & OBJECT_FLAGS_RESERVED3 || \
	f & OBJECT_FLAGS_RESERVED4 || \
	f & OBJECT_FLAGS_RESERVED5)   \
		panic("Runtime error: Obj in unexpected state! Was it initialized?");
// clang-format on

#define PROC_TRAIT_IMPL_FN_(mutability, return_type, fn_name, ...)                      \
	static return_type fn_name(mutability() Obj* self __VA_OPT__(                   \
	    , ) __VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__)))                                \
	{                                                                               \
		UNEXPECTED_FLAGS(self->flags);                                          \
		if (self->flags & OBJECT_FLAGS_CONSUMED)                                \
		{                                                                       \
			if (self->vtable == NULL)                                       \
				panic("Runtime error: Obj has not been instantiated!"); \
			else                                                            \
				panic("Runtime error: Obj [%s@%" PRIu64                 \
				      "] has already been consumed!",                   \
				      self->vtable->name, self->ptr.id);                \
		}                                                                       \
		return_type (*impl)(__VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__))) =          \
		    find_fn(self, #fn_name);                                            \
		if (!impl)                                                              \
			panic("Runtime error: Trait bound violation! "                  \
			      "Type "                                                   \
			      "'%s' does "                                              \
			      "not implement the "                                      \
			      "required function [%s]",                                 \
			      TypeName((*self)), #fn_name);                             \
		SelfCleanup sc = {__thread_local_self_Const,                            \
				  __thread_local_self_Var};                             \
		__thread_local_self_Const = self;                                       \
		__thread_local_self_Var = NULL;                                         \
		__thread_local_self_##mutability = self;                                \
		CHECK_SELF_PARAMS(self, __VA_ARGS__);                                   \
		PROCESS_FN_CHECK_OBJECTS(__VA_ARGS__);                                  \
		return impl(__VA_OPT__(PROCESS_FN_CALL(__VA_ARGS__)));                  \
	}
#define DEFAULT_IMPL__(T, default_impl_fn, mutability, return_type, fn_name,  \
		       ...)                                                   \
	return_type default_impl_fn(__VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__))); \
	static void                                                           \
	    __attribute__((constructor)) add_##fn_name##_##T##_vtable()       \
	{                                                                     \
		VtableEntry next = {#fn_name, default_impl_fn};               \
		vtable_add_entry(&T##_Vtable__, next);                        \
	}

#define CHECK_SUPER_(type, trait)                                       \
	static void                                                     \
	    __attribute__((constructor)) check_super_##type##_##trait() \
	{                                                               \
		if (!vtable_check_impl_trait(&type##_Vtable__, #trait)) \
		{                                                       \
			panic("Runtime error: Required trait [%s] not " \
			      "implemented by type %s! Halting!",       \
			      #trait, #type);                           \
		}                                                       \
	}
#define CHECK_SUPER(...) CHECK_SUPER_(__VA_ARGS__)
#define FIRST_VAR_ARGS(x, ...) x
#define SUPER_PROC(arg, x) CHECK_SUPER(arg, FIRST_VAR_ARGS x)
#define DEFAULT_IMPL_(x, ...) \
	MULTI_SWITCH2_INNER(SUPER_PROC, DEFAULT_IMPL__, x, __VA_ARGS__)
#define DEFAULT_IMPL(T, ...) DEFAULT_IMPL_(T, EXPAND_ALL __VA_ARGS__)

#define PROC_TRAIT_IMPL_FN_DEFAULT__(default_fn, mutability, return_type,               \
				     fn_name, ...)                                      \
	static return_type fn_name(mutability() Obj* self __VA_OPT__(                   \
	    , ) __VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__)))                                \
	{                                                                               \
		UNEXPECTED_FLAGS(self->flags);                                          \
		if (self->flags & OBJECT_FLAGS_CONSUMED)                                \
		{                                                                       \
			if (self->vtable == NULL)                                       \
				panic("Runtime error: Obj has not been instantiated!"); \
			else                                                            \
				panic("Runtime error: Obj [%s@%" PRIu64                 \
				      "] has already been consumed!",                   \
				      self->vtable->name, self->ptr.id);                \
		}                                                                       \
		return_type (*impl)(__VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__))) =          \
		    find_fn(self, #fn_name);                                            \
		if (!impl)                                                              \
			panic("Runtime error: Trait bound violation! "                  \
			      "Type "                                                   \
			      "'%s' does "                                              \
			      "not implement the "                                      \
			      "required function [%s]",                                 \
			      TypeName((*self)), #fn_name);                             \
		SelfCleanup sc = {__thread_local_self_Const,                            \
				  __thread_local_self_Var};                             \
		__thread_local_self_Const = self;                                       \
		__thread_local_self_Var = NULL;                                         \
		__thread_local_self_##mutability = self;                                \
		CHECK_SELF_PARAMS(self, __VA_ARGS__);                                   \
		PROCESS_FN_CHECK_OBJECTS(__VA_ARGS__);                                  \
		return impl(__VA_OPT__(PROCESS_FN_CALL(__VA_ARGS__)));                  \
	}

#define SUPER_TRAIT_CONFIG(trait_name, ignore)
#define PROC_TRAIT_IMPL_FN_DEFAULT_(x, ...)                                \
	MULTI_SWITCH2(SUPER_TRAIT_CONFIG, PROC_TRAIT_IMPL_FN_DEFAULT__, x, \
		      __VA_ARGS__)
#define PROC_TRAIT_IMPL_FN_DEFAULT(...) \
	PROC_TRAIT_IMPL_FN_DEFAULT_(EXPAND_ALL __VA_ARGS__)

#define PROC_TRAIT_IMPL_FN(x, ...)                                       \
	MULTI_SWITCH(PROC_TRAIT_IMPL_FN_DEFAULT, PROC_TRAIT_IMPL_FN_, x, \
		     __VA_ARGS__)
#define PROC_TRAIT_IMPL(arg, x) PROC_TRAIT_IMPL_FN x
#define TraitImplImpl(name, ...)            \
	typedef struct __trait_impl__##name \
	{                                   \
		char dummy;                 \
	} __trait_impl__##name;             \
	__VA_OPT__(FOR_EACH(PROC_TRAIT_IMPL, none, (), __VA_ARGS__))
#define TraitImpl(...) TraitImplImpl(__VA_ARGS__)

#define PROC_TRAIT_STATEMENT_IMPL_EXP_(impl_type, mutability, return_type, \
				       fn_name, ...)                       \
	return_type impl_type##_##fn_name(                                 \
	    __VA_OPT__(PROCESS_FN_SIG_PROTO(impl_type, __VA_ARGS__)));     \
	static void __attribute__((                                        \
	    constructor)) __required_add__##impl_type##_##fn_name()        \
	{                                                                  \
		VtableEntry next = {#fn_name, impl_type##_##fn_name};      \
		vtable_add_entry(&impl_type##_Vtable__, next);             \
	}
#define PROC_TRAIT_STATEMENT_IMPL_EXP(x, y, ...)                          \
	MULTI_SWITCH2(DEFAULT_IMPL, PROC_TRAIT_STATEMENT_IMPL_EXP_, x, y, \
		      __VA_ARGS__)
#define PROC_TRAIT_STATEMENT_IMPL(...) \
	PROC_TRAIT_STATEMENT_IMPL_EXP(__VA_ARGS__)
#define PROC_TRAIT_STATEMENT(arg, x) \
	PROC_TRAIT_STATEMENT_IMPL(arg, EXPAND_ALL x)
#define PROC_IMPL(name, ...) \
	__VA_OPT__(FOR_EACH(PROC_TRAIT_STATEMENT, name, (), __VA_ARGS__))
#define Required(...) (__VA_ARGS__)
#define RequiredWithDefault(...) ((__VA_ARGS__))
#define PROC_IMPL_(name, trait_name, ...)                                                     \
	PROC_IMPL(name, __VA_ARGS__)                                                          \
	static void __attribute__((                                                           \
	    constructor)) vtable_add_trait_impl_##name##_##trait_name()                       \
	{                                                                                     \
		_Static_assert(sizeof(__trait_impl__##trait_name) != 0, "Trait not defined"); \
		vtable_add_trait(&name##_Vtable__, #trait_name);                              \
	}
#define Impl(name, ...) PROC_IMPL_(name, __VA_ARGS__)

#define Override(name, trait, implfn)                                    \
	static void                                                      \
	    __attribute__((constructor)) ov__##name##_##trait##_vtable() \
	{                                                                \
		VtableEntry next = {#trait, implfn};                     \
		vtable_override(&name##_Vtable__, next);                 \
	}

#define IS_OBJECT_TYPE(type) __builtin_types_compatible_p(typeof(type), Obj)

#define PROC_WITH_OBJ(ptr, structure, field_name, value)     \
	Obj* _ptr__ = ptr + offsetof(structure, field_name); \
	(*_ptr__) = OBJECT_INIT;                             \
	Move(_ptr__, &value);

#define SET_OFFSET_OF_IMPL(ptr, structure, name, value, ...)                                                 \
	__VA_OPT__(do {                                                                                      \
		if (IS_OBJECT_TYPE(((structure*)0)->name))                                                   \
		{                                                                                            \
			panic("Cannot use With to set an Obj. Perhaps, use a setter?");                      \
		}                                                                                            \
		else                                                                                         \
		{                                                                                            \
			*((typeof(((structure*)0)->name)*)((char*)ptr + offsetof(structure, name))) = value; \
		}                                                                                            \
	} while (0);)                                                                                        \
	EXPAND(EXPAND(EXPAND_ALL EXPAND(__VA_OPT__(NONE)(PROC_WITH_OBJ(ptr, structure, name, value)) __VA_OPT__(()))))

#define SET_OFFSET_OF(...) SET_OFFSET_OF_IMPL(__VA_ARGS__)
#define SET_PARAM(ptr, value) SET_OFFSET_OF(EXPAND_ALL ptr, EXPAND_ALL value)
#define DefineTrait(...) __VA_ARGS__
#define Super(x) (((x)))

// TODO: _fptr__.data may be NULL, need to handle. Once we have
// 'Result' implement two versions of this macro. One which
// panics and one which returns error. Also, implement a trait
// 'validate' which is called here to validate the parameter
// settings.
// clang-format off
#define new(name, ...)({                                                         \
		FatPtr _fptr__ = build_fat_ptr(name##_size());                   \
		Obj _ret__ = {&name##_Vtable__, 0, _fptr__ };       \
		Obj_build_int(&_ret__);                                       \
		name##Config __config_;                                          \
		memset(&__config_, 0, sizeof(name##Config));                     \
		__VA_OPT__(FOR_EACH(SET_PARAM, (&__config_, name##Config), (), __VA_ARGS__)) \
		Obj_build(&_ret__, &__config_);                               \
		_ret__;                                                          \
	})
// clang-format on

#endif // _CORE_TYPE__
