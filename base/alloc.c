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

#include <base/alloc.h>
#include <base/bitmap.h>
#include <base/err.h>
#include <base/lock.h>
#include <base/macros.h>
#include <base/map.h>
#include <base/print_util.h>

#define SHIFT_PER_LEVEL (__builtin_ctz(PAGE_SIZE) - 3)
#define MASK ((1U << (SHIFT_PER_LEVEL)) - 1)
#define KOFF (SHIFT_PER_LEVEL)
#define JOFF (2 * SHIFT_PER_LEVEL)
#define IOFF (3 * SHIFT_PER_LEVEL)

typedef struct AllocImpl {
	byte ****data;
	BitMap bm;
	unsigned int size;
	Lock lock;
	byte padding[4];
} AllocImpl;

void __attribute__((constructor)) __check_alloc_sizes() {
	if (sizeof(AllocImpl) != sizeof(Alloc))
		panic("sizeof(AllocImpl) (%i) != sizeof(Alloc) (%i)", sizeof(AllocImpl),
			  sizeof(Alloc));
}

byte *alloc_data(AllocImpl *impl, Ptr ptr) {
	unsigned long long next = ptr >> (__builtin_ctz(PAGE_SIZE / impl->size));
	unsigned int entries = ((1U << (__builtin_ctz(PAGE_SIZE / impl->size))));
	unsigned long long offset = (ptr % entries) * impl->size;
	unsigned long long i = next >> (2 * SHIFT_PER_LEVEL);
	unsigned long long j = (next >> SHIFT_PER_LEVEL) & MASK;
	unsigned long long k = next & MASK;

	lockr(&impl->lock);

	if (impl->data == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data == NULL) {
			impl->data = (byte ****)map(1);
			if (impl->data == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return NULL;
			}
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}
	if (impl->data[i] == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i] == NULL) {
			impl->data[i] = (byte ***)map(1);
			if (impl->data[i] == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return NULL;
			}
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}

	if (impl->data[i][j] == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i][j] == NULL) {
			impl->data[i][j] = (byte **)map(1);
			if (impl->data[i][j] == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return NULL;
			}
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}
	if (impl->data[i][j][k] == NULL) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i][j][k] == NULL) {
			impl->data[i][j][k] = (byte *)map(1);
			if (impl->data[i][j][k] == NULL) {
				unlock(&impl->lock);
				SetErr(AllocErr);
				return NULL;
			}
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}

	unlock(&impl->lock);
	return impl->data[i][j][k] + offset;
}

int alloc_init(Alloc *a, unsigned int size, unsigned int max_slabs) {
	if (size >= PAGE_SIZE || size == 0 || PAGE_SIZE % size != 0 ||
		max_slabs == 0 ||
		(max_slabs != UINT32_MAX && max_slabs % (PAGE_SIZE * 8) != 0)) {
		SetErr(IllegalArgument);
		return -1;
	}
	AllocImpl *impl = (AllocImpl *)a;
	unsigned long long max_slabs_long;
	if (max_slabs == UINT32_MAX) {
		max_slabs_long = (unsigned long long)max_slabs + 1ULL;
	} else
		max_slabs_long = max_slabs;
	if (bitmap_init(&impl->bm, max_slabs_long / (PAGE_SIZE * 8))) return -1;
	impl->data = NULL;
	impl->lock = INIT_LOCK;
	impl->size = size;
	return 0;
}

Slab alloc(Alloc *a) {
	Slab ret = {};
	AllocImpl *impl = (AllocImpl *)a;
	int64 ptr = bitmap_allocate(&impl->bm);
	if (ptr < 0) return ret;
	ret.data = alloc_data(impl, ptr);
	if (ret.data == NULL) return ret;
	ret.ptr = ptr;
	return ret;
}

void release(Alloc *a, Ptr ptr) {
	AllocImpl *impl = (AllocImpl *)a;
	bitmap_free(&impl->bm, ptr);
}

void alloc_cleanup(Alloc *a) {
	AllocImpl *impl = (AllocImpl *)a;
	bitmap_cleanup(&impl->bm);
	if (impl->data) {
		for (int i = 0; i < PAGE_SIZE / sizeof(byte *) && impl->data[i]; i++) {
			for (int j = 0; j < PAGE_SIZE / sizeof(byte *) && impl->data[i][j];
				 j++) {
				for (int k = 0;
					 k < PAGE_SIZE / sizeof(byte *) && impl->data[i][j][k]; k++)
					unmap((byte *)impl->data[i][j][k], 1);
				unmap((byte *)impl->data[i][j], 1);
			}
			unmap((byte *)impl->data[i], 1);
		}
		unmap((byte *)impl->data, 1);
	}
}
