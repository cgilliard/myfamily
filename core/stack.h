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

#ifndef _CORE_STACK__
#define _CORE_STACK__

#include <core/heap.h>
#include <core/types.h>

typedef struct StackAllocator {
	FatPtr begin;
	void *sp;
	size_t size;
	f64 realloc_factor;
	size_t initial_size;
} StackAllocator;

typedef struct StackAllocatorConfig {
	size_t initial_size;
	f64 realloc_factor;
} StackAllocatorConfig;

int build_stack(StackAllocatorConfig *config, StackAllocator *sa);
int allocate_stack(size_t size, StackAllocator *sa);
int deallocate_stack(size_t size, StackAllocator *sa);
int calculate_size_stack(size_t return_type_size, size_t self_size,
			 StackAllocator *sa);
void *get_stack_begin(StackAllocator *sa);
void cleanup_stack(StackAllocator *sa);

#define DEFAULT_STACK_ALLOCATOR_CONFIG {2048, 2.0}
#define SP_OFFSET(type, sa, offset) (*((type *)(get_stack_begin(&sa) + offset)))

#endif // _CORE_STACK__
