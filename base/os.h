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

#include <base/types.h>

// Memory Management
void *alloc(u64 size, bool zeroed);
void release(void *ptr);
void *resize(void *ptr, u64 size);

typedef enum Whence {
	WhenceStart,
	WhenceCur,
	WhenceEnd,
} Whence;

typedef struct DirectoryEntry {
	const char *name;
} DirectoryEntry;

typedef struct Directory {
	const u64 count;
	const DirectoryEntry *entries;
} Directory;

typedef struct Path {
	const u8 *name;
} Path;

typedef struct FileInfo {
	const u8 *full_path;
} FileInfo;

// File System
i64 create(const u8 *path, bool directory);
i64 rem(const u8 *path);
i64 open(const u8 *path, u64 flags);
i64 close(i64 handle);
i64 seek(i64 handle, i64 offset, Whence whence);
i64 finfo(const u8 *path, FileInfo *finfo);

// Process Management
