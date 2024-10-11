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

#include <base/bitflags.h>
#include <base/fam_err.h>
#include <string.h>

int bitflags_set(BitFlags *ptr, u32 index, bool value) {
	u32 offset = index >> 3;

	// Out of range check
	if (offset >= ptr->capacity) {
		fam_err = IndexOutOfBounds;
		return -1;
	}

	// Calculate mask directly
	u32 mask = 1 << (index & 0x7);

	if (value) {
		ptr->flags[offset] |= mask;
	} else {
		ptr->flags[offset] &= ~mask;
	}

	return 0;
}

bool bitflags_check(BitFlags *ptr, u32 index) {
	u32 offset = index >> 3;

	// Out of range check
	if (offset >= ptr->capacity) {
		return false;
	}

	// Calculate mask directly
	u32 mask = 1 << (index & 0x7);

	return ptr->flags[offset] & mask;
}
