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
#include <base/print_util.h>
#include <base/util.h>
#include <core/bplus.h>
#include <core/orbtree.h>
#include <crypto/murmurhash.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define VERSION '0'
#define MAGIC "%^&x"
#define MURMURHASH_SEED 1
#define MIN_UNUSED_CAPACITY 64
#define CAPACITY_OFFSET 8
#define MAX_USED_OFFSET 16
#define BLOCK_COUNT_OFFSET 24
#define CURRENT_TXN_ID 32

#define GET_CUR_CAPACITY(impl) \
	(*(unsigned long long *)(impl->header + CAPACITY_OFFSET))
#define GET_CUR_MAX_USED(impl) \
	(*(unsigned long long *)(impl->header + MAX_USED_OFFSET))
#define GET_BLOCK_COUNT(impl) \
	(*(unsigned long long *)(impl->header + BLOCK_COUNT_OFFSET))
#define SET_CAPACITY(impl, value) \
	((*(unsigned long long *)(impl->header + CAPACITY_OFFSET)) = value)
#define SET_MAX_USED(impl, value) \
	((*(unsigned long long *)(impl->header + MAX_USED_OFFSET)) = value)
#define SET_BLOCK_COUNT(impl, value) \
	(*(unsigned long long *)(impl->header + BLOCK_COUNT_OFFSET)) = value)

typedef struct BPlusTxnImpl {
	BPlusTxnConfig config;
} BPlusTxnImpl;

typedef struct BPlusImpl {
	int fd;
	byte *header;
	OrbTree txns;
} BPlusImpl;

static void __attribute__((constructor)) _check_sizes() {
	if (sizeof(BPlus) != sizeof(BPlusImpl))
		panic("sizeof(BPlus) (%i) != sizeof(BPlusImpl) (%i)", sizeof(BPlus),
			  sizeof(BPlusImpl));
	if (sizeof(BPlusTxn) != sizeof(BPlusTxnImpl))
		panic("sizeof(BPlusTxn) (%i) != sizeof(BPlusTxnImpl) (%i)",
			  sizeof(BPlusTxn), sizeof(BPlusTxnImpl));
}

static int bplus_create_path(const char *path) {
	int fd = open(path, O_CREAT | O_RDWR, 0600);
	return fd;
}

static void set_checksum(BPlusImpl *impl) {
	unsigned int h = murmurhash(impl->header, BLOCK_SIZE - 4, MURMURHASH_SEED);
	copy_bytes(impl->header + (BLOCK_SIZE - 4), (byte *)&h, 4);
}

static int bplus_validate_checksum(BPlusImpl *impl) {
	unsigned int h = murmurhash(impl->header, BLOCK_SIZE - 4, MURMURHASH_SEED);
	if (cstring_compare_n(impl->header + (BLOCK_SIZE - 4), (byte *)&h, 4)) {
		SetErr(FileCorrupted);
		return -1;
	}
	return 0;
}

static int bplus_verify_file(BPlusImpl *impl) {
	impl->header =
		mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, impl->fd, 0);

	if (impl->header == MAP_FAILED) {
		SetErr(IO);
		return -1;
	}
	if (impl->header[0] != VERSION) {
		SetErr(UnsupportedVersion);
		return -1;
	}
	if (cstring_compare_n(impl->header + 1, MAGIC, 4)) {
		SetErr(FileCorrupted);
		return -1;
	}

	struct stat st;
	fstat(impl->fd, &st);
	unsigned long long size = st.st_size;

	// adjust size (if it doesn't match it means the system failed before
	// capacity could be updated) our only option is to roll back to the file
	// size before the ftruncate was presumably called. This is ok because
	// update_min_capacity never completed. So we're at the value before it was
	// called.
	if ((GET_CUR_CAPACITY(impl) + 1) * (BLOCK_SIZE) != size) {
		if (ftruncate(impl->fd, (GET_CUR_CAPACITY(impl) + 1) * (BLOCK_SIZE))) {
			SetErr(IO);
			return -1;
		}
	}

	return bplus_validate_checksum(impl);
}

static int update_min_capacity(BPlusImpl *impl) {
	unsigned long long cur_capacity = GET_CUR_CAPACITY(impl);
	unsigned long long cur_max = GET_CUR_MAX_USED(impl);
	if (cur_capacity != cur_max + MIN_UNUSED_CAPACITY) {
		if (ftruncate(impl->fd,
					  (1 + cur_max + MIN_UNUSED_CAPACITY) * (BLOCK_SIZE))) {
			SetErr(IO);
			return -1;
		}
		SET_CAPACITY(impl, cur_max + MIN_UNUSED_CAPACITY);
	}

	return 0;
}

static int bplus_init_file(BPlusImpl *impl, const char *path) {
	if (ftruncate(impl->fd, BLOCK_SIZE)) {
		SetErr(IO);
		return -1;
	}
	impl->header =
		mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, impl->fd, 0);
	if (impl->header == MAP_FAILED) {
		SetErr(IO);
		return -1;
	}

	impl->header[0] = VERSION;
	copy_bytes(impl->header + 1, MAGIC, cstring_len(MAGIC));
	set_bytes(impl->header + 1 + cstring_len(MAGIC), '\0',
			  BLOCK_SIZE - (1 + cstring_len(MAGIC)));

	if (update_min_capacity(impl)) {
		unlink(path);
		return -1;
	}

	set_checksum(impl);

	if (msync(impl->header, BLOCK_SIZE, MS_SYNC) == -1) {
		unlink(path);
		SetErr(IO);
		return -1;
	}

	return 0;
}

int bplus_open(BPlus *b, const char *path) {
	BPlusImpl *impl = (BPlusImpl *)b;
	int fd;
	bool is_new = false;
	if ((fd = open(path, O_RDWR)) == -1) {
		is_new = true;
		if ((fd = open(path, O_CREAT | O_RDWR, 0600)) == -1) {
			SetErr(FileNotFound);
			return -1;
		}
	}
	impl->txns = INIT_ORBTREE;
	impl->fd = fd;
	if (is_new) {
		if (bplus_init_file(impl, path)) return -1;
	} else {
		if (bplus_verify_file(impl)) return -1;
	}
	return 0;
}
int bplus_close(BPlus *b) {
	BPlusImpl *impl = (BPlusImpl *)b;
	if (munmap(impl->header, BLOCK_SIZE)) {
		SetErr(IO);
		return -1;
	}
	return close(impl->fd);
}

unsigned long long bplus_block_count(BPlus *b) {
	BPlusImpl *impl = (BPlusImpl *)b;
	return GET_BLOCK_COUNT(impl);
}

BPlusTxn bplus_txn_create(BPlus *b, const BPlusTxnConfig config) {
	BPlusTxnImpl txn = {.config = config};
	return *((BPlusTxn *)&txn);
}
int bplus_txn_commit(BPlusTxn *txn) {
	return 0;
}
int bplus_txn_cancel(BPlusTxn *txn) {
	return 0;
}

const Block *bplus_get(const BPlusTxn *txn, const void *key,
					   unsigned long long len, int64 range_offset) {
	return NULL;
}
const Block *bplus_put(BPlusTxn *txn, const void *key,
					   unsigned long long keylen, const void *value,
					   unsigned long long valuelen) {
	return NULL;
}
const Block *bplus_remove(BPlusTxn *txn, const void *key,
						  unsigned long long len) {
	return NULL;
}

const Block *bplus_block_next(const BPlusTxn *txn, const Block *block) {
	return NULL;
}
