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

typedef struct SlabImpl {
	struct SlabImpl *next;
	byte data[];
} SlabImpl;

byte *slab_get(Slab s) {
	return s->data;
}

unsigned long long *slab_aux(Slab s) {
	return (unsigned long long *)&s->next;
}

SlabImpl slab_allocated_impl = {.next = NULL};
Slab slab_allocated_reqd = &slab_allocated_impl;
#define SLAB_ALLOCATED slab_allocated_reqd

Slab slab_allocator_grow(SlabAllocator *sa) {
	if (AADD(&sa->total_slabs, 1) > sa->max_total_slabs) {
		ASUB(&sa->total_slabs, 1);
		SetErr(CapacityExceeded);
		return NULL;
	}
	Slab ret = malloc(sizeof(SlabImpl) + sa->slab_size);
	if (ret == NULL) {
		SetErr(AllocErr);
		ASUB(&sa->total_slabs, 1);
		return NULL;
	}
	if (sa->free_check) ret->next = SLAB_ALLOCATED;
	return ret;
}

void slab_allocator_cleanup(SlabAllocator *sa) {
	Slab itt = sa->head;
	while (itt) {
		Slab to_delete = itt;
		itt = itt->next;
		free(to_delete);
	}
}

// initialize slab allocator as a michael-scott queue with specified slab_size
int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size,
						unsigned long long max_free_slabs,
						unsigned long long max_total_slabs, bool free_check) {
	/*
		Slab s = malloc(sizeof(SlabImpl) + slab_size);
		if (s == NULL) {
			SetErr(AllocErr);
			return -1;
		}
	*/
	ASTORE(&sa->free_size, 1);
	ASTORE(&sa->total_slabs, 0);
	sa->slab_size = slab_size;
	sa->max_free_slabs = max_free_slabs;
	sa->max_total_slabs = max_total_slabs;
	sa->free_check = free_check;

	Slab s = slab_allocator_grow(sa);

	s->next = NULL;
	sa->head = sa->tail = s;

	return 0;
}

// allocate is dequeue. If null "grow" by mallocing a slab.
Slab slab_allocator_allocate(SlabAllocator *sa) {
	Slab head, tail, next, ret;
	loop {
		head = sa->head;
		tail = sa->tail;
		next = head->next;
		if (head == sa->head) {
			if (head == tail) {
				if (next == NULL) {
					ret = slab_allocator_grow(sa);
					return ret;
				}
				CAS_SEQ(&sa->tail, &tail, next);
			} else {
				ret = head;
				if (CAS_SEQ(&sa->head, &head, next)) break;
			}
		}
	}

	ASUB(&sa->free_size, 1);
	if (sa->free_check) ASTORE(&ret->next, SLAB_ALLOCATED);
	return ret;
}

// free is enqueue.
void slab_allocator_free(SlabAllocator *sa, Slab slab) {
	if (sa->free_check && !CAS(&slab->next, &SLAB_ALLOCATED, NULL))
		panic("Double free attempt! %p %p", &slab->next, &SLAB_ALLOCATED);
	else if (!sa->free_check)
		slab->next = NULL;
	if (AADD(&sa->free_size, 1) > sa->max_free_slabs) {
		free(slab);
		ASUB(&sa->total_slabs, 1);
		return;
	}

	Slab tail, next;

	loop {
		tail = sa->tail;
		next = tail->next;
		if (tail == sa->tail) {
			if (next == NULL) {
				if (CAS_SEQ(&tail->next, &next, slab)) {
					CAS_SEQ(&sa->tail, &tail, slab);
					break;
				}
			} else {
				CAS_SEQ(&sa->tail, &tail, next);
			}
		}
	}
}

unsigned long long slab_allocator_free_size(SlabAllocator *sa) {
	return ALOAD(&sa->free_size);
}
unsigned long long slab_allocator_total_slabs(SlabAllocator *sa) {
	return ALOAD(&sa->total_slabs);
}
