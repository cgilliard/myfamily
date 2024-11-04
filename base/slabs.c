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

SlabImpl slab_s = {.next = (void *)37};
SlabImpl *slab_special_location = &slab_s;	// for double free prevention
#define SPECIAL ((SlabImpl *)slab_special_location)

Slab global_allocated = NULL;

Slab slab_allocator_grow(SlabAllocator *sa) {
	return malloc(sizeof(SlabImpl) + sa->slab_size);
}

void slab_allocator_cleanup(SlabAllocator *ptr) {
	// TODO: empty queue and deallocate slabs
}

// initialize slab allocator as a michael-scott queue with specified slab_size
int slab_allocator_init(SlabAllocator *sa, unsigned int slab_size) {
	sa->slab_size = slab_size;
	Slab s = malloc(sizeof(SlabImpl) + slab_size);
	if (s == NULL) {
		SetErr(AllocErr);
		return -1;
	}
	s->next = NULL;
	sa->head = sa->tail = s;
	return 0;
}

// allocate is dequeue. If null "grow" by mallocing a slab.
Slab slab_allocator_allocate(SlabAllocator *sa) {
	Slab head, tail, next;

	loop {
		head = sa->head;
		tail = sa->tail;
		next = head->next;
		if (head == sa->head) {
			if (head == tail) {
				if (next == NULL) {
					return slab_allocator_grow(sa);
				}
				CAS_SEQ(&sa->tail, &tail, next);
			} else {
				if (CAS_SEQ(&sa->head, &head, next)) break;
			}
		}
	}
	return next;
}

// free is enqueue.
// TODO: need to allow for a maximum queue size
// with a maximum queue size, we can then simply 'free' the excess
// slabs as they are freed.
void slab_allocator_free(SlabAllocator *sa, Slab slab) {
	Slab tail, next;
	slab->next = NULL;
	loop {
		tail = sa->tail;
		next = tail->next;
		if (tail == sa->tail) {
			if (next == NULL) {
				if (CAS_SEQ(&tail->next, &next, slab)) {
					break;
				}
			} else {
				CAS_SEQ(&sa->tail, &tail, next);
			}
		}
	}
	CAS_SEQ(&sa->tail, &tail, slab);
}
