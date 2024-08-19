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

#include <core/class.h>
#include <core/global_counter.h>
#include <core/stack.h>
#include <criterion/criterion.h>
#include <pthread.h>
#include <unistd.h>

Test(core, test_stack) {
	StackAllocator sa;
	StackAllocatorConfig config = DEFAULT_STACK_ALLOCATOR_CONFIG;
	build_stack(&config, &sa);
	size_t my_offset1;
	size_t my_offset2;

	cr_assert_eq(allocate_stack(sizeof(u32), &sa), 0);
	SP_OFFSET(u32, sa, 0) = 0;
	for (int i = 0; i < 10; i++) {
		SP_OFFSET(u32, sa, 0)++;
	}
	cr_assert_eq(SP_OFFSET(u32, sa, 0), 10);

	cr_assert_eq(allocate_stack(sizeof(i64), &sa), 0);
	SP_OFFSET(i64, sa, 4) = 3;
	SP_OFFSET(i64, sa, 4) -= 10;

	cr_assert_eq(SP_OFFSET(i64, sa, 4), -7);
	cr_assert_eq(SP_OFFSET(u32, sa, 0), 10);

	cleanup_stack(&sa);
}

Test(core, test_stack_resize) {
	StackAllocator sa;
	StackAllocatorConfig config = DEFAULT_STACK_ALLOCATOR_CONFIG;
	build_stack(&config, &sa);

	size_t my_offset1;
	size_t my_offset2;
	size_t my_offset3;
	size_t my_offset4;

	cr_assert_eq(allocate_stack(sizeof(u32), &sa), 0);
	cr_assert_eq(allocate_stack(sizeof(i64), &sa), 0);

	SP_OFFSET(u32, sa, 0) = 1;
	SP_OFFSET(i64, sa, 4) = -7;

	cr_assert_eq(SP_OFFSET(u32, sa, 0), 1);
	cr_assert_eq(SP_OFFSET(i64, sa, 4), -7);

	cr_assert_eq(allocate_stack(5000, &sa), 0);

	cr_assert_eq(SP_OFFSET(u32, sa, 0), 1);
	cr_assert_eq(SP_OFFSET(i64, sa, 4), -7);

	SP_OFFSET(u32, sa, 12) = 12;
	SP_OFFSET(u32, sa, 16) = 13;
	SP_OFFSET(u32, sa, 20) = 14;

	cr_assert_eq(SP_OFFSET(u32, sa, 0), 1);
	cr_assert_eq(SP_OFFSET(i64, sa, 4), -7);
	cr_assert_eq(SP_OFFSET(u32, sa, 12), 12);
	cr_assert_eq(SP_OFFSET(u32, sa, 16), 13);
	cr_assert_eq(SP_OFFSET(u32, sa, 20), 14);

	cr_assert_eq(allocate_stack(sizeof(i64), &sa), 0);
	SP_OFFSET(i64, sa, 5000 + sizeof(u32) + sizeof(i64)) = -1;
	cr_assert_eq(SP_OFFSET(i64, sa, 5000 + sizeof(u32) + sizeof(i64)), -1);

	cr_assert_eq(SP_OFFSET(u32, sa, 0), 1);
	cr_assert_eq(SP_OFFSET(i64, sa, 4), -7);
	cr_assert_eq(SP_OFFSET(u32, sa, 12), 12);
	cr_assert_eq(SP_OFFSET(u32, sa, 16), 13);
	cr_assert_eq(SP_OFFSET(u32, sa, 20), 14);

	cr_assert_eq(allocate_stack(sizeof(i64), &sa), 0);
	SP_OFFSET(i64, sa, 5000 + sizeof(u32) + sizeof(i64) + sizeof(i64)) =
	    123;
	cr_assert_eq(SP_OFFSET(u32, sa, 0), 1);
	cr_assert_eq(SP_OFFSET(i64, sa, 4), -7);
	cr_assert_eq(
	    SP_OFFSET(i64, sa, 5000 + sizeof(u32) + sizeof(i64) + sizeof(i64)),
	    123);

	cleanup_stack(&sa);
}

Test(core, test_stack_deallocate) {
	StackAllocator sa;
	StackAllocatorConfig config = DEFAULT_STACK_ALLOCATOR_CONFIG;
	build_stack(&config, &sa);

	cr_assert_eq(allocate_stack(sizeof(i32), &sa), 0);
	SP_OFFSET(i32, sa, 0) = 100;
	cr_assert_eq(allocate_stack(sizeof(i64), &sa), 0);
	SP_OFFSET(i64, sa, 4) = 200;
	cr_assert_eq(allocate_stack(sizeof(i128), &sa), 0);
	SP_OFFSET(i128, sa, 12) = 300;

	cr_assert_eq(SP_OFFSET(i32, sa, 0), 100);
	cr_assert_eq(SP_OFFSET(i64, sa, 4), 200);
	cr_assert_eq(SP_OFFSET(i128, sa, 12), 300);

	cr_assert_eq(deallocate_stack(sizeof(i128), &sa), 0);
	cr_assert_eq(allocate_stack(sizeof(i32), &sa), 0);
	cr_assert_eq(allocate_stack(sizeof(i32), &sa), 0);
	cr_assert_eq(allocate_stack(sizeof(i32), &sa), 0);
	cr_assert_eq(allocate_stack(sizeof(i32), &sa), 0);

	SP_OFFSET(i32, sa, 12) = 1;
	SP_OFFSET(i32, sa, 16) = 2;
	SP_OFFSET(i32, sa, 20) = 3;
	SP_OFFSET(i32, sa, 24) = 4;

	// neq because of deallocation and reassignment
	cr_assert_neq(SP_OFFSET(i128, sa, 12), 300);

	cr_assert_eq(SP_OFFSET(i32, sa, 0), 100);
	cr_assert_eq(SP_OFFSET(i64, sa, 4), 200);
	cr_assert_eq(SP_OFFSET(i32, sa, 12), 1);
	cr_assert_eq(SP_OFFSET(i32, sa, 16), 2);
	cr_assert_eq(SP_OFFSET(i32, sa, 20), 3);
	cr_assert_eq(SP_OFFSET(i32, sa, 24), 4);

	cleanup_stack(&sa);
}
