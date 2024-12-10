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

#ifdef __linux__
long unsigned int getpagesize();
#elif defined(__APPLE__)
int getpagesize();
#endif
#define PAGE_SIZE (getpagesize())

void *map(u64 pages);
void unmap(void *addr, u64 pages);
int os_sleep(u64 millis);
int set_timer(void (*alarm)(int), u64 millis);
int unset_timer();

int sched_yield(void);
int getentropy(void *buffer, size_t length);

char *backtrace_full();
char *last_trace();

ssize_t write(int fd, const void *buf, size_t count);
ssize_t read(int fd, void *buf, size_t count);

int snprintf(char *buf, unsigned long capacity, const char *fmt, ...);
void __attribute__((noreturn)) _exit(int code);
i128 getnanos();

#ifdef TEST
extern u64 _alloc_sum;
#endif	// TEST

#endif	// _BASE_SYS__
