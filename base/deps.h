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

#ifndef _BASE_DEPS__
#define _BASE_DEPS__

#include <base/types.h>
// needed for error number definitions
#include <errno.h>
// needed for O_ flags
#include <fcntl.h>
// needed for PATH_MAX
#include <limits.h>

void exit(int status);

extern i32 errno;

#ifdef __linux__
#define getenv(x) secure_getenv(x)
#endif // __linux__

u8 *getenv(const u8 *name);

#ifdef __linux__
u8 *secure_getenv(const u8 *name);
#endif // __linux__

i64 write(i32 fd, const void *buf, u64 count);
i32 close(i32 fd);
u64 lseek(i32 fd, u64 offset, i32 whence);
i32 mkdir(const u8 *name, i32 mode);

#pragma clang diagnostic ignored "-Wincompatible-library-redeclaration"
#pragma GCC diagnostic ignored "-Wincompatible-library-redeclaration"
void *malloc(u64 size);
void free(void *ptr);
void *realloc(void *ptr, u64 size);

char *realpath(const char *restrict path, char *restrict resolved_path);

#endif // _BASE_DEPS__
