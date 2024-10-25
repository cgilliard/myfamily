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
u8 persistent_base[PATH_MAX + 1] = {"."};

i32 persistent_set_root(const u8 *path) {
	if (!path || mystrlen(path) >= PATH_MAX) {
		SetErr(IllegalArgument);
		return -1;
	}
	mystrcpy(persistent_base, path, PATH_MAX);
	return 0;
}

void *persistent_alloc(const u8 *name, u64 size, bool zeroed) {
	if (!name || !size || mystrlen(name) > NAME_MAX ||
		mystrlen(name) + mystrlen(persistent_base) + 1 > PATH_MAX) {
		SetErr(IllegalArgument);
		return NULL;
	}
	char path[PATH_MAX + 1];
	mystrcpy(path, persistent_base, PATH_MAX);
	mystrcat(path, "/", PATH_MAX - mystrlen(path));
	mystrcat(path, name, PATH_MAX - mystrlen(path));

	i32 fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0700);
	fchmod(fd, 0700);

	if (ftruncate(fd, size) == -1) {
		perror("ftruncate");
		exit(-1);
	}

	char buf[size];
	for (int i = 0; i < size; i++)
		buf[i] = 'x';
	i32 res = write(fd, buf, size);
	if (res < 0) {
		perror("write");
		exit(-1);
	}
	fsync(fd);
	lseek(fd, 0, SEEK_SET);
	void *ret = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (ret == NULL) {
		perror("mmap");
		exit(-1);
	}
	for (int i = 0; i < size * 2; i++)
		((char *)ret)[i] = 'a';
	if (msync(ret, size, MS_SYNC)) {
		perror("msync");
		exit(-1);
	}
	munmap(ret, 0);
	close(fd);
	return ret;
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

i32 write_impl(i32 fd, const void *buf, u64 len) {
	return write(fd, buf, len);
}
