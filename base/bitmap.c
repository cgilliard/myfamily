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
#define BITS_LEN ((PAGE_SIZE - sizeof(BitMapBits)) / sizeof(uint64))

int bitmap_index;

typedef struct BitMapCtx {
	int64 index;
} BitMapCtx;

_Thread_local BitMapCtx bitmap_ctx[MAX_BITMAPS] = {};

typedef struct BitMapBits {
	bool dirty;
	uint64 bits[];
} BitMapBits;

typedef struct BitMapImpl {
	BitMapBits **ptrs;
	int64 *fptrs;
	int64 ptr_count;
	Lock lock;
	int64 bitmap_ptr_pages;
	int64 index;
} BitMapImpl;

void __attribute__((constructor)) __check_bitmap_sizes() {
	ASTORE(&bitmap_index, 0);
	if (sizeof(BitMapImpl) != sizeof(BitMap))
		panic("sizeof(BitMapImpl) (%i) != sizeof(BitMap) (%i)",
			  sizeof(BitMapImpl), sizeof(BitMap));
}

int bitmap_init(BitMap *m, int bitmap_ptr_pages, void *ptrs, int64 *fptrs) {
	BitMapImpl *impl = (BitMapImpl *)m;

	impl->ptrs = ptrs;
	impl->fptrs = fptrs;
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
	BitMapBits *cur = impl->ptrs[ctx->index / BITS_LEN];
	while (cur) {
		do {
			initial = ALOAD(&cur->bits[ctx->index % BITS_LEN]);
			found = initial != ~0ULL;
			if (!found) break;
			x = __builtin_ctzl(~(initial));
			updated = initial | (0x1ULL << x);
		} while (!CAS(&cur->bits[ctx->index % BITS_LEN], &initial, updated));
		if (found) {
			ASTORE(&cur->dirty, true);
			return (ctx->index << 6) | x;
		}
		if (++(ctx->index) % BITS_LEN == 0)
			cur = impl->ptrs[ctx->index / BITS_LEN];
	}
	return -1;
}

void bitmap_free(BitMap *m, unsigned long long index) {
	BitMapImpl *impl = (BitMapImpl *)m;
	BitMapCtx *ctx = &bitmap_ctx[impl->index];

	uint64 x = 1 << (index & 0x3F);
	index >>= 6;

	if (index < ctx->index) ctx->index = index;

	BitMapBits *cur = impl->ptrs[index / BITS_LEN];
	uint64 initial, updated;

	do {
		initial = ALOAD(&cur->bits[index % BITS_LEN]);
		updated = initial & ~x;
		if (updated == initial)
			panic("Double free attempt on index=%lli! [%llx %llx]", index,
				  initial, updated);

	} while (!CAS(&cur->bits[index % BITS_LEN], &initial, updated));
}

int bitmap_sync(BitMap *dst, BitMap *src, bool all) {
	int ret = 0;
	BitMapImpl *dst_impl = (BitMapImpl *)dst;
	BitMapImpl *src_impl = (BitMapImpl *)src;

	for (int64 i = 0; i < src_impl->ptr_count; i++) {
		if (all || src_impl->ptrs[i]->dirty) {
			if (i >= dst_impl->ptr_count) {
				SetErr(CapacityExceeded);
				ret = -1;
				break;
			}
			copy_bytes((byte *)dst_impl->ptrs[i]->bits,
					   (byte *)src_impl->ptrs[i]->bits,
					   PAGE_SIZE - sizeof(BitMapBits));
			dst_impl->ptrs[i]->dirty = true;
		}
	}
	return ret;
}

void bitmap_clean(BitMap *m) {
	BitMapBits *next, *cur = ((BitMapImpl *)m)->ptrs[0];
	int i = 0;
	while (cur) {
		cur->dirty = false;
		next = ((BitMapImpl *)m)->ptrs[++i];
		cur = next;
	}
}

int64 bitmap_ptr_count(BitMap *m) {
	return ((BitMapImpl *)m)->ptr_count;
}

int bitmap_extend(BitMap *m, void *ptr, int64 v) {
	BitMapImpl *impl = (BitMapImpl *)m;
	BitMapCtx *ctx = &bitmap_ctx[impl->index];
	if ((ctx->index / BITS_LEN) + 1 >= (PAGE_SIZE / 8) * impl->bitmap_ptr_pages)
		return -1;

	impl->ptrs[impl->ptr_count] = ptr;
	if (v >= 0) impl->fptrs[impl->ptr_count] = v;
	impl->ptr_count++;

	return 0;
}
