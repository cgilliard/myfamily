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

#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/macros.h>
#include <base/osdef.h>
#include <base/print_util.h>

SlabAllocator global_slab_allocator = NULL;

Ptr fam_ptr_for(unsigned int id, unsigned int len) {
	if (len > MAX_SLAB_SIZE) return NULL;
	return ptr_for(global_slab_allocator, id, len);
}

void __get_page_size__();
void __attribute__((constructor)) __init_sa() {
	if (global_slab_allocator == NULL) {
		// ensure page size set before we continue
		__get_page_size__();
		global_slab_allocator = slab_allocator_create();
		if (global_slab_allocator == NULL) {
			panic("Could not initialize global slab allocator!");
		}
	}
}

void __attribute__((destuctor)) __init_sa_tear_down() {
	if (global_slab_allocator) slab_allocator_cleanup(&global_slab_allocator);
	global_slab_allocator = NULL;
}

// use highest byte in aux for flags
void ptr_flag_set(Ptr ptr, byte flag, bool value) {
	int64 *aux = ptr_aux(ptr);
	if (value)
		*aux |= (0x1ULL << (unsigned long long)flag) << 56;
	else
		*aux &= ~((0x1ULL << (unsigned long long)flag) << 56);
}

bool ptr_flag_check(Ptr ptr, byte flag) {
	int64 *aux = ptr_aux(ptr);
	return (*aux) & ((0x1ULL << (unsigned long long)flag) << 56);
}

Ptr fam_alloc(unsigned int size) {
	// this size is reserved for 'null'
	if (size == UINT32_MAX) {
		SetErr(Overflow);
		return NULL;
	}
	Ptr ret;
	if (size > MAX_SLAB_SIZE)
		ret = ptr_direct_alloc(size);
	else {
		ret = slab_allocator_allocate(global_slab_allocator, size);
	}

	if (ret) {
		int64 *aux = ptr_aux(ret);
		*aux = 0;
		ptr_flag_set(ret, PTR_FLAGS_DIRECT, size > MAX_SLAB_SIZE);
	}

	return ret;
}
Ptr fam_resize(Ptr ptr, unsigned int size) {
	// this size is reserved for 'null'
	if (size == UINT32_MAX) {
		SetErr(Overflow);
		return NULL;
	}

	if (ptr == NULL) {
		SetErr(IllegalArgument);
		return NULL;
	}

	Ptr ret = fam_alloc(size);
	if (ret) {
		unsigned int len = $len(ptr);
		if (len > size) len = size;
		memcpy($(ret), $(ptr), len);
		fam_release(ptr);
	}

	return ret;
}

void fam_release(Ptr ptr) {
	if (nil(ptr) || ptr_len(ptr) == UINT32_MAX) {
		panic("fam_free on nil or special ptr!");
	} else if (ptr_flag_check(ptr, PTR_FLAGS_DIRECT)) {
		ptr_direct_release(ptr);
	} else {
		slab_allocator_free(global_slab_allocator, ptr);
	}
}

#ifdef TEST
void fam_alloc_cleanup() {
	if (global_slab_allocator) slab_allocator_cleanup(&global_slab_allocator);
	global_slab_allocator = NULL;
}
int64 fam_alloc_count_global_allocator() {
	return slab_allocator_cur_slabs_allocated(global_slab_allocator);
}
#endif	// TEST
