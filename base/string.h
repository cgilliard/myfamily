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

i32 mymemcmp(const u8 *s1, const u8 *s2, u64 len);
i32 mystrcmp(const u8 *s1, const u8 *s2);
u8 *mystrcat(u8 *dest, const u8 *src, u64 limit);
i32 mystrlen(const u8 *s);
const u8 *mystrstr(const u8 *str1, const u8 *str2);
u8 *mystrcpy(u8 *destination, const u8 *source, u64 limit);
i32 mystrncmp(const u8 *X, const u8 *Y, u64 limit);
void *mymemcpy(void *destination, const void *source, u64 limit);
void reverse(u8 str[], u64 length);
u8 *citoau64(u64 num, u8 *str, u64 base);
u8 *citoai64(i64 num, u8 *str, u64 base);
const u8 *rstrstr(const u8 *s1, const u8 *s2);
void memzero(void *ptr, u64 size);

#ifdef TEST
extern bool __is_debug_mystrlen_overflow;
#endif // TEST

#endif // _BASE_STRING__