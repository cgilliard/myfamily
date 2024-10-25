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

#ifndef _BASE_OS__
#define _BASE_OS__

#include <base/fam_err.h>
#include <base/macro_util.h>
#include <base/path.h>
#include <base/types.h>

// Constants
#define ENV_VAR_MAX 256
#define NAME_MAX 256

// type definitions
typedef struct ResourceStats {
	u64 alloc_sum;
	u64 resize_sum;
	u64 release_sum;
	u64 fopen_sum;
	u64 fclose_sum;
} ResourceStats;

typedef enum Whence {
	WhenceStart,
	WhenceCur,
	WhenceEnd,
} Whence;

typedef struct DirectoryEntry {
	u8 *name;
} DirectoryEntry;

typedef struct DirectoryNc {
	const u64 count;
	const DirectoryEntry *entries;
} DirectoryNc;
#define INIT_DIRECTORY {.count = 0}

typedef struct FileNc {
	u8 *full_path;
	bool is_dir;
	DirectoryNc *dir_info;
	u64 len;
} FileNc;

typedef struct StreamNc {
	const u64 handle;
} StreamNc;
#define INIT_STREAM {.handle = UINT64_MAX}

typedef struct EnvListEntry {
	const u8 *name;
	const u8 *value;
} EnvListEntry;

typedef struct EnvListNc {
	const u64 count;
	const EnvListEntry entries[];
} EnvListNc;
#define INIT_ENV_LIST {.count = 0}

// cleanup functions
void cleanup_directory(DirectoryNc *ptr);
void cleanup_file(FileNc *ptr);
void cleanup_stream(StreamNc *ptr);
void cleanup_env_list(EnvListNc *ptr);

#define Directory DirectoryNc __attribute__((warn_unused_result, cleanup(cleanup_directory)))
#define File FileNc __attribute__((warn_unused_result, cleanup(cleanup_file)))
#define Stream StreamNc __attribute__((warn_unused_result, cleanup(cleanup_stream)))
#define EnvList EnvListNc __attribute__((warn_unused_result, cleanup(cleanup_env_list)))

// Memory Management
void *alloc(u64 len, bool zeroed);
void release(void *ptr);
void *resize(void *ptr, u64 len);
u64 alloc_sum();
u64 resize_sum();
u64 release_sum();

// File System
i32 create(const Path *path, bool directory);
i32 rem(const Path *path);
i32 openfile(Stream *strm, const Path *path, u64 flags);
i32 closefile(const Stream *strm);
i32 seek(const Stream *strm, i64 offset, Whence whence);
i32 info(const Path *path, File *info);
i32 writestrm(const Stream *strm, const u8 *buf, u64 len);
i32 readstrm(const Stream *strm, u8 *buf, u64 len);
i32 changedir(const Path *path);
i32 pwd(Path *path);

// Process Management
// TBD

// Misc
u8 *env(const u8 *name);
i32 set_env(const u8 *name, const u8 *value);
i32 list_env(EnvList *env);

#endif // _BASE_OS__
