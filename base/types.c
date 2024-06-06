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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <base/types.h>

void string_free(StringImpl *s) {
	if(s->ptr) {
		free(s->ptr);
		s->ptr = NULL;
	}
}

int string_set(StringImpl *s, const char *ptr) {
	int ret = 0;
	s->len = strlen(ptr);

	s->ptr = malloc(sizeof(char) * (s->len + 1));
	if(s->ptr == NULL)
		ret = -1;

	if(!ret)
		strcpy(s->ptr, ptr);

	return ret;
}

i64 saddi64(i64 a, i64 b)
{
    if (a > 0) {
        if (b > INT64_MAX - a) {
            return INT64_MAX;
        }
    } else if (b < INT64_MIN - a) {
            return INT64_MIN;
    }

    return a + b;
}

u64 saddu64(u64 a, u64 b) {
  return (a > 0xFFFFFFFFFFFFFFFF - b) ? 0xFFFFFFFFFFFFFFFF : a + b;
}
