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

#ifndef _BASE_SYS__
#define _BASE_SYS__

#include <base/types.h>

int getpagesize();
#define PAGE_SIZE (getpagesize())

void *map(int64 pages);
void unmap(void *addr, int64 pages);
void *fmap(int64 id);
int flush();
int64 fsize();
int64 send(int fd, const byte *buf, uint64 len);
int64 recv(int fd, byte *buf, uint64 len);
void __attribute__((noreturn)) halt(int code);

void init_sys(const char *path);
void shutdown_sys();

#ifdef TEST
extern int64 _alloc_sum;
#endif	// TEST

#endif	// _BASE_SYS__
