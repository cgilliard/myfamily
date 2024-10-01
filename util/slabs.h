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

#ifndef _UTIL_SLABS__
#define _UTIL_SLABS__

#include <base/types.h>

// Opaque FatPtr data type
typedef struct FatPtr {
	void *data;
} FatPtr;

// A slab allocator will always allocate these, so the user only needs to
// be able to read the length and the data pointer.
u64 fat_ptr_len(const FatPtr *ptr);
void *fat_ptr_data(const FatPtr *ptr);

// These are test helper functions
#ifdef TEST
u64 fat_ptr_id(const FatPtr *ptr);
void fat_ptr_test_obj64(FatPtr *ptr, u64 id, u64 len);
void fat_ptr_free_test_obj64(FatPtr *ptr);
void fat_ptr_test_obj32(FatPtr *ptr, u32 id, u32 len);
void fat_ptr_free_test_obj32(FatPtr *ptr);
#endif // TEST

#endif // _UTIL_SLABS__
