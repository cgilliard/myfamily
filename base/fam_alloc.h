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

#include <base/slabs.h>

typedef struct SendStateGuardNc {
	FatPtr impl;
} SendStateGuardNc;

void send_guard_cleanup(SendStateGuardNc *ptr);

#define SendStateGuard                                                                             \
	SendStateGuardNc __attribute__((warn_unused_result, cleanup(send_guard_cleanup)))

#define SEND_GUARD_INIT = {null}
#define SetSend(v)                                                                                 \
	({                                                                                             \
		SendStateGuardNc _sg__ = {null};                                                           \
		fam_alloc_increment_state(v);                                                              \
		_sg__;                                                                                     \
	})
#define ExecSend(v, exe)                                                                           \
	{                                                                                              \
		SendStateGuardSet ____ = SetSend(v);                                                       \
		exe                                                                                        \
	}

int fam_alloc(FatPtr *ptr, u64 size);
int fam_realloc(FatPtr *ptr, u64 size);
void fam_free(FatPtr *ptr);
void fam_alloc_increment_state(bool send);

#ifdef TEST
void fam_alloc_cleanup();
u64 fam_alloc_count_tl_slab_allocator();
u64 fam_alloc_count_global_allocator();
#endif // TEST

#endif // _BASE_FAM_ALLOC__
