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

#ifndef _BASE_PRINT_UTIL__
#define _BASE_PRINT_UTIL__

#include <base/types.h>
#ifndef va_list
#define va_list __builtin_va_list
#define va_start(args, n) __builtin_va_start(args, n)
#define va_end(args) __builtin_va_end(args)
#endif	// va_list

int println(const byte *fmt, ...);
int print(const byte *fmt, ...);
int vprint(const byte *fmt, va_list args);
int sprint(byte *str, uint64 capacity, const byte *fmt, ...);
int vsprint(byte *str, uint64 capacity, const byte *fmt, va_list args);
void panic(const byte *fmt, ...);

#ifdef TEST
extern bool _debug_print_util_disable__;
#endif	// TEST

#endif	// _BASE_PRINT_UTIL__
