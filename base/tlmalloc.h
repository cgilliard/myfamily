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

#ifndef _TLMALLOC_BASE__
#define _TLMALLOC_BASE__

#include <base/types.h>
#include <stddef.h>

typedef struct TLMallocStats {
	u128 cur_bytes;
	u64 cur_allocs;
	u64 cur_reallocs;
	u64 cur_frees;
} TLMallocStats;

void *tlmalloc(usize size);
void *tlrealloc(void *ptr, usize size);
void tlfree(void *ptr);
u128 cur_bytes_alloc();
u64 alloc_count();
u64 realloc_count();
u64 free_count();

#endif // _TLMALLOC_BASE__
