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

#include <base/ekinds.h>
#include <base/panic.h>
#include <base/string.h>
#include <base/tlmalloc.h>

void string_free(StringPtr *s) {
	if (!s->no_cleanup && s->ptr) {
		tlfree(s->ptr);
		s->ptr = NULL;
	}
}
Result string_build(const char *s) {
	u64 len = strlen(s);
	char *scopy = tlmalloc((len + 1) * sizeof(char));
	if (scopy == NULL) {
		Error e = ERROR(ILLEGAL_STATE, "could not allocate memory");
		return Err(e);
	}
	strcpy(scopy, s);
	String ret = {&StringVtable, scopy, len, false};
	return Ok(ret);
}

bool string_copy(String *dst, String *src) {
	dst->len = src->len;
	dst->no_cleanup = false;

	dst->ptr = tlmalloc(sizeof(char) * (dst->len + 1));
	if (dst->ptr) {
		strcpy(dst->ptr, src->ptr);
		return true;
	} else {
		return false;
	}

	// TODO: handle allocation error
}
size_t string_size(String *s) { return sizeof(String); }
bool string_equal(String *s1, String *s2) {
	return !strcmp(to_str(s1), to_str(s2));
}
char *string_unwrap(String *s) { return s->ptr; }

String string_build_expect(const char *ptr) {
	Result r = string_build(ptr);
	StringPtr *p = unwrap(&r);
	StringPtr ret = EMPTY_STRING;
	ret.no_cleanup = false;
	copy(&ret, p);
	return ret;
}
