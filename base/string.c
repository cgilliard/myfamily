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

#include <base/string.h>
#include <base/tlmalloc.h>

void string_free(StringPtr *s) {}
Result string_build(const char *ptr) {
	u64 x = 0;
	return Ok(x);
}
void string_copy(String *dst, String *src) {
	dst->len = src->len;

	dst->ptr = tlmalloc(sizeof(char) * (dst->len + 1));
	if (dst->ptr) {
		strcpy(dst->ptr, src->ptr);
	}
	// TODO: handle allocation error
}
size_t string_size(String *s) { return 0; }
bool string_equal(String *s1, String *s2) { return false; }
char *string_unwrap(String *s) { return NULL; }
