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

#ifndef _BASE_BITMAP__
#define _BASE_BITMAP__

#include <base/types.h>

#define BITMAP_IMPL_SIZE 32
typedef struct BitMap {
	byte impl[BITMAP_IMPL_SIZE];
} BitMap;

int bitmap_init(BitMap *m, int bitmap_ptr_pages);
int64 bitmap_allocate(BitMap *m);
void bitmap_free(BitMap *m, unsigned long long index);
void bitmap_cleanup(BitMap *m);

int bitmap_sync(BitMap *dst, BitMap *src);
void bitmap_clean(BitMap *m);
int64 bitmap_ptr_count(BitMap *m);
int bitmap_extend(BitMap *m, void *ptr);

#endif	// _BASE_BITMAP__
