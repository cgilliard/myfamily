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

#ifndef _CORE_MYMALLOC__
#define _CORE_MYMALLOC__

#include <core/slabs.h>
#include <stddef.h>

typedef struct ResourceStats {
	u64 malloc_sum;
	u64 realloc_sum;
	u64 free_sum;
	u64 fopen_sum;
	u64 fclose_sum;
} ResourceStats;

int mymalloc(SlabAllocator *sa, Slab *slab, u64 size);
int myrealloc(SlabAllocator *sa, Slab *slab, u64 size);
int myfree(SlabAllocator *sa, Slab *slab);
FILE *myfopen(const char *path, const char *mode);
void myfclose(FILE *ptr);
ResourceStats get_resource_stats();

#endif // _CORE_MYMALLOC__