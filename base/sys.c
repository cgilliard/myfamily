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
#include <fcntl.h>
#include <sys/mman.h>

// needed system calls:
int close(int fd);
int lseek(int fd, off_t offset, int whence);
int ftruncate(int fd, off_t size);
int fsync(int fd);
// also mmap/munmap

// for macos
#ifndef O_DIRECT
#define O_DIRECT 0
#endif	// O_DIRECT

#define VERSION '\0'
#define MAGIC "^1337*Z"
#define MAX_BITMAP_PTR_PAGES 16

int _gfd = -1;
int64 _alloc_sum = 0;

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

typedef struct SuperBlock {
	byte version;
	byte magic[7];
	uint64 seqno;
	int64 alternate;
	int64 bitmap;
	int64 root;
	int64 size;
	byte padding[];
} SuperBlock;

SuperBlock *super0 = NULL;
SuperBlock *super1 = NULL;
BitMap sbm0;
BitMap sbm1;
void *bitmap_ptrs0, *bitmap_ptrs1, *bitmap_ptr00, *bitmap_ptr01, *bitmap_mem,
	*bitmap_ptrmem;
BitMap mbm;
Lock bitmap_lock = INIT_LOCK;

void sys_init_bitmaps() {
	_alloc_sum += MAX_BITMAP_PTR_PAGES * 2;
	bitmap_ptrs0 =
		mmap(NULL, PAGE_SIZE * MAX_BITMAP_PTR_PAGES, PROT_READ | PROT_WRITE,
			 MAP_SHARED, _gfd, 2 * PAGE_SIZE);
	bitmap_ptrs1 =
		mmap(NULL, PAGE_SIZE * MAX_BITMAP_PTR_PAGES, PROT_READ | PROT_WRITE,
			 MAP_SHARED, _gfd, (MAX_BITMAP_PTR_PAGES + 2) * PAGE_SIZE);
	bitmap_ptr00 = fmap(2 * MAX_BITMAP_PTR_PAGES + 2);
	bitmap_ptr01 = fmap(2 * MAX_BITMAP_PTR_PAGES + 3);

	bitmap_init(&sbm0, MAX_BITMAP_PTR_PAGES, bitmap_ptrs0);
	bitmap_init(&sbm1, MAX_BITMAP_PTR_PAGES, bitmap_ptrs1);
	bitmap_extend(&sbm0, bitmap_ptr00);
	bitmap_extend(&sbm1, bitmap_ptr01);

	bitmap_mem = map(MAX_BITMAP_PTR_PAGES);
	bitmap_init(&mbm, MAX_BITMAP_PTR_PAGES, bitmap_mem);
	bitmap_ptrmem = map(1);
	bitmap_extend(&mbm, bitmap_ptrmem);

	if (super0->seqno > super1->seqno)
		bitmap_sync(&mbm, &sbm0, true);
	else
		bitmap_sync(&mbm, &sbm1, true);

	bitmap_clean(&mbm);
	bitmap_clean(&sbm1);
	bitmap_clean(&sbm0);
}

void sys_check_file() {
	super0 = fmap(0);
	super1 = fmap(1);
	if (super0->version != VERSION) panic("version mismatch");
	if (super1->version != VERSION) panic("version mismatch");
	if (super0->seqno > super1->seqno) {
		if (cstring_compare_n(super0->magic, MAGIC, 7))
			panic("magic mismatch!");
	} else {
		if (cstring_compare_n(super1->magic, MAGIC, 7))
			panic("magic mismatch!");
	}
}

void sys_init_file() {
	if (ftruncate(_gfd, PAGE_SIZE * (5 + MAX_BITMAP_PTR_PAGES * 2)))
		panic("Could not create sys file: insufficient storage");

	super0 = fmap(0);
	super1 = fmap(1);

	super1->version = super0->version = VERSION;
	copy_bytes(super0->magic, MAGIC, 7);
	copy_bytes(super1->magic, MAGIC, 7);

	super0->seqno = 1;
	super1->seqno = 0;
	super0->alternate = 1;
	super1->alternate = 0;
	super0->bitmap = 2;
	super1->bitmap = 2 + MAX_BITMAP_PTR_PAGES;
	super0->root = MAX_BITMAP_PTR_PAGES * 2 + 4;
	super1->root = MAX_BITMAP_PTR_PAGES * 2 + 4;
}

int64 root_block() {
	return super0->root;
}
void *fmap(int64 id) {
	_alloc_sum += 1;
	return mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _gfd,
				id * PAGE_SIZE);
}
void *fview(int64 id) {
	return mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, _gfd, id * PAGE_SIZE);
}

void free_block(int64 id) {
	id -= 5 + MAX_BITMAP_PTR_PAGES * 2;
	lockr(&bitmap_lock);
	bitmap_free(&mbm, id);
	unlock(&bitmap_lock);
}

int64 allocate_block() {
	int64 size;
	lockr(&bitmap_lock);
	size = super0->size;
	int64 alloc = bitmap_allocate(&mbm);
	unlock(&bitmap_lock);
	int ret = alloc;
	if (ret >= 0) ret += 5 + MAX_BITMAP_PTR_PAGES * 2;
	if ((alloc + 1) * PAGE_SIZE > size) {
		lockw(&bitmap_lock);
		if ((alloc + 1) * PAGE_SIZE > super0->size) {
			if (ftruncate(_gfd, PAGE_SIZE * (alloc + 1))) {
				free_block(ret);
				ret = -1;
			} else {
				super0->size = super1->size = (alloc + 1) * PAGE_SIZE;
			}
		}
		unlock(&bitmap_lock);
	}
	return ret;
}

int flush() {
	int ret = 0;
	BitMap *staging, *prod;
	lockw(&bitmap_lock);
	if (super0->seqno > super1->seqno) {
		staging = &sbm1;
		prod = &sbm0;
	} else {
		staging = &sbm0;
		prod = &sbm1;
	}
	// TODO: need to handle case where additional bitmap blocks are used by
	// allocating additional fmap blocks and using bitmap_extend.
	bitmap_sync(staging, &mbm, false);
	bitmap_clean(&mbm);
	unlock(&bitmap_lock);

	if (fsync(_gfd))
		ret = -1;
	else {
		bitmap_sync(prod, staging, false);
		if (staging == &sbm1) {
			super1->seqno += 2;
		} else {
			super0->seqno += 2;
		}
		bitmap_clean(&sbm1);
		bitmap_clean(&sbm0);
	}
	return ret;
}

int init_sys(const char *path) {
	_gfd = open(path, O_DIRECT | O_RDWR);
	bool create = false;
	if (_gfd == -1) {
		create = true;
		_gfd = open(path, O_DIRECT | O_RDWR | O_CREAT, 0600);
	}
	if (_gfd == -1) panic("Could not open file [%s]", path);

	if (create)
		sys_init_file();
	else
		sys_check_file();

#ifdef __APPLE__
	if (fcntl(_gfd, F_NOCACHE, 1))
		panic("Could not disable cache for file [%s]", path);
#endif	// __APPLE__

	sys_init_bitmaps();
	super0->size = super1->size = lseek(_gfd, 0, SEEK_END);

	return 0;
}

int shutdown_sys() {
	if (_gfd != -1) {
		close(_gfd);

		unmap(super0, 1);
		unmap(super1, 1);
		unmap(bitmap_ptrs0, MAX_BITMAP_PTR_PAGES);
		unmap(bitmap_ptrs1, MAX_BITMAP_PTR_PAGES);
		unmap(bitmap_ptr00, 1);
		unmap(bitmap_ptr01, 1);
		unmap(bitmap_mem, MAX_BITMAP_PTR_PAGES);
		unmap(bitmap_ptrmem, 1);
	}
	return 0;
}
