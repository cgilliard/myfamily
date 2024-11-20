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

#include <base/block.h>
#include <base/macros.h>
#include <base/map.h>
#include <base/print_util.h>
#include <base/util.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

// for macos
#ifndef O_DIRECT
#define O_DIRECT 0
#endif	// O_DIRECT

#define VERSION '\0'
#define MAGIC "^l337*Z"

typedef struct SuperBlock {
	byte version;
	byte magic[7];
	unsigned long long seqno;
	int64 alternate;
	int64 bitmap;
	int64 root;
	byte padding[];
} SuperBlock;

typedef struct Bits {
	int64 next;
	byte bits[];
} Bits;

int _gfd = -1;
byte *_test_dir = NULL;
Bits *mem_bits = NULL;

void create_blocks() {
	if (ftruncate(_gfd, 5 * PAGE_SIZE))
		panic("Could not reserve enough space for block file: %s",
			  strerror(errno));
	SuperBlock *block0 = fmap(0);
	SuperBlock *block1 = fmap(1);
	block1->version = block0->version = VERSION;
	copy_bytes(block0->magic, MAGIC, 7);
	copy_bytes(block1->magic, MAGIC, 7);

	block0->seqno = 0;
	block1->seqno = 1;
	block0->alternate = 1;
	block1->alternate = 0;
	block0->bitmap = 2;
	block1->bitmap = 3;
	block0->root = 4;
	block1->root = 4;

	unmap(block0, 1);
	unmap(block1, 1);
}

void check_blocks() {
}

SuperBlock *block0, *block1, *sb, *staging_sb;

void block_init_bits(SuperBlock *dst, SuperBlock *src) {
	int64 src_itt = src->bitmap;
	int64 dst_itt = dst->bitmap;
	Bits *mem_bits_itt = mem_bits = NULL;
	while (src_itt) {
		Bits *src_bits = fmap(src_itt);
		Bits *dst_bits = fmap(dst_itt);
		Bits *mem_bits_itt_next = map(1);
		if (mem_bits_itt) mem_bits_itt->next = (int64)mem_bits_itt_next;
		mem_bits_itt = mem_bits_itt_next;
		if (mem_bits == NULL) mem_bits = mem_bits_itt;
		copy_bytes((byte *)dst_bits + sizeof(int64),
				   (byte *)src_bits + sizeof(int64), PAGE_SIZE - sizeof(int64));
		copy_bytes((byte *)mem_bits_itt + sizeof(int64),
				   (byte *)src_bits + sizeof(int64), PAGE_SIZE - sizeof(int64));
		src_itt = src_bits->next;
		dst_itt = dst_bits->next;
		unmap(src_bits, 1);
		unmap(dst_bits, 1);
	}
}

void init_blocks() {
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	int len;

	if (_test_dir)
		len = cstring_len(_test_dir);
	else {
		len = cstring_len(homedir);
	}

	char file[len + 10];

	if (_test_dir) {
		copy_bytes(file, _test_dir, len);
		copy_bytes(file + len, "/.fam.dat", 9);
		file[len + 9] = 0;
	} else {
		copy_bytes(file, homedir, len);
		copy_bytes(file + len, "/.fam.dat", 9);
		file[len + 9] = 0;
	}
	_gfd = open(file, O_DIRECT | O_RDWR);
	if (_gfd == -1) {
		_gfd = open(file, O_DIRECT | O_RDWR | O_CREAT, 0600);
		if (_gfd == -1)
			panic("Could not create block file: %s", strerror(errno));
		create_blocks();
	} else
		check_blocks();

	if (O_DIRECT == 0) {
#ifdef __APPLE__
		if (fcntl(_gfd, F_NOCACHE, 1)) panic("Could not disable cache!");
#endif	// __APPLE__
	}

	block0 = fmap(0);
	block1 = fmap(1);
	if (block0->seqno > block1->seqno) {
		ASTORE(&sb, block0);
		ASTORE(&staging_sb, block1);
		block_init_bits(block1, block0);
	} else {
		ASTORE(&sb, block1);
		ASTORE(&staging_sb, block0);
		block_init_bits(block0, block1);
	}

	flush();
}

void destroy_blocks() {
	if (_gfd != -1) {
		while (mem_bits) {
			void *next = (void *)mem_bits->next;
			unmap(mem_bits, 1);
			mem_bits = next;
		}
		unmap(block0, 1);
		unmap(block1, 1);
		close(_gfd);
	}
}

#ifndef TEST
void __attribute__((constructor)) __init_blocks() {
	init_blocks();
}
void __attribute__((destructor)) __close_blocks() {
	destroy_blocks();
}
#endif	// TEST

int64 root_block() {
	return sb->root;
}

int64 allocate_block() {
	return 0;
}
void free_block(int64 id) {
}

int flush() {
	fsync(_gfd);
	return 0;
}

void *fmap(int64 id) {
#ifdef TEST
	_alloc_sum += 1;
#endif	// TEST
	return mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _gfd,
				id * PAGE_SIZE);
}

void *fview(int64 id) {
#ifdef TEST
	_alloc_sum += 1;
#endif	// TEST
	return mmap(NULL, PAGE_SIZE, PROT_READ, MAP_SHARED, _gfd, id * PAGE_SIZE);
}
