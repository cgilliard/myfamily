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

#include <base/fam_err.h>
#include <base/os.h>
#include <base/unix_impl.h>
#include <stdio.h>

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};

void cleanup_directory(DirectoryNc *ptr) {
	if (!ptr) {
		SetErr(IllegalArgument);
		return;
	}
	if (ptr->count) {
		DirectoryNc *ptr_unconst;
#if defined(__clang__)
// Clang-specific pragma
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__) && !defined(__clang__)
// GCC-specific pragma
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#warning "Unknown compiler or platform. No specific warning pragmas applied."
		ptr_unconst = ptr;
#endif

		for (u64 i = 0; i < ptr->count; i++) {
			release(&ptr_unconst->entries[i].name);
			release(&ptr_unconst->entries[i]);
		}
	}
}

void cleanup_file(FileNc *ptr) {
	if (!ptr) {
		SetErr(IllegalArgument);
		return;
	}
}
void cleanup_stream(StreamNc *ptr) {
	if (!ptr) {
		SetErr(IllegalArgument);
		return;
	}
}
void cleanup_env_list(EnvListNc *ptr) {
	if (!ptr) {
		SetErr(IllegalArgument);
		return;
	}
}

void *alloc(u64 len, bool zeroed) {
	void *ret;
	ret = malloc(len);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.alloc_sum += 1;
	} else
		SetErr(AllocErr);

	return ret;
}
void release(void *ptr) {
	THREAD_LOCAL_RESOURCE_STATS.release_sum += 1;
	free(ptr);
}
void *resize(void *ptr, u64 len) {
	void *ret;
	ret = realloc(ptr, len);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.resize_sum += 1;
	} else
		SetErr(AllocErr);
	return ret;
}

u64 alloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.alloc_sum;
}
u64 resize_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.resize_sum;
}
u64 release_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.release_sum;
}

i32 create(const Path *path, bool directory) {
	return 0;
}
i32 rem(const Path *path) {
	return 0;
}
i32 openfile(Stream *strm, const Path *path, u64 flags) {
	return 0;
}
i32 closefile(const Stream *strm) {
	return 0;
}
i32 seek(const Stream *strm, i64 offset, Whence whence) {
	return 0;
}
i32 info(const Path *path, File *info) {
	return 0;
}
i32 writestrm(const Stream *strm, const u8 *buf, u64 len) {
	return 0;
}
i32 readstrm(const Stream *strm, u8 *buf, u64 len) {
	return 0;
}
i32 changedir(const Path *path) {
	return 0;
}
i32 pwd(Path *path) {
	return 0;
}

u8 *env(const u8 *name) {
	return 0;
}
i32 set_env(const u8 *name, const u8 *value) {
	return 0;
}
i32 list_env(EnvList *env) {
	return 0;
}
