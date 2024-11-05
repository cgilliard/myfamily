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

#ifndef _BASE_OSDEF__
#define _BASE_OSDEF__

#include <base/types.h>
#include <sys/mman.h>

void *malloc(size_t size);
void free(void *ptr);
void exit(int);
int getpagesize();
char *getenv(const char *name);
void *memcpy(void *, const void *, size_t);
void *memset(void *ptr, int x, size_t n);

#endif	// _BASE_OSDEF__
