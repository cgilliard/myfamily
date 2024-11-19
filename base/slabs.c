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

#include <base/err.h>
#include <base/macros.h>
#include <base/print_util.h>
#include <base/slabs.h>

typedef struct SlabList {
	struct SlabList *next;
	Slab slab;
	byte data[];
} SlabList;

SlabList ptr_reserved_concrete = {};
SlabList *ptr_reserved = &ptr_reserved_concrete;

typedef struct SlabAllocatorImpl {
	Alloc a;
	unsigned int slab_size;
	unsigned int max_free_slabs;
	unsigned int max_total_slabs;
	unsigned int free_size;
	unsigned int total_slabs;
	SlabList *head;
	SlabList *tail;
	Ptr reserved1;
	Ptr reserved2;
} SlabAllocatorImpl;

void __attribute__((constructor)) __check_slabs_sizes() {
	if (sizeof(SlabAllocatorImpl) != sizeof(SlabAllocator))
		panic("sizeof(SlabAllocatorImpl) (%i) != sizeof(SlabAllocator) (%i)",
			  sizeof(SlabAllocatorImpl), sizeof(SlabAllocator));
}

byte *slab_get(const SlabAllocator *sa, Ptr p) {
	return NULL;
}

Slab slab_allocator_grow(SlabAllocatorImpl *impl) {
	Slab ret = {};
	if (AADD(&impl->total_slabs, 1) > impl->max_total_slabs) {
		ASUB(&impl->total_slabs, 1);
		SetErr(CapacityExceeded);
		return ret;
	}
	ret = alloc(&impl->a);
	if (ret.data == NULL) return ret;
	SlabList *sl = (SlabList *)ret.data;
	sl->slab = ret;
	sl->next = ptr_reserved;
	ret.data = sl->data;
	return ret;
}

int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						unsigned int max_free_slabs,
						unsigned int max_total_slabs) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	if (alloc_init(&impl->a, slab_size + sizeof(SlabList), UINT32_MAX))
		return -1;
	impl->slab_size = slab_size;
	impl->max_free_slabs = max_free_slabs;
	impl->max_total_slabs = max_total_slabs;
	impl->free_size = 1;
	impl->total_slabs = 1;

	// start by allocating the first two slabs which will never be used.
	impl->reserved1 = slab_allocator_grow(impl).ptr;
	impl->reserved2 = slab_allocator_grow(impl).ptr;

	// create initial free slab (required for michael-scott queue)
	Slab slab = slab_allocator_grow(impl);
	if (slab.data == NULL) {
		alloc_cleanup(&impl->a);
		return -1;
	}
	impl->head = impl->tail = (SlabList *)(slab.data - sizeof(SlabList));

	ASTORE(&impl->free_size, 1);
	ASTORE(&impl->total_slabs, 1);

	return 0;
}
void slab_allocator_cleanup(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;

	release(&impl->a, impl->reserved1);
	release(&impl->a, impl->reserved2);
	alloc_cleanup(&impl->a);
}
Slab slab_allocator_allocate(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;

	Slab ret = {};
	SlabList *head, *tail, *next;
	loop {
		head = impl->head;
		tail = impl->tail;
		next = head->next;
		if (head == impl->head) {
			if (head == tail) {
				return slab_allocator_grow(impl);
			} else {
				ret.ptr = head->slab.ptr;
				ret.data = head->data;
				if (CAS_SEQ(&impl->head, &head, next)) break;
			}
		}
	}

	ASUB(&impl->free_size, 1);
	SlabList *sl = (SlabList *)((byte *)ret.data - sizeof(SlabList));
	sl->next = ptr_reserved;
	return ret;
}
void slab_allocator_free(SlabAllocator *sa, Slab *slab) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;

	SlabList *slptr = (SlabList *)((byte *)slab->data - sizeof(SlabList));
	if (!CAS(&slptr->next, &ptr_reserved, NULL)) {
		panic("Double free attempt! ptr=%u", slab->ptr);
	}
	if (AADD(&impl->free_size, 1) >= impl->max_free_slabs) {
		release(&impl->a, slab->ptr);
		ASUB(&impl->total_slabs, 1);
		ASUB(&impl->free_size, 1);
		return;
	}

	SlabList *tail, *next;
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

#ifdef TEST
unsigned long long slab_allocator_free_size(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	return ALOAD(&impl->free_size);
}

unsigned long long slab_allocator_total_slabs(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	return ALOAD(&impl->total_slabs);
}
#endif	// TEST
