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

#ifndef _BASE_MAP__
#define _BASE_MAP__

#include <base/types.h>

int getpagesize();
#define PAGE_SIZE (getpagesize())

byte *map(unsigned long long pages);
byte *fmap(unsigned long long pages, unsigned long long offset);
void unmap(byte *addr, unsigned long long pages);
void flush(byte *addr, unsigned long long pages);

#ifdef TEST
void fmap_init_path(const char *fpath);
void fmap_close();
#endif	// TEST

#endif	// _BASE_MAP__
