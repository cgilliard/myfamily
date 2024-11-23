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
#include <base/slabs.h>
#include <base/sys.h>

#define SHIFT_PER_LEVEL (__builtin_ctz(PAGE_SIZE) - 3)
#define MASK ((1U << (SHIFT_PER_LEVEL)) - 1)
#define KOFF (SHIFT_PER_LEVEL)
#define JOFF (2 * SHIFT_PER_LEVEL)
#define IOFF (3 * SHIFT_PER_LEVEL)

typedef struct SlabList {
	struct SlabList *next;
	int64 id;
	byte padding[16];
	byte data[];
} SlabList;

SlabList sl_reserved_impl;
SlabList *sl_reserved = &sl_reserved_impl;

typedef struct SlabAllocatorImpl {
	byte ****data;
	BitMap bm;
	Lock lock;
	SlabList *head;
	SlabList *tail;
	void *bitmap_pages;
	unsigned int slab_size;
	unsigned int sl_size;
	unsigned int max_free_slabs;
	unsigned int max_total_slabs;
	unsigned int free_slabs;
	unsigned int total_slabs;
	byte padding[32];
} SlabAllocatorImpl;

void __attribute__((constructor)) __check_slabs_sizes() {
	if (sizeof(SlabAllocatorImpl) != sizeof(SlabAllocator))
		panic("sizeof(SlabAllocatorImpl) (%i) != sizeof(SlabAllocator) (%i)",
			  sizeof(SlabAllocatorImpl), sizeof(SlabAllocator));
	if (sizeof(SlabList) != SLAB_LIST_SIZE)
		panic("sizeof(SlabList) (%i) != SLAB_LIST_SIZE (%i)", sizeof(SlabList),
			  SLAB_LIST_SIZE);
}

SlabList *slab_allocator_grow(SlabAllocatorImpl *impl) {
	if (AADD(&impl->total_slabs, 1) > impl->max_total_slabs) {
		ASUB(&impl->total_slabs, 1);
		SetErr(CapacityExceeded);
		return NULL;
	}

	int64 id = bitmap_allocate(&impl->bm);
	if (id == -1) {
		void *addr = map(1);
		if (addr == NULL) return NULL;
		bitmap_extend(&impl->bm, addr);
		id = bitmap_allocate(&impl->bm);
	}
	if (id < 0) return NULL;
	int64 next = id >> (__builtin_ctz(PAGE_SIZE / impl->sl_size));
	unsigned int entries = ((1U << (__builtin_ctz(PAGE_SIZE / impl->sl_size))));
	int offset = (id % entries) * impl->sl_size;
	int i = next >> (2 * SHIFT_PER_LEVEL);
	int j = (next >> SHIFT_PER_LEVEL) & MASK;
	int k = next & MASK;

	SlabList *sl = NULL;
	bool alloc_err = false;

	lockr(&impl->lock);

	if (impl->data == NULL) {
		locku(&impl->lock);
		impl->data = (byte ****)map(1);
		if (impl->data == NULL) alloc_err = true;
		lockd(&impl->lock);
	}

	if (!alloc_err && impl->data[i] == NULL) {
		locku(&impl->lock);
		impl->data[i] = (byte ***)map(1);
		if (impl->data[i] == NULL) alloc_err = true;
		lockd(&impl->lock);
	}

	if (!alloc_err && impl->data[i][j] == NULL) {
		locku(&impl->lock);
		impl->data[i][j] = (byte **)map(1);
		if (impl->data[i][j] == NULL) alloc_err = true;
		lockd(&impl->lock);
	}

	if (!alloc_err && impl->data[i][j][k] == NULL) {
		locku(&impl->lock);
		impl->data[i][j][k] = (byte *)map(1);
		if (impl->data[i][j][k] == NULL) alloc_err = true;
		lockd(&impl->lock);
	}

	if (!alloc_err) {
		sl = (SlabList *)(impl->data[i][j][k] + offset);
		sl->id = id;
		sl->next = sl_reserved;
	}

	unlock(&impl->lock);

	return sl;
}

bool is_power_of_2(unsigned int n) {
	if (n == 0) {
		return false;
	}
	return (n & (n - 1)) == 0;
}

int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						unsigned int max_free_slabs,
						unsigned int max_total_slabs) {
	if (!is_power_of_2(slab_size + sizeof(SlabList))) {
		SetErr(IllegalArgument);
		return -1;
	}
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	impl->sl_size = slab_size + sizeof(SlabList);
	impl->slab_size = slab_size;
	impl->max_free_slabs = max_free_slabs;
	impl->max_total_slabs = max_total_slabs;
	impl->data = NULL;
	impl->lock = INIT_LOCK;

	ASTORE(&impl->free_slabs, 0);
	ASTORE(&impl->total_slabs, 0);

	impl->bitmap_pages = map(1);
	if (impl->bitmap_pages == NULL) {
		SetErr(AllocErr);
		return -1;
	}
	if (bitmap_init(&impl->bm, 1, impl->bitmap_pages)) {
		unmap(impl->bitmap_pages, 1);
		return -1;
	}
	void *page0 = map(1);
	if (page0 == NULL) {
		unmap(impl->bitmap_pages, 1);
		SetErr(AllocErr);
		return -1;
	}
	bitmap_extend(&impl->bm, page0);

	impl->head = impl->tail = slab_allocator_grow(impl);

	return 0;
}

void slab_allocator_cleanup(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
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

Slab *slab_allocator_allocate(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	SlabList *ret, *head, *tail, *next;
	loop {
		head = impl->head;
		tail = impl->tail;
		next = head->next;
		if (head == impl->head) {
			if (head == tail) {
				return ((Slab *)slab_allocator_grow(impl));
			} else {
				ret = head;
				if (CAS_SEQ(&impl->head, &head, next)) break;
			}
		}
	}
	ASUB(&impl->free_slabs, 1);
	ASTORE(&ret->next, sl_reserved);
	return ((Slab *)ret);
}

void slab_allocator_free(SlabAllocator *sa, Slab *slab) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	SlabList *slptr = (SlabList *)slab, *tail, *next;
	if (!CAS(&slptr->next, &sl_reserved, NULL)) {
		panic("Double free attempt! id=%lli", slptr->id);
	}

	if (AADD(&impl->free_slabs, 1) >= impl->max_free_slabs) {
		ASUB(&impl->free_slabs, 1);
		bitmap_free(&impl->bm, ((SlabList *)slab)->id);
	}
	loop {
		tail = impl->tail;
		next = tail->next;
		if (tail == impl->tail) {
			if (CAS_SEQ(&tail->next, &next, slptr)) {
				CAS_SEQ(&impl->tail, &tail, slptr);
				break;
			}
		}
	}
}
