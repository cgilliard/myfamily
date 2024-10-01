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

#ifndef _BASE_RESOURCES__
#define _BASE_RESOURCES__

#include <base/path.h>
#include <base/types.h>

#define NULL ((void *)0)

typedef struct MYFILE MYFILE;

typedef struct ResourceStats {
	u64 malloc_sum;
	u64 realloc_sum;
	u64 free_sum;
	u64 fopen_sum;
	u64 fclose_sum;
} ResourceStats;

void *mymalloc(u64 size);
void *myrealloc(void *ptr, u64 size);
void myfree(void *ptr);
MYFILE *myfopen(const Path *path, const char *mode);
void myfclose(MYFILE *ptr);
u64 mymalloc_sum();
u64 myrealloc_sum();
u64 myfree_sum();
u64 myfopen_sum();
u64 myfclose_sum();

#ifdef TEST
extern bool __is_debug_malloc;
extern bool __is_debug_realloc;
extern u64 __is_debug_malloc_counter_;
extern u64 __is_debug_realloc_counter_;
#endif // TEST

#endif // _BASE_RESOURCES__
