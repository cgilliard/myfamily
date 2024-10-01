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

#include <base/resources.h>
#include <stdio.h>
#include <util/slabs.h>

// Define both 64/32 bit impls the slab allocator will build the approproate types
// based on configuration.
typedef struct FatPtr64Impl {
	u64 id;
	u64 len;
	void *data;
} FatPtr64Impl;

typedef struct FatPtr32Impl {
	u32 id;
	u32 len;
	void *data;
} FatPtr32Impl;

bool is_little_endian() {
	u16 test = 0x1;
	return (*(u8 *)&test == 0x1);
}

// deteremine which kind of fat pointer this is. 32 bit fat pointers are restricted to having an ID
// that does not set the most significant bit. Therefore they must be less than 2^31 or the
// maximum signed int value (INT32_MAX). Conversely, the 64 bit ids must have this bit set.
// This is the slab allocator's responsibility to ensure the bit is set correctly.
// Users need not worry about these details as they are handled internally.
bool fat_ptr_is64(const FatPtr *ptr) {
	if (is_little_endian()) {
		return ((FatPtr32Impl *)ptr->data)->id & 0x1;
	} else {
		return ((FatPtr32Impl *)ptr->data)->id & 0x80000000;
	}
}

// Return the id of the FatPtr. First check bit for u64/u32 status, then return value.
u64 fat_ptr_id(const FatPtr *ptr) {
	if (fat_ptr_is64(ptr))
		return ((FatPtr64Impl *)ptr->data)->id;
	else
		return ((FatPtr32Impl *)ptr->data)->id;
}

// Get the length of this FatPtr. Once again use the status bit in the id then return value.
u64 fat_ptr_len(const FatPtr *ptr) {
	if (fat_ptr_is64(ptr))
		return ((FatPtr64Impl *)ptr->data)->len;
	else
		return ((FatPtr32Impl *)ptr->data)->len;
}

// Get the data pointer of this FatPtr. Once again use the status bit in the id then return value.
void *fat_ptr_data(const FatPtr *ptr) {
	if (fat_ptr_is64(ptr))
		return ((FatPtr64Impl *)ptr->data)->data;
	else
		return ((FatPtr32Impl *)ptr->data)->data;
}

// Test function to free the 64 bit objects.
void fat_ptr_free_test_obj64(FatPtr *ptr) {
	if (ptr && ptr->data) {
		myfree(ptr->data);
		ptr->data = NULL;
	}
}

// Test function to free the 32 bit objects.
void fat_ptr_free_test_obj32(FatPtr *ptr) {
	if (ptr && ptr->data) {
		myfree(ptr->data);
		ptr->data = NULL;
	}
}

// Test function to allocate the 64 bit objects.
void fat_ptr_test_obj64(FatPtr *ptr, u64 id, u64 len) {
	ptr->data = mymalloc(2 * sizeof(u64) + len);
	FatPtr64Impl *fptr = ptr->data;
	fptr->id = id;
	fptr->len = len;
	fptr->data = ptr->data + 2 * sizeof(u64);
}

// Test function to allocate the 32 bit objects.
void fat_ptr_test_obj32(FatPtr *ptr, u32 id, u32 len) {
	ptr->data = mymalloc(2 * sizeof(u32) + len);
	FatPtr32Impl *fptr = ptr->data;
	fptr->id = id;
	fptr->len = len;
	fptr->data = ptr->data + 2 * sizeof(u32);
}

// map a sequential id to the 64 bit version for our slab allocator
// slab allocators should use this function to map ids sequentially when configured to 64 bit
// 0 -> 1, 1 -> 3, 2 -> 5,, ... slab allocators may use up to 2^63. Beyond that is out of bounds.
// on big endian systems the MSB is set
u64 map_id_64(u64 seq_id) {
	if (is_little_endian()) {
		return (seq_id * 2) + 1;
	} else {
		return seq_id | 0x8000000000000000;
	}
}

// map a sequential id to the 32 bit version for our slab allocator
// slab allocators should use this function to map ids sequentially when configured to 32 bit
// 0 -> 0, 1 -> 2, 2 -> 4,, ... slab allocators may use up to 2^31. Beyond that is out of bounds.
// on big endian systems the MSB is unset
u32 map_id_32(u32 seq_id) {
	if (is_little_endian()) {
		return seq_id * 2;
	} else {
		return seq_id & 0x7FFFFFFF;
	}
}
