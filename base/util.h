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

#ifndef _BASE_UTIL__
#define _BASE_UTIL__

#include <base/types.h>

void copy_bytes(byte *dest, const byte *src, unsigned long long n);
void set_bytes(byte *dst, byte b, unsigned long long n);
unsigned long long cstring_len(const char *S);
int cstring_compare(const char *s1, const char *s2);
int cstring_compare_n(const byte *s1, const byte *s2, unsigned long long n);

#endif	// _BASE_UTIL__