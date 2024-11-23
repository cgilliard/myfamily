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

#define MAX_BITMAPS 256
#define BITS_LEN (PAGE_SIZE / sizeof(uint64))

int bitmap_index;

typedef struct BitMapCtx {
	int64 index;
} BitMapCtx;

_Thread_local BitMapCtx bitmap_ctx[MAX_BITMAPS] = {};

typedef struct BitMapImpl {
	int64 **ptrs;
	int64 ptr_count;
	Lock lock;
	int bitmap_ptr_pages;
	int index;
} BitMapImpl;

void __attribute__((constructor)) __check_bitmap_sizes() {
	ASTORE(&bitmap_index, 0);
	if (sizeof(BitMapImpl) != sizeof(BitMap))
		panic("sizeof(BitMapImpl) (%i) != sizeof(BitMap) (%i)",
			  sizeof(BitMapImpl), sizeof(BitMap));
}

int bitmap_init(BitMap *m, int bitmap_ptr_pages, void *ptrs) {
	BitMapImpl *impl = (BitMapImpl *)m;

	impl->ptrs = ptrs;
	impl->ptr_count = 0;
	impl->lock = INIT_LOCK;
	impl->bitmap_ptr_pages = bitmap_ptr_pages;
	impl->index = AADD(&bitmap_index, 1);

	if (impl->index >= MAX_BITMAPS) {
		SetErr(CapacityExceeded);
		return -1;
	}

	bitmap_ctx[impl->index].index = 0;

	return 0;
}

int64 bitmap_allocate(BitMap *m) {
	BitMapImpl *impl = (BitMapImpl *)m;
	BitMapCtx *ctx = &bitmap_ctx[impl->index];

	bool found;
	uint64 updated, initial, x;
	int64 *cur = impl->ptrs[ctx->index / BITS_LEN];
	while (cur) {
		do {
			initial = ALOAD(&cur[ctx->index % BITS_LEN]);
			found = initial != ~0ULL;
			if (!found) break;
			x = __builtin_ctzl(~(initial));
			updated = initial | (0x1ULL << x);
		} while (!CAS(&cur[ctx->index % BITS_LEN], &initial, updated));
		if (found) {
			return (ctx->index << 6) | x;
		}
		if (++(ctx->index) % BITS_LEN == 0)
			cur = impl->ptrs[ctx->index / BITS_LEN];
	}
	return -1;
}

void bitmap_free(BitMap *m, uint64 index) {
	BitMapImpl *impl = (BitMapImpl *)m;
	BitMapCtx *ctx = &bitmap_ctx[impl->index];

	uint64 x = 1 << (index & 0x3F);
	index >>= 6;

	if (index < ctx->index) ctx->index = index;

	int64 *cur = impl->ptrs[index / BITS_LEN];
	uint64 initial, updated;

	do {
		initial = ALOAD(&cur[index % BITS_LEN]);
		updated = initial & ~x;
		if (updated == initial)
			panic("Double free attempt on index=%lli! [%llx %llx]", index,
				  initial, updated);

	} while (!CAS(&cur[index % BITS_LEN], &initial, updated));
}

void bitmap_cleanup(BitMap *m) {
	int64 *cur = ((BitMapImpl *)m)->ptrs[0];
	int i = 0;
	while (cur) {
		unmap(cur, 1);
		cur = ((BitMapImpl *)m)->ptrs[++i];
	}
	unmap(((BitMapImpl *)m)->ptrs, ((BitMapImpl *)m)->bitmap_ptr_pages);
}

int64 bitmap_ptr_count(BitMap *m) {
	return ((BitMapImpl *)m)->ptr_count;
}

int bitmap_extend(BitMap *m, void *ptr) {
	BitMapImpl *impl = (BitMapImpl *)m;
	BitMapCtx *ctx = &bitmap_ctx[impl->index];
	if (impl->ptr_count + 1 >= (PAGE_SIZE / 8) * impl->bitmap_ptr_pages)
		return -1;

	impl->ptrs[impl->ptr_count] = ptr;
	impl->ptr_count++;

	return 0;
}
