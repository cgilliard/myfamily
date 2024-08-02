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

#ifndef _CORE_SLABS__
#define _CORE_SLABS__

#include <core/class.h>
#include <core/enum.h>
#include <core/slice.h>
#include <core/traits.h>
#include <core/unit.h>

CLASS(SlabSizeCount,
      FIELD(u64, slab_size) FIELD(u64, slab_count) FIELD(u64, max_slabs))
IMPL(SlabSizeCount, TRAIT_COPY)
#define SlabSizeCount DEFINE_CLASS(SlabSizeCount)

ENUM(SlabAllocatorOption,
     VARIANTS(SLAB_OPTION_SIZE_COUNT, SLAB_OPTION_ZEROED,
	      SLAB_OPTION_SLABS_PER_RESIZE),
     TYPES("SlabSizeCount", "bool", "u64"))
#define SlabAllocatorOption DEFINE_ENUM(SlabAllocatorOption)

#define TRAIT_SLAB_DATA(T)                                                     \
	TRAIT_REQUIRED(T, Result, init, T##Ptr *ptr, u64 initial_slabs,        \
		       u64 slab_size, u64 max_slabs)                           \
	TRAIT_REQUIRED(T, Result, read, T##Ptr *ptr, Slice *slice, u64 offset) \
	TRAIT_REQUIRED(T, Result, reference, T##Ptr *ptr, Slice *slice,        \
		       u64 offset)                                             \
	TRAIT_REQUIRED(T, Result, write, T##Ptr *ptr, Slice *slice,            \
		       u64 offset)                                             \
	TRAIT_REQUIRED(T, Result, resize, T##Ptr *ptr, u64 slabs)

#define TRAIT_SLAB_ALLOCATOR(T)                                                \
	TRAIT_REQUIRED(T, Result, build, int num, ...)                         \
	TRAIT_REQUIRED(T, Result, allocate, T##Ptr *ptr, u64 size)             \
	TRAIT_REQUIRED(T, Result, get, T##Ptr *ptr, Slice *slice, u64 id)      \
	TRAIT_REQUIRED(T, Result, free, T##Ptr *ptr, u64 id)

typedef struct SlabDataParams {
	u64 slab_size;
	u64 slab_count;
	u64 max_slabs;
	u64 free_list_head;
} SlabDataParams;

CLASS(SlabData, FIELD(void *, data) FIELD(SlabDataParams, sdp))
IMPL(SlabData, TRAIT_SLAB_DATA)
IMPL(SlabData, TRAIT_CMP)
#define SlabData DEFINE_CLASS(SlabData)

CLASS(SlabAllocator,
      FIELD(SlabData *, slab_data_arr) FIELD(u64, slab_data_arr_size)
	  FIELD(bool, zeroed) FIELD(u64, slabs_per_resize))
IMPL(SlabAllocator, TRAIT_SLAB_ALLOCATOR)
#define SlabAllocator DEFINE_CLASS(SlabAllocator)

#define SlabDataConfig(size, count, max)                                       \
	({                                                                     \
		SlabSizeCount __v9_ = BUILD(SlabSizeCount, size, count, max);  \
		SlabAllocatorOptionPtr __opt9_ = BUILD_ENUM(                   \
		    SlabAllocatorOption, SLAB_OPTION_SIZE_COUNT, __v9_);       \
		RcPtr __rc9_ = HEAPIFY(__opt9_);                               \
		__rc9_;                                                        \
	})

#define Zeroed(zeroed)                                                         \
	({                                                                     \
		bool __v10_ = zeroed;                                          \
		SlabAllocatorOptionPtr __opt10_ = BUILD_ENUM(                  \
		    SlabAllocatorOption, SLAB_OPTION_ZEROED, __v10_);          \
		RcPtr __rc10_ = HEAPIFY(__opt10_);                             \
		__rc10_;                                                       \
	})

#define SlabsPerResize(count)                                                  \
	({                                                                     \
		u64 __v11_ = count;                                            \
		SlabAllocatorOptionPtr __opt11_ =                              \
		    BUILD_ENUM(SlabAllocatorOption,                            \
			       SLAB_OPTION_SLABS_PER_RESIZE, __v11_);          \
		RcPtr __rc11_ = HEAPIFY(__opt11_);                             \
		__rc11_;                                                       \
	})

#define PROC_SLAB_CONFIG(value) ({ value; })

#define SLABS(...)                                                             \
	({                                                                     \
		int __counter___ = 0;                                          \
		EXPAND(FOR_EACH(COUNT_ARGS, __VA_ARGS__));                     \
		Result __rr___ = SlabAllocator_build(                          \
		    __counter___ __VA_OPT__(, ) __VA_OPT__(                    \
			EXPAND(FOR_EACH(PROC_SLAB_CONFIG, __VA_ARGS__))));     \
		SlabAllocatorPtr __sa__ = TRY(__rr___, __sa__);                \
		__sa__;                                                        \
	})

#endif // _CORE_SLABS__
