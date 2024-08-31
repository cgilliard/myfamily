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

#include <core/panic.h>
#include <core/type.h>
#include <stdlib.h>
#include <string.h>

_Thread_local const Object* __thread_local_self_Const = NULL;
_Thread_local Object* __thread_local_self_Var = NULL;

atomic_ullong __global_counter__;
void __attribute__((constructor)) init_global_counter()
{
	atomic_init(&__global_counter__, 1);
}

u64 unique_id()
{
	u64 ret = atomic_fetch_add(&__global_counter__, 1);
	return ret;
}

FatPtr build_fat_ptr(u64 size)
{
	FatPtr ret;
	if (chain_malloc(&ret, size))
	{
		// if chain_malloc is an error we set data to NULL and caller to
		// handle
		ret.data = NULL;
	}
	return ret;
}

int compare_vtable_entry(const void* ent1, const void* ent2)
{
	const VtableEntry* vtent1 = ent1;
	const VtableEntry* vtent2 = ent2;
	return strcmp(vtent1->name, vtent2->name);
}

void sort_vtable(Vtable* table)
{
	qsort(table->entries, table->len, sizeof(VtableEntry),
	      compare_vtable_entry);
}

void vtable_add_entry(Vtable* table, VtableEntry entry)
{
	if (table->entries == NULL)
	{
		table->entries = malloc(sizeof(VtableEntry) * (table->len + 1));
		if (table->entries == NULL)
			panic("Couldn't allocate memory for vtable");
	}
	else
	{
		void* tmp = realloc(table->entries,
				    sizeof(VtableEntry) * (table->len + 1));
		if (tmp == NULL)
			panic("Couldn't allocate memory for vtable");
		table->entries = tmp;
	}

	memcpy(&table->entries[table->len], &entry, sizeof(VtableEntry));
	table->len += 1;
	sort_vtable(table);
}

bool vtable_check_impl_trait(Vtable* table, char* trait)
{
	bool ret = false;

	for (u64 i = 0; i < table->trait_len; i++)
	{
		if (!strcmp(table->trait_entries[i].trait_name, trait))
		{
			ret = true;
			break;
		}
	}

	return ret;
}

void vtable_add_trait(Vtable* table, char* trait)
{
	if (table->trait_entries == NULL)
	{
		table->trait_entries =
		    malloc(sizeof(VtableTraitEntry) * (table->trait_len + 1));
		if (table->trait_entries == NULL)
			panic("Couldn't allocate memory for vtable");
	}
	else
	{
		void* tmp =
		    realloc(table->trait_entries,
			    sizeof(VtableTraitEntry) * (table->trait_len + 1));
		if (tmp == NULL)
			panic("Couldn't allocate memory for vtable");
		table->trait_entries = tmp;
	}

	VtableTraitEntry entry;
	if (strlen(trait) >= MAX_TRAIT_NAME_LEN)
		panic("trait name [%s] is too long. MAX_TRAIT_NAME_LEN = %i\n",
		      trait, MAX_TRAIT_NAME_LEN - 1);
	strcpy(entry.trait_name, trait);

	memcpy(&table->trait_entries[table->trait_len], &entry,
	       sizeof(VtableTraitEntry));
	table->trait_len += 1;
}

void* find_fn(const Object* obj, const char* name)
{
	int left = 0;
	int right = obj->vtable->len - 1;
	while (left <= right)
	{
		int mid = left + (right - left) / 2;
		int cmp = strcmp(name, obj->vtable->entries[mid].name);

		if (cmp == 0)
		{
			return obj->vtable->entries[mid].fn_ptr;
		}
		else if (cmp < 0)
		{
			right = mid - 1;
		}
		else
		{
			left = mid + 1;
		}
	}
	return NULL;
}

void SelfCleanupImpl_update(SelfCleanupImpl* ptr)
{
	__thread_local_self_Const = ptr->prev_tl_self_Const;
	__thread_local_self_Var = ptr->prev_tl_self_Var;
}

void Object_check_param(const Object* obj)
{
	if (obj && (obj->flags & OBJECT_FLAGS_CONSUMED) != 0)
		panic("Passing a consumed object as a function "
		      "parameter!");
}

void Object_build_int(Object* ptr)
{
	// call internal build handler
	void (*build_int)(Object* ptr) = find_fn(ptr, "build_internal");
	if (!build_int)
		panic("no internal build handler found");
	build_int(ptr);
}

void Object_build(Object* ptr)
{
	void (*do_build)(Object* ptr) = find_fn(ptr, "build");
	if (do_build)
	{
		// setup self references
		Object* tmp_Var = __thread_local_self_Var;
		const Object* tmp_Const = __thread_local_self_Const;
		__thread_local_self_Const = ptr;
		__thread_local_self_Var = ptr;
		do_build(ptr);
		// revert
		__thread_local_self_Var = tmp_Var;
		__thread_local_self_Const = tmp_Const;
	}
}

#if defined(__clang__)
// Clang-specific pragma
#pragma clang diagnostic ignored \
    "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__) && !defined(__clang__)
// GCC-specific pragma
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#warning "Unknown compiler or platform. No specific warning pragmas applied."
#endif
void Object_cleanup(const Object* ptr)
{
	Object* unconst = ptr;
	if ((unconst->flags & OBJECT_FLAGS_NO_CLEANUP) == 0)
	{
		// call internal drop handler
		void (*drop_int)(Object* ptr) = find_fn(ptr, "drop_internal");
		if (!drop_int)
			panic("no internal drop handler found");
		drop_int(unconst);

		// call defined drop handler
		void (*drop)(Object* ptr) = find_fn(ptr, "drop");
		if (drop)
		{
			// setup self references
			Object* tmp_Var = __thread_local_self_Var;
			Object* tmp_Const = __thread_local_self_Const;
			__thread_local_self_Const = unconst;
			__thread_local_self_Var = unconst;
			drop(ptr);
			// revert
			__thread_local_self_Var = tmp_Var;
			__thread_local_self_Const = tmp_Const;
		}
		if (fat_ptr_data(&unconst->ptr))
		{
			chain_free(&unconst->ptr);
		}
	}
}

#if defined(__clang__)
// Clang-specific pragma
#pragma GCC diagnostic push
#pragma clang diagnostic ignored \
    "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__) && !defined(__clang__)
// GCC-specific pragma
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#warning "Unknown compiler or platform. No specific warning pragmas applied."
#endif
void Object_mark_consumed(const Object* ptr)
{
	Object* unconst = ptr;
	unconst->flags |= OBJECT_FLAGS_NO_CLEANUP | OBJECT_FLAGS_CONSUMED;
}
#pragma GCC diagnostic pop
