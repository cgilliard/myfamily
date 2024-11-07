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
#include <base/fam_err.h>
#include <base/lock.h>
#include <base/macros.h>
#include <base/osdef.h>
#include <base/print_util.h>
#include <base/slabs.h>

typedef struct SlabList {
	Ptr next;
	byte data[];
} SlabList;

typedef struct SlabAllocatorImpl {
	MemMap mm;
	unsigned int slab_size;
	unsigned int max_free_slabs;
	unsigned int max_total_slabs;
	unsigned int free_size;
	unsigned int total_slabs;
	Ptr head;
	Ptr tail;
} SlabAllocatorImpl;

void __attribute__((constructor)) __slabs_check_sizes() {
	if (sizeof(SlabAllocatorImpl) != sizeof(SlabAllocator))
		panic("sizeof(SlabAllocatorImpl) (%i) != sizeof(SlabAllocator) (%i)",
			  sizeof(SlabAllocatorImpl), sizeof(SlabAllocator));
}

byte *slab_get(SlabAllocator *sa, Ptr ptr) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	SlabList *sl = memmap_data(&impl->mm, ptr);
	return sl->data;
}
unsigned long long *slab_aux(SlabAllocator *sa, Ptr ptr) {
	return NULL;
}

Ptr slab_allocator_grow(SlabAllocatorImpl *impl) {
	if (AADD(&impl->total_slabs, 1) > impl->max_total_slabs) {
		ASUB(&impl->total_slabs, 1);
		SetErr(CapacityExceeded);
		return null;
	}
	return memmap_allocate(&impl->mm);
}

int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						unsigned long long max_free_slabs,
						unsigned long long max_total_slabs) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;

	memmap_init(&impl->mm, slab_size + sizeof(SlabList));
	ASTORE(&impl->free_size, 1);
	ASTORE(&impl->total_slabs, 0);
	impl->slab_size = slab_size;
	impl->max_free_slabs = max_free_slabs;
	impl->max_total_slabs = max_total_slabs;

	Ptr ptr = slab_allocator_grow(impl);
	if (ptr == null) {
		return -1;
	}

	SlabList *sl = memmap_data(&impl->mm, ptr);
	sl->next = null;
	impl->head = impl->tail = ptr;

	return 0;
}

void slab_allocator_cleanup(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	Ptr itt = impl->head;
	while (itt) {
		Ptr to_delete = itt;
		SlabList *sl = memmap_data(&impl->mm, itt);
		itt = sl->next;
		memmap_free(&impl->mm, to_delete);
	}

	memmap_cleanup(&impl->mm);
}

Ptr slab_allocator_allocate(SlabAllocator *sa) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;
	Ptr head, tail, next, ret;
	loop {
		head = impl->head;
		tail = impl->tail;
		SlabList *sl = memmap_data(&impl->mm, head);
		next = sl->next;
		if (head == impl->head) {
			if (head == tail) {
				if (next == null) {
					return slab_allocator_grow(impl);
				}
				CAS_SEQ(&impl->tail, &tail, next);
			} else {
				ret = head;
				if (CAS_SEQ(&impl->head, &head, next)) break;
			}
		}
	}

	ASUB(&impl->free_size, 1);
	return ret;
}
void slab_allocator_free(SlabAllocator *sa, Ptr ptr) {
	SlabAllocatorImpl *impl = (SlabAllocatorImpl *)sa;

	SlabList *slptr = memmap_data(&impl->mm, ptr);
	slptr->next = null;
	if (AADD(&impl->free_size, 1) > impl->max_free_slabs) {
		memmap_free(&impl->mm, ptr);
		ASUB(&impl->total_slabs, 1);
		return;
	}

	Ptr tail, next;

	loop {
		tail = impl->tail;
		SlabList *sltail = memmap_data(&impl->mm, tail);
		next = sltail->next;
		if (tail == impl->tail) {
			if (next == null) {
				if (CAS_SEQ(&sltail->next, &next, ptr)) {
					CAS_SEQ(&impl->tail, &tail, ptr);
					break;
				}
			} else {
				CAS_SEQ(&impl->tail, &tail, next);
			}
		}
	}
}

#ifdef TEST
unsigned long long slab_allocator_free_size(SlabAllocator *sa) {
	return 0;
}

unsigned long long slab_allocator_total_slabs(SlabAllocator *sa) {
	return 0;
}
#endif	// TEST
