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

#ifndef _BASE_MMAP__
#define _BASE_MMAP__

#include <base/types.h>

unsigned long long mmap_aligned_size(unsigned long long size);
void *mmap_allocate(unsigned long long size);
void mmap_free(void *, unsigned long long size);

#ifdef TEST
extern int64 _allocation_sum;
void set_mmap_fail(int count);
#endif	// TEST

#endif	// _BASE_MMAP__