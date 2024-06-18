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
#include <base/string.h>

GETTER(String, ptr)
SETTER(String, ptr)
GETTER(String, len)
SETTER(String, len)

void String_cleanup(StringPtr *s) {
	char *ptr = *String_get_ptr(s);
	if (ptr != NULL) {
		tlfree(ptr);
		String_set_ptr(s, NULL);
		String_set_len(s, 0);
	}
}
size_t String_size(String *s) { return sizeof(String); }
bool String_copy(String *dst, String *src) {
	u64 len = *String_get_len(src);
	String_set_len(dst, len);
	void *tmp = tlmalloc(sizeof(char) * (1 + len));
	String_set_ptr(dst, tmp);

	char *dst_ptr = *String_get_ptr(dst);
	char *src_ptr = *String_get_ptr(src);
	if (dst_ptr == NULL)
		return false;
	else {
		strcpy(dst_ptr, src_ptr);
		return true;
	}
}
void *String_unwrap(StringPtr *obj) {
	char *ptr = *String_get_ptr(obj);
	return ptr;
}
bool String_equal(String *obj1, String *obj2) {
	char *obj1_ptr = *String_get_ptr(obj1);
	char *obj2_ptr = *String_get_ptr(obj2);
	if (obj1_ptr == NULL || obj2_ptr == NULL)
		return false;
	return !strcmp(obj1_ptr, obj2_ptr);
}
Result String_build(const char *s) {
	if (s == NULL) {
		Error e = ERROR(ILLEGAL_ARGUMENT, "char pointer was NULL");
		return Err(e);
	}
	u64 len = strlen(s);
	char *ptr = tlmalloc(sizeof(char) * (1 + len));
	if (ptr == NULL) {
		Error e = ERROR(ALLOC_ERROR, "Could not allocate memory");
		return Err(e);
	}
	strcpy(ptr, s);

	String ret = BUILD(String, ptr, len);
	return Ok(ret);
}

String String_build_expect(const char *s) {
	if (s == NULL)
		panic("char pointer was NULL");
	u64 len = strlen(s);
	char *ptr = tlmalloc(sizeof(char) * (1 + len));
	if (ptr == NULL) {
		panic("Allocation Error: Could not allocate memory");
	}
	strcpy(ptr, s);

	StringPtr ret = BUILD(String, ptr, len);
	return ret;
}
