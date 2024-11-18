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

int64 bitmap_try_allocate(BitMapImpl *impl) {
	unsigned long long itt = bitmap_last_index[impl->bitmap_index];
	loop {
		if (itt >= (impl->cur_blocks * PAGE_SIZE) / 8) break;
		unsigned long long *ptr =
			impl->data[itt >> 24][(itt >> 16) & 0xFF][(itt >> 8) & 0xFF];

		if (*ptr != ~0) {
			int x;
			for (x = 0; (*ptr & (0x1ULL << x)) != 0; x++);
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
	unsigned long long next = impl->cur_blocks + 1;
	unsigned long long i = next >> 24;
	unsigned long long j = (next >> 16) & 0xFF;
	unsigned long long k = (next >> 8) & 0xFF;

	if (impl->data == NULL) {
	}
	if (impl->data[i] == NULL) {
	}
	if (impl->data[i][j] == NULL) {
	}
	if (impl->data[i][j][k] == NULL) {
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
}
