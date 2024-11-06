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

#ifndef _BASE_MEMMAP__
#define _BASE_MEMMAP__

#include <base/macros.h>
#include <base/types.h>

typedef unsigned int Ptr;

#define null ((Ptr)0)

#define MEM_MAP_NUM_CHUNKS 8192
#define MEM_MAP_CHUNK_SIZE 65536
#define MM_IMPL_SIZE (32 + MEM_MAP_NUM_CHUNKS * sizeof(byte *))
typedef struct MemMap {
	byte impl[MM_IMPL_SIZE];
} MemMap;

void *memmap_data(const MemMap *mm, Ptr ptr);
int memmap_init(MemMap *mm, unsigned int size);
Ptr memmap_allocate(MemMap *mm);
void memmap_free(MemMap *mm, Ptr ptr);

#endif	// _BASE_MEMMAP__
