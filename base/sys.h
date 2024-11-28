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

typedef unsigned long size_t;
typedef long long ssize_t;

int getpagesize();
void sched_yield();
int munmap(void *addr, size_t n);
ssize_t write(int fd, const void *buf, size_t count);

#endif	// _BASE_SYS__
