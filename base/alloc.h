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

typedef unsigned int Ptr;

#define ALLOC_IMPL_SIZE 80
typedef struct Alloc {
	byte impl[ALLOC_IMPL_SIZE];
} Alloc;

typedef struct Slab {
	Ptr ptr;
	byte *data;
} Slab;

int alloc_init(Alloc *a, unsigned int size, unsigned int max_slabs);
Slab alloc(Alloc *a);
void release(Alloc *a, Ptr ptr);
void alloc_cleanup(Alloc *a);

#endif	// _BASE_ALLOC__
