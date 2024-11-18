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
#include <base/map.h>
#include <base/print_util.h>
#include <base/util.h>

#define MAX_BITMAPS 1024

int bitmap_index = 0;
_Thread_local unsigned long long bitmap_last_index[MAX_BITMAPS] = {};

typedef struct BitMapImpl {
	unsigned long long ****data;
	unsigned long long cur_blocks;
	unsigned long long max_blocks;
	unsigned long long bitmap_index;
	Lock lock;
	byte padding[8];
} BitMapImpl;

void __attribute__((constructor)) __check_bitmap_sizes() {
	if (sizeof(BitMapImpl) != sizeof(BitMap))
		panic("sizeof(BitMapImpl) (%i) != sizeof(BitMap) (%i)",
			  sizeof(BitMapImpl), sizeof(BitMap));
}

#define SHIFT_PER_LEVEL (__builtin_ctz(PAGE_SIZE) - 3)
#define MASK ((1U << (SHIFT_PER_LEVEL)) - 1)
#define KOFF (SHIFT_PER_LEVEL)
#define JOFF (2 * SHIFT_PER_LEVEL)
#define IOFF (3 * SHIFT_PER_LEVEL)

#define DATA(impl, index)                              \
	(impl->data[index >> IOFF][(index >> JOFF) & MASK] \
			   [(index >> KOFF) & MASK] +              \
	 (index & MASK))

int64 bitmap_try_allocate(BitMapImpl *impl) {
	bool has_looped = false;
	unsigned long long itt = bitmap_last_index[impl->bitmap_index];
	loop {
		if (itt >= (impl->cur_blocks * PAGE_SIZE) / 8) {
			if (impl->cur_blocks == impl->max_blocks) {
				// try one more loop
				if (!has_looped) {
					has_looped = true;
					itt = 0;
				} else
					break;
			} else
				break;
		}
		unsigned long long initial, updated;
		int x;
		bool found;

		do {
			initial = ALOAD(DATA(impl, itt));
			if (initial == ~0ULL) {
				found = false;
				break;
			}
			found = true;
			x = __builtin_ctzl(~(initial));
			updated = initial | (0x1ULL << x);

		} while (!CAS_SEQ(DATA(impl, itt), &initial, updated));

		if (found) {
			bitmap_last_index[impl->bitmap_index] = itt;
			return (itt << 6) | x;
		}

		itt++;
	}
	bitmap_last_index[impl->bitmap_index] = itt;
	return -1;
}

int bitmap_try_resize(BitMapImpl *impl) {
	if (impl->cur_blocks == impl->max_blocks) return -1;

	lockr(&impl->lock);
	unsigned long long next = (impl->cur_blocks + 0) * (PAGE_SIZE / 8);
	unsigned long long i = next >> IOFF;
	unsigned long long j = (next >> JOFF) & MASK;
	unsigned long long k = (next >> KOFF) & MASK;

	if (impl->data == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data == NULL) {
			impl->data = (unsigned long long ****)map(1);
			if (impl->data == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return -1;
			}
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}
	if (impl->data[i] == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i] == NULL) {
			impl->data[i] = (unsigned long long ***)map(1);
			if (impl->data[i] == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return -1;
			}
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}
	if (impl->data[i][j] == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i][j] == NULL) {
			impl->data[i][j] = (unsigned long long **)map(1);
			if (impl->data[i][j] == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return -1;
			}
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}
	if (impl->data[i][j][k] == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i][j][k] == NULL) {
			impl->data[i][j][k] = (unsigned long long *)map(1);
			if (impl->data[i][j][k] == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return -1;
			}
		}
		impl->cur_blocks++;
		unlock(&impl->lock);
		lockr(&impl->lock);
	}

	unlock(&impl->lock);

	return 0;
}

int bitmap_init(BitMap *m, unsigned long long max_blocks) {
	BitMapImpl *impl = (BitMapImpl *)m;
	impl->cur_blocks = 0;
	impl->max_blocks = max_blocks;
	impl->lock = INIT_LOCK;
	impl->bitmap_index = AADD(&bitmap_index, 1);
	impl->data = NULL;
	bitmap_last_index[impl->bitmap_index] = 0;
	if (impl->bitmap_index >= MAX_BITMAPS) {
		SetErr(CapacityExceeded);
		return -1;
	}
	return 0;
}

int64 bitmap_allocate(BitMap *m) {
	BitMapImpl *impl = (BitMapImpl *)m;

	unsigned long long ret = bitmap_try_allocate(impl);
	if (ret == -1) {
		if (bitmap_try_resize(impl)) return -1;
		return bitmap_try_allocate(impl);
	} else
		return ret;
}

void bitmap_free(BitMap *m, unsigned long long index) {
	BitMapImpl *impl = (BitMapImpl *)m;
	int bit = index & 0x7F;
	index >>= 6;

	unsigned long long initial, updated;
	do {
		initial = ALOAD(DATA(impl, index));
		updated = initial & ~(0x1ULL << bit);
	} while (!CAS_SEQ(DATA(impl, index), &initial, updated));

	if (index < bitmap_last_index[impl->bitmap_index])
		bitmap_last_index[impl->bitmap_index] = index;
}
