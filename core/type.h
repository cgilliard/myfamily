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
#include <string.h>

#define MAX_TRAIT_NAME_LEN 128

#define OBJECT_FLAGS_NO_CLEANUP (0x1 << 0)
#define OBJECT_FLAGS_CONSUMED (0x1 << 1)

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
} Vtable;

typedef struct Object
{
	Vtable* vtable;
	u64 id;
	u8 flags;
	FatPtr ptr;
} Object;

typedef struct SelfCleanupImpl
{
	const Object* prev_tl_self_Const;
	Object* prev_tl_self_Var;
} SelfCleanupImpl;

void SelfCleanupImpl_update(SelfCleanupImpl* sc);

#define SelfCleanup                                        \
	SelfCleanupImpl __attribute__((warn_unused_result, \
				       cleanup(SelfCleanupImpl_update)))

extern _Thread_local const Object* __thread_local_self_Const;
extern _Thread_local Object* __thread_local_self_Var;

void Object_cleanup(const Object* ptr);
void Object_mark_consumed(const Object* ptr);
void Object_build(Object* ptr, void* config);
void Object_build_int(Object* ptr);
void Object_check_param(const Object* obj);
void sort_vtable(Vtable* table);
void vtable_override(Vtable* table, VtableEntry entry);
void vtable_add_entry(Vtable* table, VtableEntry entry);
void vtable_add_trait(Vtable* table, char* trait);
bool vtable_check_impl_trait(Vtable* table, char* trait);
void* find_fn(const Object* obj, const char* name);
u64 unique_id();
FatPtr build_fat_ptr(u64 size);

#define Cleanup \
	Object __attribute__((warn_unused_result, cleanup(Object_cleanup)))

#define TypeName(obj) obj.vtable->name

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

#define FIRST_TWO(x, y, ...) x y
#define CALL_FIRST_TWO(x, y) FIRST_TWO y

#define DROP_OBJECTS__(name, struct_type, inner, ...) \
	__VA_OPT__(Object_cleanup(&((name*)(ptr->ptr.data))->inner);)
#define DROP_OBJECTS_(...) DROP_OBJECTS__(__VA_ARGS__)
#define DROP_OBJECTS(name, inner) DROP_OBJECTS_(name, EXPAND_ALL inner)

#define BUILD_OBJECTS__(name, struct_type, inner, ...)            \
	__VA_OPT__(((name*)(ptr->ptr.data))->inner = OBJECT_INIT; \
		   ((name*)(ptr->ptr.data))->inner.vtable =       \
		       &__VA_ARGS__##_Vtable__);

#define BUILD_OBJECTS_(...) BUILD_OBJECTS__(__VA_ARGS__)
#define BUILD_OBJECTS(name, inner) BUILD_OBJECTS_(name, EXPAND_ALL inner)

#define TypeDef(name, ...)                                    \
	typedef struct name name;                             \
	u64 name##_size();                                    \
	extern Vtable name##_Vtable__;                        \
	typedef struct name##Config                           \
	{                                                     \
		FOR_EACH(CALL_FIRST_TWO, , (;), __VA_ARGS__); \
	} name##Config;

#define Type(name, ...)                                                 \
	typedef struct name name;                                       \
	Vtable name##_Vtable__ = {#name, 0, NULL, 0, NULL};             \
	u64 name##_size();                                              \
	typedef struct name                                             \
	{                                                               \
		FOR_EACH(CALL_FIRST_TWO, , (;), __VA_ARGS__);           \
	} name;                                                         \
	u64 name##_size() { return sizeof(name); }                      \
	void name##_build_internal(Object* ptr)                         \
	{                                                               \
		u64 size = name##_size();                               \
		memset(ptr->ptr.data, 0, size);                         \
		FOR_EACH(BUILD_OBJECTS, name, (;), __VA_ARGS__);        \
	}                                                               \
	void name##_drop_internal(Object* ptr)                          \
	{                                                               \
		FOR_EACH(DROP_OBJECTS, name, (;), __VA_ARGS__);         \
	}                                                               \
	void __attribute__((constructor)) __add_impls_##name##_vtable() \
	{                                                               \
		VtableEntry size = {"size", name##_size};               \
		vtable_add_entry(&name##_Vtable__, size);               \
		VtableEntry build_internal = {"build_internal",         \
					      name##_build_internal};   \
		vtable_add_entry(&name##_Vtable__, build_internal);     \
		VtableEntry drop_internal = {"drop_internal",           \
					     name##_drop_internal};     \
		vtable_add_entry(&name##_Vtable__, drop_internal);      \
	}

#define Field(field_type, field_name) (field_type, field_name)
#define Config(field_type, field_name) (field_type, field_name)
#define Obj(obj_type, obj_name) (Object, obj_name, obj_type)
#define With(x, y) (x, y, ignore)
#define WithObj(x, y) (x, y)

#define OBJECT_INIT                                                           \
	({                                                                    \
		FatPtr _fptr__;                                               \
		_fptr__.data = NULL;                                          \
		_fptr__.len = 0;                                              \
		Object _ret__ = {                                             \
		    NULL, 0, OBJECT_FLAGS_NO_CLEANUP | OBJECT_FLAGS_CONSUMED, \
		    _fptr__};                                                 \
		_ret__;                                                       \
	})

#define Move(dst, src)                                                                                              \
	({                                                                                                          \
		if (((*((Object*)src)).flags & OBJECT_FLAGS_CONSUMED) != 0)                                         \
			panic("src object has already been consumed\n");                                            \
		/* Check for vtable mismatch */                                                                     \
		if (((*((Object*)dst)).vtable != NULL && ((*((Object*)dst)).vtable != ((*((Object*)src)).vtable)))) \
		{                                                                                                   \
			panic("Vtable mismatch! Trying to set type %s to "                                          \
			      "type %s!",                                                                           \
			      ((*((Object*)dst)).vtable)->name,                                                     \
			      ((*((Object*)src)).vtable)->name);                                                    \
		}                                                                                                   \
		/* If the object is not consumed we call cleanup as we are                                          \
		 * overwriting it.*/                                                                                \
		if (((*((Object*)dst)).flags & OBJECT_FLAGS_CONSUMED) == 0)                                         \
			Object_cleanup(dst);                                                                        \
		*((Object*)dst) = *((Object*)src);                                                                  \
		(*((Object*)src)).flags |=                                                                          \
		    OBJECT_FLAGS_CONSUMED | OBJECT_FLAGS_NO_CLEANUP;                                                \
	})

// Define macros to extract the type and name from a tuple
#define EXTRACT_TYPE_NAME(type, name, ...) type name
#define EXTRACT_NAME(type, name, ...) name

#define UNWRAP_PARAM_TYPE_NAME(ignore, type_name) EXTRACT_TYPE_NAME type_name
#define UNWRAP_PARAM_NAME(ignore, type_name) EXTRACT_NAME type_name

// Define PROC_FN_SIGNATURE and PROC_PARAMS to use UNWRAP_PARAMS correctly
#define PROC_FN_SIGNATURE(...) \
	FOR_EACH(UNWRAP_PARAM_TYPE_NAME, , (, ), __VA_ARGS__)
#define PROC_PARAMS(...) FOR_EACH(UNWRAP_PARAM_NAME, , (, ), __VA_ARGS__)

#define Param(type) (type, CAT(name_, __COUNTER__))

#define Var()
#define Const() const

#define CALL_SECOND(x, y) SECOND y
#define PROCESS_FN_CALL(...) \
	FOR_EACH_INNER(CALL_SECOND, none, (, ), __VA_ARGS__)

#define OBJECT_ONLY(i, value)                                   \
	_Generic((value),                                       \
	    Object *: Object_check_param((const Object*)value), \
	    default: Object_check_param(NULL))
#define CALL_SECOND_OBJECT(x, y) OBJECT_ONLY y
#define PROCESS_FN_CHECK_OBJECTS(...) \
	FOR_EACH_INNER(CALL_SECOND_OBJECT, none, (;), __VA_ARGS__)

#define CALL_COUNT_OBJS(x, v) __count_obj_args__++
#define COUNT_OBJS(...) FOR_EACH_INNER(CALL_COUNT_OBJS, none, (;), __VA_ARGS__)

#define CALL_BOTH(x, y) BOTH y
#define PROCESS_FN_SIG(...) FOR_EACH_INNER(CALL_BOTH, none, (, ), __VA_ARGS__)

#define PROC_TRAIT_IMPL_FN_(mutability, return_type, fn_name, ...)             \
	static return_type fn_name(mutability() Object* self __VA_OPT__(       \
	    , ) __VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__)))                       \
	{                                                                      \
		if (self->flags & OBJECT_FLAGS_CONSUMED)                       \
			panic("Runtime error: Object [%s@%" PRIu64             \
			      "] has already been consumed!",                  \
			      self->vtable->name, self->ptr.id);               \
		return_type (*impl)(__VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__))) = \
		    find_fn(self, #fn_name);                                   \
		if (!impl)                                                     \
			panic("Runtime error: Trait bound violation! "         \
			      "Type "                                          \
			      "'%s' does "                                     \
			      "not implement the "                             \
			      "required function [%s]",                        \
			      TypeName((*self)), #fn_name);                    \
		SelfCleanup sc = {__thread_local_self_Const,                   \
				  __thread_local_self_Var};                    \
		__thread_local_self_Const = self;                              \
		__thread_local_self_Var = NULL;                                \
		__thread_local_self_##mutability = self;                       \
		PROCESS_FN_CHECK_OBJECTS(__VA_ARGS__);                         \
		return impl(__VA_OPT__(PROCESS_FN_CALL(__VA_ARGS__)));         \
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

#define PROC_TRAIT_IMPL_FN_DEFAULT__(default_fn, mutability, return_type,      \
				     fn_name, ...)                             \
	static return_type fn_name(mutability() Object* self __VA_OPT__(       \
	    , ) __VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__)))                       \
	{                                                                      \
		if (self->flags & OBJECT_FLAGS_CONSUMED)                       \
			panic("Runtime error: Object [%s@%" PRIu64             \
			      "] has already been consumed!",                  \
			      self->vtable->name, self->id);                   \
		return_type (*impl)(__VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__))) = \
		    find_fn(self, #fn_name);                                   \
		if (!impl)                                                     \
			panic("Runtime error: Trait bound violation! "         \
			      "Type "                                          \
			      "'%s' does "                                     \
			      "not implement the "                             \
			      "required function [%s]",                        \
			      TypeName((*self)), #fn_name);                    \
		SelfCleanup sc = {__thread_local_self_Const,                   \
				  __thread_local_self_Var};                    \
		__thread_local_self_Const = self;                              \
		__thread_local_self_Var = NULL;                                \
		__thread_local_self_##mutability = self;                       \
		PROCESS_FN_CHECK_OBJECTS(__VA_ARGS__);                         \
		return impl(__VA_OPT__(PROCESS_FN_CALL(__VA_ARGS__)));         \
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
#define TraitImplImpl(name, ...) \
	FOR_EACH(PROC_TRAIT_IMPL, none, (), __VA_ARGS__)
#define TraitImpl(...) TraitImplImpl(__VA_ARGS__)

#define PROC_TRAIT_STATEMENT_IMPL_EXP_(impl_type, mutability, return_type, \
				       fn_name, ...)                       \
	return_type impl_type##_##fn_name(                                 \
	    __VA_OPT__(PROCESS_FN_SIG(__VA_ARGS__)));                      \
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
	FOR_EACH(PROC_TRAIT_STATEMENT, name, (), __VA_ARGS__)
#define Required(...) (__VA_ARGS__)
#define RequiredWithDefault(...) ((__VA_ARGS__))
#define PROC_IMPL_(name, trait_name, ...)                               \
	PROC_IMPL(name, __VA_ARGS__)                                    \
	static void __attribute__((                                     \
	    constructor)) vtable_add_trait_impl_##name##_##trait_name() \
	{                                                               \
		vtable_add_trait(&name##_Vtable__, #trait_name);        \
	}
#define Impl(name, ...) PROC_IMPL_(name, __VA_ARGS__)

#define Override(name, trait, implfn)                                    \
	static void                                                      \
	    __attribute__((constructor)) ov__##name##_##trait##_vtable() \
	{                                                                \
		VtableEntry next = {#trait, implfn};                     \
		vtable_override(&name##_Vtable__, next);                 \
	}

#define IS_OBJECT_TYPE(type) __builtin_types_compatible_p(typeof(type), Object)

#define PROC_WITH_OBJ(ptr, structure, field_name, value)        \
	Object* _ptr__ = ptr + offsetof(structure, field_name); \
	(*_ptr__) = OBJECT_INIT;                                \
	Move(_ptr__, &value);

#define SET_OFFSET_OF_IMPL(ptr, structure, name, value, ...)                                                 \
	__VA_OPT__(do {                                                                                      \
		if (IS_OBJECT_TYPE(((structure*)0)->name))                                                   \
		{                                                                                            \
			panic("Cannot use With to set an Object. Use "                                       \
			      "'WithObj' isntead.\n");                                                       \
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
		Object _ret__ = {&name##_Vtable__, unique_id(), 0, _fptr__};     \
		Object_build_int(&_ret__);                                       \
		name##Config __config_;                                          \
		memset(&__config_, 0, sizeof(name##Config));                     \
		FOR_EACH(SET_PARAM, (&__config_, name##Config), (), __VA_ARGS__) \
		Object_build(&_ret__, &__config_);                               \
		_ret__;                                                          \
	})
// clang-format on

#endif // _CORE_TYPE__
