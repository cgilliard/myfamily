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

#define PTR_FLAGS_DIRECT 0

#include <base/slabs.h>

Ptr fam_alloc(unsigned int size);
Ptr fam_resize(Ptr ptr, unsigned int size);
void fam_release(Ptr ptr);
Ptr fam_ptr_for(unsigned int id, unsigned int len);
Ptr fam_ptr_copy(Ptr ptr);
void fam_alloc_init();

#ifdef TEST
void fam_alloc_cleanup();
int64 fam_alloc_count_global_allocator();
#endif	// TEST

#endif	// _BASE_FAM_ALLOC__
