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

#ifndef _STRING_BASE__
#define _STRING_BASE__

#include <base/result.h>
#include <base/vtable.h>

#define String StringPtr Cleanup(string_free)

typedef struct StringPtr {
	Vtable *vtable;
	char *ptr;
	u64 len;
	bool no_cleanup;
} StringPtr;

void string_free(StringPtr *s);
Result string_build(const char *ptr);
String string_build_expect(const char *ptr);
bool string_copy(String *dst, String *src);
size_t string_size(String *s);
bool string_equal(String *s1, String *s2);
char *string_unwrap(String *s);

// vtable
static VtableEntry StringVtableEntries[] = {
    {"copy", string_copy},     {"size", string_size},
    {"equal", string_equal},   {"cleanup", string_free},
    {"unwrap", string_unwrap}, {"to_str", string_unwrap}};

DEFINE_VTABLE(StringVtable, StringVtableEntries)

#define EMPTY_STRING {&StringVtable, "", 0, true}
#define Str(s) string_build_expect(s)

#endif // _STRING_BASE__
