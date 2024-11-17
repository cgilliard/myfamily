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

#ifndef _CORE_BPLUS__
#define _CORE_BPLUS__

#include <base/types.h>

#define BLOCK_SIZE 4096
typedef struct Block {
	unsigned long long next;
	byte data[];
} Block;

#define SIZE_OF_BLUS_TXN_IMPL 8
typedef struct BPlusTxn {
	byte impl[SIZE_OF_BLUS_TXN_IMPL];
} BPlusTxn;

#define DEFAULT_BPLUS_TXN_CONFIG {.timeout_millis = 1000}
typedef struct BPlusTxnConfig {
	int64 timeout_millis;
} BPlusTxnConfig;

#define SIZE_OF_BPLUS_IMPL 24
typedef struct BPlus {
	byte impl[SIZE_OF_BPLUS_IMPL];
} BPlus;

int bplus_open(BPlus *b, const char *path);
int bplus_close(BPlus *b);

unsigned long long bplus_block_count(BPlus *b);

BPlusTxn bplus_txn_create(BPlus *b, const BPlusTxnConfig config);
int bplus_txn_commit(BPlusTxn *txn);
int bplus_txn_cancel(BPlusTxn *txn);

const Block *bplus_get(const BPlusTxn *txn, const void *key,
					   unsigned long long len, int64 range_offset);
const Block *bplus_put(BPlusTxn *txn, const void *key,
					   unsigned long long keylen, const void *value,
					   unsigned long long valuelen);
const Block *bplus_remove(BPlusTxn *txn, const void *key,
						  unsigned long long len);

const Block *bplus_block_next(const BPlusTxn *txn, const Block *block);

#endif	// _CORE_BPLUS__
