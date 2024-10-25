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

// Main OS interface

#ifndef _BASE_OS__
#define _BASE_OS__

#include <base/types.h>

typedef struct ResourceStats {
	u64 alloc_sum;
	u64 resize_sum;
	u64 release_sum;
	u64 fopen_sum;
	u64 fclose_sum;
} ResourceStats;

// Memory Management
void *alloc(u64 size, bool zeroed);
void *resize(void *, u64 size, bool zeroed);
void release(void *);

u64 alloc_sum();
u64 resize_sum();
u64 release_sum();

// Persistence
i32 persistent_set_root(const u8 *path);
void *persistent_alloc(const u8 *name, u64 size, bool zeroed);
void *persistent_resize(const u8 *name, u64 size);
i32 persistent_sync(void *ptr);
i32 persistent_delete(const u8 *name);

// Process Management
// TBD

// Network / IPC
// TBD

// Misc
u8 *env(const u8 *name);
void exit(i32 code);
// for println/print
i32 write_impl(i32 fd, const void *buf, u64 len);

#endif // _BASE_OS__
