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

#include <core/ekinds.h>
#include <core/string.h>
#include <core/string_builder.h>

u64 nearest_multiple_of_1000(u64 number) {
	// Compute the remainder when divided by 1000
	u64 remainder = number % 1000;

	// If the remainder is zero, the number is already a multiple of 1000
	if (remainder == 0) {
		return number;
	}

	// Otherwise, add the difference to the next multiple of 1000
	return number + (1000 - remainder);
}

void StringBuilder_cleanup(StringBuilder *ptr) {}
Result StringBuilder_build(char *s) {
	if (s == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "s cannot be null");
		return Err(e);
	}

	u64 slen = strlen(s);
	u64 initial_size = nearest_multiple_of_1000(slen);

	return Ok(_());
}
Result StringBuilder_append(StringBuilder *dst, char *ptr) { return Ok(_()); }
Result StringBuilder_append_s(StringBuilder *s1, Object *s2) { return Ok(_()); }
