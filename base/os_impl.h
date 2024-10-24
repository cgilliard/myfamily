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

#ifndef _BASE_OS_IMPL__
#define _BASE_OS_IMPL__

#include <base/types.h>
// We rely on dirent.h to define how the DIR and struct dirent data structures look on each
// platform. We also rely on the following directory related functions: opendir, readdir, closedir
#include <dirent.h>

// File System Operations
i32 open(const u8 *path, i32 flags);
i64 write(i32 fd, const void *buf, u64 count);
i32 close(i32 fd);
u64 lseek(i32 fd, u64 offset, i32 whence);
i32 mkdir(const u8 *name, i32 mode);
i32 rmdir(const u8 *path);
i32 unlink(const u8 *pathname);
i32 chmod(const u8 *pathname, i32 mode);

// Path related
#define _PC_NAME_MAX                                                                               \
	4 /* TODO: this is the value on MacOS, but need to confirm other system values */
#define _PC_PATH_MAX                                                                               \
	5 /* TODO: this is the value on MacOS, but need to confirm other system values */
i32 fpathconf(i32 fd, i32 name);
i32 pathconf(const u8 *path, i32 name);
u8 *realpath(const u8 *path, u8 *resolved_path);

// Memory Manaagement Operations
#pragma clang diagnostic ignored "-Wincompatible-library-redeclaration"
#pragma GCC diagnostic ignored "-Wincompatible-library-redeclaration"
void *malloc(u64 size);
void free(void *ptr);
void *realloc(void *ptr, u64 size);

// Process Related Operations
void exit(i32 status);
extern i32 errno;

// Miscellaneous
u8 *getenv(const u8 *name);

// Define constants used
// O flags from fcntl (note: not guaranteed by POSIX to be the same, but in practice
// these values are used. TODO: add assertions in type.c)
#define O_RDONLY 0x0000	   /* open for reading only */
#define O_WRONLY 0x0001	   /* open for writing only */
#define O_RDWR 0x0002	   /* open for reading and writing */
#define O_CREAT 0x00000200 /* create if nonexistant */
#define O_DIRECTORY 0x00100000

#define SEEK_SET 0 /* set file offset to offset */
#define SEEK_CUR 1 /* set file offset to current plus offset */
#define SEEK_END 2 /* set file offset to EOF plus offset */

// Errno values that we use (values defined by POSIX)
#define ENOENT 2 /* No such file or directory */

#endif // _BASE_OS_IMPL__
