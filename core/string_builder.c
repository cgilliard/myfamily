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

GETTER(StringBuilder, ptr)
SETTER(StringBuilder, ptr)
SETTER(StringBuilder, len)
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
	char *v = GET(StringBuilder, sb, ptr);
	if (v) {
		myfree(v);
		SET(StringBuilder, sb, ptr, NULL);
	}
}
Result StringBuilder_build(char *s) {
	if (s == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "s cannot be null");
		return Err(e);
	}

	u64 slen = strlen(s);
	u64 initial_size = nearest_multiple_of_1000(slen + 1);

	char *scopy = mymalloc(sizeof(char) * (initial_size));
	strcpy(scopy, s);

	StringBuilderPtr ret = BUILD(StringBuilder, scopy, slen, initial_size);
	return Ok(ret);
}
Result StringBuilder_append(StringBuilder *s1, char *s2) {
	if (s1 == NULL || s2 == NULL) {
		Error e = ERR(ILLEGAL_ARGUMENT, "input must not be NULL");
		return Err(e);
	}

	char *s1_ptr = GET(StringBuilder, s1, ptr);
	if (s1_ptr == NULL) {
		Error e = ERR(ILLEGAL_STATE, "ptr must not be NULL");
		return Err(e);
	}

	u64 s1len = GET(StringBuilder, s1, len);
	u64 s2len = strlen(s2);
	u64 s1_new_len = s1len + s2len;
	u64 oldcapacity = GET(StringBuilder, s1, capacity);
	u64 ncapacity = nearest_multiple_of_1000(s1_new_len + 1);

	if (ncapacity != oldcapacity) {

		void *tmp = myrealloc(s1_ptr, sizeof(char) * (ncapacity));
		if (tmp == NULL) {
			return STATIC_ALLOC_RESULT;
		}

		strcat(tmp, s2);
		SET(StringBuilder, s1, ptr, tmp);
	} else {
		strcat(s1_ptr, s2);
	}
	SET(StringBuilder, s1, len, s1_new_len);
	SET(StringBuilder, s1, capacity, ncapacity);

	return Ok(_());
	return Ok(_());
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
	char *sbstr = GET(StringBuilder, sb, ptr);
	return String_build(sbstr);
}

u64 StringBuilder_len(StringBuilder *sb) { return GET(StringBuilder, sb, len); }
