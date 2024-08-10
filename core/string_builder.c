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
#include <core/formatter.h>
#include <core/string.h>
#include <core/string_builder.h>

SETTER(StringBuilder, slab)
SETTER(StringBuilder, capacity)

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

void StringBuilder_cleanup(StringBuilder *sb) {
	Slab slab = GET(StringBuilder, sb, slab);
	char *v = slab.data;
	if (v) {
		myfree(&slab);
		slab.data = NULL;
		SET(StringBuilder, sb, slab, slab);
	}
}
Result StringBuilder_build(char *s) {
	if (s == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "s cannot be null");
		return Err(e);
	}

	u64 slen = strlen(s);
	u64 initial_size = nearest_multiple_of_1000(slen + 1);

	Slab slab;
	if (mymalloc(&slab, sizeof(char) * (initial_size)))
		return STATIC_ALLOC_RESULT;
	strcpy(slab.data, s);

	StringBuilder ret = BUILD(StringBuilder, slab, initial_size);
	return Ok(ret);
}
Result StringBuilder_append(StringBuilder *s1, char *s2) {
	if (s1 == NULL || s2 == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "input must not be NULL");
		return Err(e);
	}

	Slab slab = GET(StringBuilder, s1, slab);
	char *s1_ptr = slab.data;
	if (s1_ptr == NULL) {
		Error e = ERR(ILLEGAL_STATE, "ptr must not be NULL");
		return Err(e);
	}

	u64 s1len = strlen(slab.data);
	u64 s2len = strlen(s2);
	u64 s1_new_len = s1len + s2len;
	u64 oldcapacity = GET(StringBuilder, s1, capacity);
	u64 ncapacity = nearest_multiple_of_1000(s1_new_len + 1);

	if (ncapacity != oldcapacity) {
		if (myrealloc(&slab, sizeof(char) * (ncapacity))) {
			return STATIC_ALLOC_RESULT;
		}
	}
	strcat(s1_ptr, s2);
	SET(StringBuilder, s1, capacity, ncapacity);

	return Ok(UNIT);
}
Result StringBuilder_append_s(StringBuilder *s1, Object *s2) {
	if (strcmp(CLASS_NAME(s2), "String")) {
		Error e = ERR(UNEXPECTED_CLASS_ERROR,
			      "Trying to append class '%s', expected String\n",
			      CLASS_NAME(s2));
		return Err(e);
	}

	StringPtr *s2ptr = (StringPtr *)s2;
	return StringBuilder_append(s1, unwrap(s2));
}

Result StringBuilder_to_string(StringBuilder *sb) {
	Slab slab = GET(StringBuilder, sb, slab);
	return String_build(slab.data);
}

u64 StringBuilder_len(StringBuilder *sb) {
	Slab slab = GET(StringBuilder, sb, slab);
	return strlen(slab.data);
}

Result StringBuilder_fmt(StringBuilder *sb, Formatter *f) {
	Slab slab = GET(StringBuilder, sb, slab);
	return WRITE(f, slab.data);
}
