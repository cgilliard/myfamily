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

// Main OS interface

#ifndef _BASE_OS__
#define _BASE_OS__

#include <base/types.h>

typedef struct ResourceStats {
	number alloc_sum;
	number resize_sum;
	number release_sum;
	number fopen_sum;
	number fclose_sum;
} ResourceStats;

// Memory Management
void *alloc(number size, number zeroed);
void *resize(void *, number size);
void release(void *);
void release_no_stat(void *);

number alloc_sum();
number resize_sum();
number release_sum();

// Misc
byte *env(const byte *name);

#endif // _BASE_OS__
