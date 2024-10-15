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

#ifndef _BASE_SLABS__
#define _BASE_SLABS__

#include <base/macro_utils.h>
#include <base/types.h>

// FatPtr flags
#define FAT_PTR_FLAG_GLOBAL 0
#define FAT_PTR_FLAG_MALLOC 1
#define FAT_PTR_FLAG_PIN 2
#define FAT_PTR_FLAG_USR1 3
#define FAT_PTR_FLAG_USR2 4
#define FAT_PTR_FLAG_USR3 5
#define FAT_PTR_FLAG_NIL 6
#define MAX_SLABS 4000000000

// FatPtr data type
typedef struct FatPtr {
	void *data;
} FatPtr;

int fat_ptr_mallocate(FatPtr *ptr, u64 size);
void fat_ptr_malloc_free(FatPtr *ptr);

// A slab allocator will always allocate these (exception mallocate), so the user only needs to
// be able to read the length and the data pointer.
u32 fat_ptr_size(const FatPtr *ptr);
void *fat_ptr_data(const FatPtr *ptr);
bool fat_ptr_flag_get(const FatPtr *ptr, u8 flag);
void fat_ptr_flag_set(const FatPtr *ptr, u8 flag, bool value);
bool fat_ptr_is_nil(const FatPtr *ptr);

// Slab Allocator

// Slab Type definition
typedef struct SlabType {
	u32 slab_size;
	u32 slabs_per_resize;
	u32 initial_chunks;
	u32 max_slabs;
} SlabType;

// Slab Allocator configuration
typedef struct SlabAllocatorConfigNc {
	bool zeroed;
	bool global;
	u32 slab_types_count;
	SlabType *slab_types;
} SlabAllocatorConfigNc;

void slab_allocator_config_cleanup(SlabAllocatorConfigNc *ptr);

#define SlabAllocatorConfig                                                                        \
	SlabAllocatorConfigNc                                                                          \
		__attribute__((warn_unused_result, cleanup(slab_allocator_config_cleanup)))

int slab_allocator_config_build(SlabAllocatorConfig *sc, bool zeroed, bool global);
int slab_allocator_config_add_type(SlabAllocatorConfig *sc, const SlabType *st);

typedef struct SlabAllocatorNc {
	void *impl;
} SlabAllocatorNc;

void slab_allocator_cleanup(SlabAllocatorNc *ptr);

#define SlabAllocator                                                                              \
	SlabAllocatorNc __attribute__((warn_unused_result, cleanup(slab_allocator_cleanup)))

int slab_allocator_build(SlabAllocator *ptr, const SlabAllocatorConfig *config);
int slab_allocator_allocate(SlabAllocator *ptr, u32 size, FatPtr *fptr);
void slab_allocator_free(SlabAllocator *ptr, FatPtr *fptr);
u64 slab_allocator_cur_slabs_allocated(const SlabAllocator *ptr);

typedef struct FatPtrMask {
	u8 v0;
	u8 v1;
	u8 v2;
	u8 v3;
	u8 v4;
	u8 v5;
	u8 v6;
	u8 v7;
	void *data;
} FatPtrMask;

#if defined(__clang__)
// Clang-specific pragma
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__) && !defined(__clang__)
// GCC-specific pragma
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#warning "Unknown compiler or platform. No specific warning pragmas applied."
#endif
static const FatPtrMask fat_ptr_not_null = {.data = &fat_ptr_not_null, .v7 = 0};
static const FatPtrMask fat_ptr_null = {.data = &fat_ptr_not_null, .v7 = UINT8_MAX};
#pragma GCC diagnostic pop

#define not_null                                                                                   \
	(const FatPtr) {                                                                               \
		.data = (void *)&fat_ptr_not_null                                                          \
	}
#define null                                                                                       \
	(const FatPtr) {                                                                               \
		.data = (void *)&fat_ptr_null                                                              \
	}
#define nil(v) (fat_ptr_flag_get(&v, FAT_PTR_FLAG_NIL))

#define $(v) (fat_ptr_data(&v))
#define $size(v) (fat_ptr_size(&v))
#define $flag(v, flag, ...)                                                                        \
	__VA_OPT__((fat_ptr_flag_set(&v, flag, __VA_ARGS__)))                                          \
	__VA_OPT__(NONE)(fat_ptr_flag_get(&v, flag))
#define $global(v) $flag(v, FAT_PTR_FLAG_GLOBAL)
#define $pin(v) $flag(v, FAT_PTR_FLAG_PIN, true)
#define $is_pin(v) $flag(v, FAT_PTR_FLAG_PIN)

// These are test helper functions
#ifdef TEST
u32 fat_ptr_id(const FatPtr *ptr);
void fat_ptr_test_obj32(FatPtr *ptr, u32 id, u32 len, bool global, bool pin);
void fat_ptr_free_test_obj32(FatPtr *ptr);
#endif // TEST

#endif // _BASE_SLABS__
