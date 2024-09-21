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

#ifndef _UTIL_VEC__
#define _UTIL_VEC__

#include <base/heap.h>
#include <base/types.h>

typedef struct VecImpl VecImpl;

typedef struct VecNc {
	FatPtr impl;
} VecNc;

void vec_cleanup(VecNc *p);

#define Vec VecNc __attribute__((warn_unused_result, cleanup(vec_cleanup)))

int vec_init(Vec *p, u64 initial_capacity, u64 size_of_entry);
int vec_push(Vec *p, void *entry);
void *vec_element_at(const Vec *p, u64 index);
int vec_remove(Vec *p, u64 index);
void *vec_pop(Vec *p);
int vec_resize(Vec *p, u64 new_size);
int vec_truncate(Vec *p, u64 new_size);
int vec_clear(Vec *p);
u64 vec_size(const Vec *p);
u64 vec_capacity(const Vec *p);

#endif // _UTIL_VEC__
