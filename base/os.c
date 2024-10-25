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
#include <base/print_util.h>
#include <base/string.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

u8 *getenv(const u8 *name);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
void *memset(void *ptr, i32 x, size_t n);

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};

void *alloc(u64 size, bool zeroed) {
	if (!size) {
		SetErr(IllegalArgument);
		return NULL;
	}
	void *ret;
	ret = malloc(size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.alloc_sum += 1;
		if (zeroed)
			memset(ret, 0, size);
	} else
		SetErr(AllocErr);
	return ret;
}
void *resize(void *ptr, u64 size, bool zeroed) {
	if (!ptr || !size) {
		SetErr(IllegalArgument);
		return NULL;
	}
	void *ret;
	ret = realloc(ptr, size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.resize_sum += 1;
		if (zeroed)
			memset(ret, 0, size);
	} else
		SetErr(AllocErr);
	return ret;
}

void release(void *ptr) {
	if (!ptr) {
		SetErr(IllegalArgument);
		return;
	}
	THREAD_LOCAL_RESOURCE_STATS.release_sum += 1;
	free(ptr);
}

void release_no_stat(void *ptr) {
	if (!ptr) {
		SetErr(IllegalArgument);
		return;
	}
	free(ptr);
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

// Persistence
void *persistent_alloc(const u8 *path, u64 size, bool zeroed) {
	return NULL;
}

void *persistent_resize(const u8 *name, u64 size) {
	return NULL;
}

i32 persistent_sync(void *ptr) {
	return 0;
}

i32 persistent_delete(const u8 *name) {
	return 0;
}

u8 *env(const u8 *name) {
	return getenv(name);
}

i64 strm_write(const Stream *strm, const void *buf, u64 len) {
	if (!strm || strm->handle < 0 || buf == NULL || len == 0) {
		SetErr(IllegalArgument);
		return -1;
	}
	return write(strm->handle, buf, len);
}

i64 strm_read(const Stream *strm, void *buf, u64 len) {
	if (!strm || strm->handle < 0 || buf == NULL || len == 0) {
		SetErr(IllegalArgument);
		return -1;
	}
	return read(strm->handle, buf, len);
}

i64 strm_open(Stream *strm, const u8 *command, const u8 *type) {
	FILE *fp = popen(command, type);
	if (fp == NULL) {
		SetErr(POpenErr);
		return -1;
	}
	i32 fd = fileno(fp);
	strm->handle = fd;
	strm->strm = fp;
	return 0;
}

void strm_close(const Stream *strm) {
	if (strm->strm != NULL)
		pclose(strm->strm);
	else
		close(strm->handle);
}
