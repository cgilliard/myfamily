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

#ifndef _BASE_STRING__
#define _BASE_STRING__

#include <base/types.h>
#include <stddef.h>

void *memcpy(void *dest_str, const void *src_str, size_t n);
i32 memcmp(const void *str1, const void *str2, size_t n);
i32 strncmp(const char *lhs, const char *rhs, size_t n);
unsigned long strnlen(const char *s, size_t n);
char *strncat(char *s1, const char *s2, size_t n);
char *strncpy(char *s1, const char *s2, size_t n);
i32 snprintf(char *__restrict __str, size_t __size, const char *__restrict __format, ...);
void *memset(void *str, i32 c, size_t n);
unsigned long strlen(const char *);
char *strstr(const char *s1, const char *s2);

#endif // _BASE_STRING__
