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

#include <core/mymalloc.h>
#include <core/stack.h>
#include <errno.h>
#include <stdlib.h>

/*
 * To use this library, things must manually be aligned.
 * See below for formula for alignment
 * size_t alignment = (size >= 16)  ? 16
 *                         : (size >= 8) ? 8
 *                         : (size >= 4) ? 4
 *                         : (size >= 2) ? 2
 *                                       : 1;
 *
 * // Adjust stack pointer with alignment in mind
 * uintptr_t sp = (uintptr_t)sa->sp;
 * size_t ret = sa;
 * sp += size;
 * sp &= ~(alignment - 1);
 */

int reallocate_stack(size_t size, StackAllocator *sa) {
	size_t nsize = sa->size * sa->realloc_factor;
	size_t cur = sa->sp - sa->begin.data;

	// attempt realloc
	if (myrealloc(&sa->begin, nsize))
		return -1;

	// success, values from the pointer
	sa->sp = sa->begin.data + cur;
	sa->size = nsize;

	return 0;
}

int build_stack(StackAllocatorConfig *config, StackAllocator *sa) {
	if (config->initial_size == 0) {
		// Handle invalid parameters
		return -1;
	}
	int res = mymalloc(&sa->begin, config->initial_size);
	if (res != 0) {
		// Handle allocation failure
		return -1;
	}

	sa->sp = sa->begin.data;
	sa->size = config->initial_size;
	sa->realloc_factor = config->realloc_factor;
	sa->initial_size = config->initial_size;

	return 0;
}

int allocate_stack(size_t size, StackAllocator *sa) {
	if (size == 0) {
		return -1; // Invalid parameters
	}

	uintptr_t sp = (uintptr_t)sa->sp;
	sp += size;

	// Check if the aligned stack pointer causes overflow
	while (sp > (uintptr_t)sa->begin.data + sa->size) {
		if (reallocate_stack(size, sa)) {
			return -1;
		}
		sp = (uintptr_t)sa->sp;
		sp += size;
	}

	// update stack pointer and return value
	sa->sp = (void *)sp;

	return 0; // Success
}

int deallocate_stack(size_t size, StackAllocator *sa) {
	if (size == 0) {
		return -1; // Invalid parameters
	}

	uintptr_t sp = (uintptr_t)sa->sp;
	sp -= size;

	// Check if the aligned stack pointer causes underflow
	if (sp < (uintptr_t)sa->begin.data) {
		errno = EINVAL;
		return -1;
	}

	sa->sp = (void *)sp;

	return 0; // Success
}

void *get_stack_begin(StackAllocator *sa) { return sa->begin.data; }

void cleanup_stack(StackAllocator *sa) {
	if (sa && sa->begin.data) {
		myfree(&sa->begin);
		sa->begin.data = NULL;
	}
}
