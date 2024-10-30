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

#ifndef _BASE_FAM_ALLOC__
#define _BASE_FAM_ALLOC__

#define PTR_FLAGS_SEND 0
#define PTR_FLAGS_DIRECT 1

#include <base/slabs.h>

Ptr fam_alloc(unsigned int size, bool send);
Ptr fam_resize(Ptr ptr, unsigned int size);
void fam_release(Ptr *ptr);

void fam_alloc_thread_local_cleanup();

#ifdef TEST
void fam_alloc_global_cleanup();
int64 fam_alloc_count_tl_slab_allocator();
int64 fam_alloc_count_global_allocator();
#endif // TEST

#endif // _BASE_FAM_ALLOC__
