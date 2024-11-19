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

#include <base/err.h>
#include <base/lock.h>
#include <base/macros.h>
#include <base/map.h>
#include <base/print_util.h>
#include <base/util.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/stat.h>

unsigned long long _alloc_sum = 0;

uid_t getuid(void);
char *strerror(int e);
int close(int fd);
int ftruncate(int fd, off_t size);

int _gfd = -1;
int64 _cur_size = 0;
Lock _gfd_lock = INIT_LOCK;

void fmap_init_path(const char *fpath) {
	_gfd = open(fpath, O_RDWR);
	if (_gfd == -1) _gfd = open(fpath, O_CREAT | O_RDWR, 0600);
	if (_gfd == -1) panic("Could not open global file: %s", strerror(errno));
}

void fmap_close() {
	if (_gfd != -1 && close(_gfd)) panic("close error: %s", strerror(errno));
}

#ifndef TEST
static void __attribute__((constructor)) open_gfd() {
	struct passwd *pw = getpwuid(getuid());
	int len = cstring_len(pw->pw_dir);
	char fpath[len + 10];
	copy_bytes(fpath, pw->pw_dir, len);
	copy_bytes(fpath + len, "/.fam.dat", 9);
	fpath[len + 9] = 0;

	fmap_init_path(fpath);
}
static void __attribute__((destructor)) close_gfd() {
	fmap_close();
}
#endif	// TEST

void *map(unsigned long long pages) {
	if (pages == 0) return NULL;
	_alloc_sum += pages;
	return mmap(NULL, pages * PAGE_SIZE, PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}
void *fmap(unsigned long long pages, unsigned long long offset) {
	if (pages == 0) return NULL;
	if (_gfd == -1) panic("Global file descriptor not initialized");
	bool size_ok = false;
	lockr(&_gfd_lock);
	if (_cur_size >= (pages + offset)) size_ok = true;
	unlock(&_gfd_lock);

	if (!size_ok) {
		lockw(&_gfd_lock);
		struct stat st;
		fstat(_gfd, &st);
		unsigned long long size = st.st_size / PAGE_SIZE;
		if (size < pages + offset) {
			if (ftruncate(_gfd, (pages + offset) * PAGE_SIZE)) {
				SetErr(TooBig);
				unlock(&_gfd_lock);
				return NULL;
			}
			_cur_size = pages + offset;
		} else
			_cur_size = size;

		unlock(&_gfd_lock);
	}
	_alloc_sum += pages;
	return mmap(NULL, pages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
				_gfd, offset * PAGE_SIZE);
}
void unmap(void *addr, unsigned long long pages) {
	_alloc_sum -= pages;

	if (munmap(addr, pages * PAGE_SIZE))
		panic("munmap error: %s", strerror(errno));
}

void flush(void *addr, unsigned long long pages) {
	if (msync(addr, pages * PAGE_SIZE, MS_SYNC))
		panic("msync error: %s", strerror(errno));
}
