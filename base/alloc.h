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

#ifndef _BASE_ALLOC__
#define _BASE_ALLOC__

#include <base/types.h>

unsigned int page_aligned_size(unsigned int size);

typedef struct ResourceStats {
	int64 alloc_sum;
	int64 resize_sum;
	int64 release_sum;
	int64 fopen_sum;
	int64 fclose_sum;
} ResourceStats;

typedef struct Alloc {
	void *ptr;
	unsigned int size;
} Alloc;

Alloc alloc(unsigned int size);
void release(Alloc alloc);

int64 alloc_sum();
int64 resize_sum();
int64 release_sum();

#endif	// _BASE_ALLOC__
