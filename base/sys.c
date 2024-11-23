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

#include <base/bitmap.h>
#include <base/err.h>
#include <base/lock.h>
#include <base/macros.h>
#include <base/print_util.h>
#include <base/sys.h>
#include <base/util.h>
#include <fcntl.h>	   // for O_ constants
#include <sys/mman.h>  // for MAP and PROT constants

// needed system calls:
int open(const char *pathname, int flags, ...);
int close(int fd);
int lseek(int fd, off_t offset, int whence);
int ftruncate(int fd, off_t size);
int fsync(int fd);
void *mmap(void *addr, size_t length, int prot, int flags, int fd,
		   off_t offset);
int munmap(void *addr, size_t length);

// for macos
#ifndef O_DIRECT
#define O_DIRECT 0
#endif	// O_DIRECT

int _gfd = -1;
int64 cur_file_size = 0;
int64 _alloc_sum = 0;
Lock resize_lock = INIT_LOCK;

int check_size(int64 id) {
	if (_gfd == -1) return -1;

	int ret = 0;
	if ((1 + id) * PAGE_SIZE > ALOAD(&cur_file_size)) {
		lockw(&resize_lock);
		if ((1 + id) * PAGE_SIZE > ALOAD(&cur_file_size)) {
			ret = ftruncate(_gfd, (1 + id) * PAGE_SIZE);
			if (!ret)
				cur_file_size = (1 + id) * PAGE_SIZE;
			else
				SetErr(OutOfSpace);
		}
		unlock(&resize_lock);
	}
	return ret;
}

void *map(int64 pages) {
	if (pages == 0) return NULL;
	_alloc_sum += pages;
	void *ret = mmap(NULL, pages * PAGE_SIZE, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ret == NULL) SetErr(AllocErr);
	return ret;
}
void unmap(void *addr, int64 pages) {
	_alloc_sum -= pages;
	if (munmap(addr, pages * PAGE_SIZE)) panic("munmap error!");
}

void *fmap(int64 id) {
	if (check_size(id)) return NULL;
	_alloc_sum += 1;
	return mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _gfd,
				id * PAGE_SIZE);
}
void *fview(int64 id) {
	if (check_size(id)) return NULL;
	_alloc_sum += 1;
	return mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, _gfd, id * PAGE_SIZE);
}

int flush() {
	if (_gfd == -1) return -1;
	return fsync(_gfd);
}

int64 fsize() {
	return cur_file_size;
}

void init_sys(const char *path) {
	_gfd = open(path, O_DIRECT | O_RDWR);
	bool create = false;
	if (_gfd == -1) {
		create = true;
		_gfd = open(path, O_DIRECT | O_RDWR | O_CREAT, 0600);
	}
	if (_gfd == -1) panic("Could not open file [%s]", path);

#ifdef __APPLE__
	if (fcntl(_gfd, F_NOCACHE, 1))
		panic("Could not disable cache for file [%s]", path);
#endif	// __APPLE__

	if (!create) cur_file_size = lseek(_gfd, 0, SEEK_END);
}

void shutdown_sys() {
	if (_gfd != -1) {
		close(_gfd);
	}
}
