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

#ifndef _CORE_VEC__
#define _CORE_VEC__

#include <core/class.h>
#include <core/enum.h>
#include <core/traits.h>

#define TRAIT_VEC(T)                                                           \
	TRAIT_REQUIRED(T, Result, push, T##Ptr *ptr, void *elem)               \
	TRAIT_REQUIRED(T, Result, get, T##Ptr *ptr, u64 index)                 \
	TRAIT_REQUIRED(T, Result, set, T##Ptr *ptr, u64 index, void *elem)     \
	TRAIT_REQUIRED(T, Result, resize, T##Ptr *ptr, u64 new_size)

ENUM(VecOption, VARIANTS(VEC_CAPACITY), TYPES("u64"))
#define VecOption DEFINE_ENUM(VecOption)

CLASS(Vec, FIELD(char *, buf) FIELD(u64, capacity) FIELD(u64, len))
// IMPL(Vec, TRAIT_VEC)
// IMPL(Vec, TRAIT_APPEND)
#define Vec DEFINE_CLASS(Vec)

#define VEC_CAPACITY(cap)                                                      \
	({                                                                     \
		u64 __vv11_ = cap;                                             \
		VecOption __optv11_ =                                          \
		    BUILD_ENUM(VecOption, VEC_CAPACITY, __vv11_);              \
		RcPtr __rvv11_ = HEAPIFY(__optv11_);                           \
		__rvvb11_;                                                     \
	})

#define VEC(...) BUILD(Vec)

#endif // _CORE_VEC__
