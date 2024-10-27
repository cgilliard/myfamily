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

#include <base/os.h>
#include <base/slabs.h>

typedef struct PtrImpl {
	int64 id;	 // slab id
	int64 len;	 // length of data in this slab
	byte data[]; // user data
} PtrImpl;

//  returns the slab's length
int ptr_len(const Ptr ptr) {
	return ptr->len;
}

// returns the slabs id
int64 ptr_id(const Ptr ptr) {
	// mask off flag byte (top byte)
	return ptr->id & 0x00FFFFFFFFFFFFFFULL;
}

// reutrn pointer to slab data
void *ptr_data(const Ptr ptr) {
	return ptr->data;
}

// check whether a flag is set (0-7 are possible values)
bool ptr_flag_check(const Ptr ptr, byte flag) {
	return flag < 8 && ((int64)(ptr->id >> 56)) & (1ULL << flag);
}

// set/unset a flag
void ptr_flag_set(const Ptr ptr, byte flag, bool value) {
	if (flag >= 8)
		return;
	if (value)
		ptr->id |= (int64)(1ULL << (int64)flag) << 56;
	else
		ptr->id &= ~((int64)(1ULL << (int64)flag) << 56);
}

// return true if this Ptr is nil (unallocated)
bool ptr_is_nil(const Ptr ptr) {
	return ptr->len; // a nil pointer has 0 len
}

void slab_allocator_config_cleanup(SlabAllocatorConfigNc *ptr) {
}

typedef struct SlabAllocatorImpl {
	int x;
	int y;
	byte slabs[];
} SlabAllocatorImpl;

void slab_allocator_cleanup(SlabAllocator *ptr) {
	SlabAllocatorNc sa = *ptr;
}

SlabAllocator slab_allocator_create(SlabAllocatorConfig *sc) {
	SlabAllocatorNc ret = (SlabAllocatorImpl *)alloc(sizeof(SlabAllocatorImpl) + 3, false);
	return ret;
}

void slab_allocator_print(SlabAllocator ptr) {
}

#ifdef TEST
Ptr ptr_test_obj(int id, int len, byte flags) {
	Ptr ptr = (PtrImpl *)alloc(sizeof(PtrImpl) + len, false);
	ptr->id = id;
	ptr->len = len;
	ptr->id |= ((int64)flags) << 56;
	return ptr;
}

void ptr_free_test_obj(Ptr ptr) {
	release(ptr);
}
#endif // TEST
