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

#ifndef _UTIL_BITFLAGS__
#define _UTIL_BITFLAGS__

#include <base/types.h>

typedef struct BitFlags {
	u8 *flags;
	u64 capacity;
} BitFlags;

int bitflags_set(BitFlags *ptr, u32 index, bool value);
bool bitflags_check(const BitFlags *ptr, u32 index);

#endif // _UTIL_BITFLAGS__
