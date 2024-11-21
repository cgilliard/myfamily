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

#define MAX_BITMAPS 256
#define BITS_LEN ((PAGE_SIZE - sizeof(BitMapBits)) / sizeof(uint64))

int bitmap_index = 0;
_Thread_local int64 bitmap_last_index[MAX_BITMAPS] = {};

typedef struct BitMapBits {
	struct BitMapBits *next;
	uint64 bits[];
} BitMapBits;

typedef struct BitMapImpl {
	BitMapBits **ptrs;
	int64 ptr_count;
	Lock lock;
	int bitmap_ptr_pages;
	int index;
} BitMapImpl;

void __attribute__((constructor)) __check_bitmap_sizes() {
	if (sizeof(BitMapImpl) != sizeof(BitMap))
		panic("sizeof(BitMapImpl) (%i) != sizeof(BitMap) (%i)",
			  sizeof(BitMapImpl), sizeof(BitMap));
}

int bitmap_try_resize(BitMapImpl *impl, int64 ptr_count) {
	lockw(&impl->lock);
	if (ptr_count != impl->ptr_count) {
		unlock(&impl->lock);
		return 0;
	}

	if (ptr_count + 1 >= (PAGE_SIZE / 8) * impl->bitmap_ptr_pages) {
		unlock(&impl->lock);
		return -1;
	}
	if (impl->ptrs[ptr_count] == NULL) {
		impl->ptrs[impl->ptr_count] = map(1);
		if (impl->ptrs[impl->ptr_count] == NULL) {
			unlock(&impl->lock);
			return -1;
		}
		impl->ptrs[impl->ptr_count - 1]->next = impl->ptrs[impl->ptr_count];
		impl->ptr_count++;
	}
	unlock(&impl->lock);
	return 0;
}

int bitmap_init(BitMap *m, int bitmap_ptr_pages) {
	BitMapImpl *impl = (BitMapImpl *)m;

	impl->ptrs = map(bitmap_ptr_pages);
	if (impl->ptrs == NULL) {
		return -1;
	}

	impl->ptrs[0] = map(1);
	if (impl->ptrs[0] == NULL) {
		unmap(impl->ptrs, bitmap_ptr_pages);
		return -1;
	}

	impl->bitmap_ptr_pages = bitmap_ptr_pages;
	impl->ptr_count = 1;
	impl->lock = INIT_LOCK;
	impl->index = AADD(&bitmap_index, 1);
	if (impl->index >= MAX_BITMAPS) {
		unmap(impl->ptrs[0], 1);
		unmap(impl->ptrs, bitmap_ptr_pages);
		SetErr(CapacityExceeded);
		return -1;
	}

	bitmap_last_index[impl->index] = 0;

	return 0;
}

int64 bitmap_try_allocate(BitMapImpl *impl) {
	bool found;
	uint64 updated, initial, index = bitmap_last_index[impl->index], x = 0;
	BitMapBits *cur = impl->ptrs[index / BITS_LEN];
	while (cur) {
		do {
			initial = ALOAD(&cur->bits[index % BITS_LEN]);
			if (initial != ~0ULL) {
				found = true;
				x = __builtin_ctzl(~(initial));
				updated = initial | (0x1ULL << x);
			} else {
				found = false;
				break;
			}
		} while (!CAS(&cur->bits[index % BITS_LEN], &initial, updated));
		if (found) {
			bitmap_last_index[impl->index] = index;
			return (index << 6) | x;
		}
		if (++index % BITS_LEN == 0) cur = cur->next;
	}

	bitmap_last_index[impl->index] = index;
	return -1;
}

int64 bitmap_allocate(BitMap *m) {
	BitMapImpl *impl = (BitMapImpl *)m;
	int64 ptr_count = ALOAD(&impl->ptr_count);
	int64 ret = bitmap_try_allocate(impl);
	if (ret >= 0)
		return ret;
	else {
		if (bitmap_try_resize(impl, ptr_count)) {
			if (ptr_count + 1 >= (PAGE_SIZE / 8) * impl->bitmap_ptr_pages) {
				bitmap_last_index[impl->index] = 0;
				return bitmap_try_allocate(impl);
			}
			return -1;
		}
		return bitmap_try_allocate(impl);
	}
}

void bitmap_free(BitMap *m, uint64 index) {
	BitMapImpl *impl = (BitMapImpl *)m;

	uint64 x = 1 << (index & 0x3F);
	index >>= 6;

	if (index < bitmap_last_index[impl->index]) {
		bitmap_last_index[impl->index] = index;
	}

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

void bitmap_cleanup(BitMap *m) {
	BitMapBits *next, *cur = ((BitMapImpl *)m)->ptrs[0];
	while (cur) {
		next = cur->next;
		unmap(cur, 1);
		cur = next;
	}

	unmap(((BitMapImpl *)m)->ptrs, ((BitMapImpl *)m)->bitmap_ptr_pages);
}
