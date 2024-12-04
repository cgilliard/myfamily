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

#include <base/atomic.h>
#include <base/bitmap.h>
#include <base/err.h>
#include <base/lock.h>
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
	long long id;
	unsigned char padding[16];
	unsigned char data[];
} SlabList;

SlabList sl_reserved_impl;
SlabList *sl_reserved = &sl_reserved_impl;

typedef struct SlabAllocatorImpl {
	unsigned char ****data;
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
	unsigned char padding[32];
} SlabAllocatorImpl;

void __attribute__((constructor)) __check_slabs_sizes() {
	if (sizeof(SlabAllocatorImpl) != sizeof(SlabAllocator))
		panic("sizeof(SlabAllocatorImpl) ({}) != sizeof(SlabAllocator) ({})",
			  sizeof(SlabAllocatorImpl), sizeof(SlabAllocator));
	if (sizeof(SlabList) != SLAB_LIST_SIZE)
		panic("sizeof(SlabList) ({}) != SLAB_LIST_SIZE ({})", sizeof(SlabList),
			  SLAB_LIST_SIZE);
}

SlabList *slab_allocator_grow(SlabAllocatorImpl *impl) {
	if (AADD(&impl->total_slabs, 1) > impl->max_total_slabs) {
		ASUB(&impl->total_slabs, 1);
		return 0;
	}

	let bobj = bitmap_allocate(&impl->bm);
	unsigned long long id = $uint(bobj);
	if (id == -1) {
		void *addr = map(1);
		if (addr == 0) return 0;
		let x = bitmap_extend(&impl->bm, addr);
		if (object_type(&x) == Err) return 0;
		let v = bitmap_allocate(&impl->bm);
		id = $uint(v);
	}
	if (id < 0) return 0;
	long long next = id >> (__builtin_ctz(PAGE_SIZE / impl->sl_size));
	unsigned int entries = ((1U << (__builtin_ctz(PAGE_SIZE / impl->sl_size))));
	int offset = (id % entries) * impl->sl_size;
	int i = next >> (2 * SHIFT_PER_LEVEL);
	int j = (next >> SHIFT_PER_LEVEL) & MASK;
	int k = next & MASK;

	SlabList *sl = 0;
	int alloc_err = 0;

	lockr(&impl->lock);

	if (impl->data == 0) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data == 0) {
			impl->data = (unsigned char ****)map(1);
			if (impl->data == 0) alloc_err = 1;
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}

	if (!alloc_err && impl->data[i] == 0) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i] == 0) {
			impl->data[i] = (unsigned char ***)map(1);
			if (impl->data[i] == 0) alloc_err = 1;
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}

	if (!alloc_err && impl->data[i][j] == 0) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i][j] == 0) {
			impl->data[i][j] = (unsigned char **)map(1);
			if (impl->data[i][j] == 0) alloc_err = 1;
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}

	if (!alloc_err && impl->data[i][j][k] == 0) {
		unlock(&impl->lock);
		lockw(&impl->lock);
		if (impl->data[i][j][k] == 0) {
			impl->data[i][j][k] = (unsigned char *)map(1);
			if (impl->data[i][j][k] == 0) alloc_err = 1;
		}
		unlock(&impl->lock);
		lockr(&impl->lock);
	}

	if (!alloc_err) {
		sl = (SlabList *)(impl->data[i][j][k] + offset);
		sl->id = id;
		sl->next = sl_reserved;
	}

	unlock(&impl->lock);
	return sl;
}

int is_power_of_2(unsigned int n) {
	if (n == 0) return 0;
	return (n & (n - 1)) == 0;
}

Object slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						   unsigned int max_free_slabs,
						   unsigned int max_total_slabs) {
	if (!is_power_of_2(slab_size + sizeof(SlabList))) {
		return Err(IllegalArgument);
	}
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	impl->sl_size = slab_size + sizeof(SlabList);
	impl->slab_size = slab_size;
	impl->max_free_slabs = max_free_slabs;
	impl->max_total_slabs = max_total_slabs;
	impl->data = 0;
	impl->lock = INIT_LOCK;

	ASTORE(&impl->free_slabs, 0);
	ASTORE(&impl->total_slabs, 0);

	impl->bitmap_pages = map(1);
	if (impl->bitmap_pages == 0) {
		return Err(AllocErr);
	}
	Object bm_status = bitmap_init(&impl->bm, 1, impl->bitmap_pages);

	if (object_type(&bm_status) == Err) {
		unmap(impl->bitmap_pages, 1);
		return bm_status;
	}
	void *page0 = map(1);
	if (page0 == 0) {
		unmap(impl->bitmap_pages, 1);
		return Err(AllocErr);
	}
	let res = bitmap_extend(&impl->bm, page0);
	if (object_type(&res) == Err) return res;

	impl->head = impl->tail = slab_allocator_grow(impl);

	return $(0);
}

void slab_allocator_cleanup(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	bitmap_cleanup(&impl->bm);
	if (impl->data) {
		for (int i = 0;
			 i < PAGE_SIZE / sizeof(unsigned char *) && impl->data[i]; i++) {
			for (int j = 0;
				 j < PAGE_SIZE / sizeof(unsigned char *) && impl->data[i][j];
				 j++) {
				for (int k = 0; k < PAGE_SIZE / sizeof(unsigned char *) &&
								impl->data[i][j][k];
					 k++)
					unmap((unsigned char *)impl->data[i][j][k], 1);
				unmap((unsigned char *)impl->data[i][j], 1);
			}
			unmap((unsigned char *)impl->data[i], 1);
		}
		unmap((unsigned char *)impl->data, 1);
	}
}

void *slab_allocator_allocate(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	SlabList *ret, *head, *tail, *next;
	while (1) {
		head = impl->head;
		tail = impl->tail;
		next = head->next;
		if (head == impl->head) {
			if (head == tail) {
				return ((unsigned char *)slab_allocator_grow(impl) +
						sizeof(SlabList));
			} else {
				ret = head;
				if (CAS_SEQ(&impl->head, &head, next)) break;
			}
		}
	}
	ASUB(&impl->free_slabs, 1);
	ASTORE(&ret->next, sl_reserved);
	return ((unsigned char *)ret + sizeof(SlabList));
}

void slab_allocator_free(SlabAllocator *sa, void *ptr) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	SlabList *slptr = (SlabList *)((unsigned char *)ptr - sizeof(SlabList)),
			 *tail, *next;
	if (!CAS(&slptr->next, &sl_reserved, 0)) {
		panic("Double free attempt! id={}", slptr->id);
	}

	if (AADD(&impl->free_slabs, 1) >= impl->max_free_slabs) {
		ASUB(&impl->free_slabs, 1);
		bitmap_free(&impl->bm, $(slptr->id));
	}
	while (1) {
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
