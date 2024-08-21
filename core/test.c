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

#include <core/std.h>
#include <criterion/criterion.h>

Test(core, test_heap) {
	cr_assert_eq(__malloc_count, __free_count);

	// setup some fat pointers and assert len and data function
	FatPtr ptrs[40];
	FatPtr ptr1 = {1, NULL, 2};
	cr_assert_eq(fat_ptr_len(&ptr1), 2);
	cr_assert_eq(fat_ptr_data(&ptr1), NULL);

	// setup an ha with three sizes
	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 1, 40};
	HeapDataParamsConfig hdconfig2 = {8, 20, 1, 40};
	HeapDataParamsConfig hdconfig3 = {32, 14, 1, 40};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 3, hdconfig1,
					  hdconfig2, hdconfig3),
		     0);

	u64 last_size8_id = 0;
	u64 last_size16_id = 0;
	u64 last_size32_id = 0;
	for (u64 i = 1; i < 40; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, i, &ptrs[i]), 0);
		if (i <= 8) {
			// id from first batch with no batch id
			cr_assert_eq(ptrs[i].id, i - 1);
			cr_assert_eq(ptrs[i].len, 8);
			last_size8_id = ptrs[i].id;
		} else if (i <= 16) {
			// id from the second batch with (1 << 56)
			cr_assert_eq(ptrs[i].id, (i - 9) | (0x1L << 56));
			cr_assert_eq(ptrs[i].len, 16);
			last_size16_id = ptrs[i].id;
		} else if (i <= 32) {
			// id from the third batch with (2 << 56)
			cr_assert_eq(ptrs[i].id, (i - 17) | (0x2L << 56));
			cr_assert_eq(ptrs[i].len, 32);
			last_size32_id = ptrs[i].id;
		} else {
			// these are allocating using malloc
			cr_assert_eq(ptrs[i].len, i);
			cr_assert_eq(ptrs[i].id, UINT64_MAX);
		}
	}

	// free all slabs
	for (u64 i = 1; i < 40; i++) {
		cr_assert_eq(heap_allocator_free(&ha, &ptrs[i]), 0);
	}

	// assigning a new slab should assign the last id from each batch
	cr_assert_eq(heap_allocator_allocate(&ha, 8, &ptrs[0]), 0);
	cr_assert_eq(ptrs[0].id, last_size8_id);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[1]), 0);
	cr_assert_eq(ptrs[1].id, last_size16_id);

	cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptrs[2]), 0);
	cr_assert_eq(ptrs[2].id, last_size32_id);

	// and so on try again
	cr_assert_eq(heap_allocator_allocate(&ha, 8, &ptrs[3]), 0);
	cr_assert_eq(ptrs[3].id, last_size8_id - 1);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[4]), 0);
	cr_assert_eq(ptrs[4].id, last_size16_id - 1);

	cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptrs[5]), 0);
	cr_assert_eq(ptrs[5].id, last_size32_id - 1);

	cr_assert_eq(heap_allocator_allocate(&ha, 8, &ptrs[6]), 0);
	cr_assert_eq(ptrs[6].id, last_size8_id - 2);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[7]), 0);
	cr_assert_eq(ptrs[7].id, last_size16_id - 2);

	cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptrs[8]), 0);
	cr_assert_eq(ptrs[8].id, last_size32_id - 2);

	// continue with the 32 all the way (16 total 13 + 3)
	for (u64 i = 0; i < 13; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptrs[9 + i]), 0);
		cr_assert_eq(ptrs[9 + i].id, last_size32_id - (3 + i));
	}

	// next allocation id should be last_size32_id + 1 because the freed ids
	// have been exhausted
	cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptrs[39]), 0);
	cr_assert_eq(ptrs[39].id, last_size32_id + 1);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_heap_resize) {
	cr_assert_eq(__malloc_count, __free_count);

	// setup an ha and 46 fat pointers
	FatPtr ptrs[46];
	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 1, 45};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	// allocate all slabs. The ha should resize twice and return ids in
	// sequential order
	for (u64 i = 0; i < 45; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[i]), 0);
		cr_assert_eq(ptrs[i].id, i);
		cr_assert_eq(ptrs[i].len, 16);
	}

	// this one is allocated using malloc so id = UINT64_MAX
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[45]), 0);
	cr_assert_eq(ptrs[45].id, UINT64_MAX);
	cr_assert_eq(ptrs[45].len, 16);

	// free malloced fat pointer
	heap_allocator_free(&ha, &ptrs[45]);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_heap_free) {
	cr_assert_eq(__malloc_count, __free_count);

	// setup ha with a single size
	FatPtr ptrs[46];
	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 1, 45};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	// allocate a few slabs and check ids
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[0]), 0);
	cr_assert_eq(ptrs[0].id, 0);
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[1]), 0);
	cr_assert_eq(ptrs[1].id, 1);
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[2]), 0);
	cr_assert_eq(ptrs[2].id, 2);

	// free the first slab
	cr_assert_eq(heap_allocator_free(&ha, &ptrs[0]), 0);

	// reallocate, it should be the first slab id = 0
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[0]), 0);
	cr_assert_eq(ptrs[0].id, 0);

	// next in the free list would be 3
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[3]), 0);
	cr_assert_eq(ptrs[3].id, 3);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_heap_initial_size0) {
	cr_assert_eq(__malloc_count, __free_count);

	// setup ha with a single size and no slabs initially
	FatPtr ptrs[46];
	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 0, 45};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[0]), 0);
	cr_assert_eq(ptrs[0].id, 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[1]), 0);
	cr_assert_eq(ptrs[1].id, 1);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[2]), 0);
	cr_assert_eq(ptrs[2].id, 2);

	cr_assert_eq(heap_allocator_free(&ha, &ptrs[0]), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[3]), 0);
	cr_assert_eq(ptrs[3].id, 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[4]), 0);
	cr_assert_eq(ptrs[4].id, 3);

	cr_assert_eq(heap_allocator_free(&ha, &ptrs[4]), 0);
	cr_assert_eq(heap_allocator_free(&ha, &ptrs[2]), 0);
	cr_assert_eq(heap_allocator_free(&ha, &ptrs[1]), 0);
	cr_assert_eq(heap_allocator_free(&ha, &ptrs[3]), 0);

	for (u64 i = 0; i < 45; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[i]), 0);
		cr_assert_eq(ptrs[i].id, i);
		cr_assert_eq(ptrs[i].len, 16);
	}

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[45]), 0);
	cr_assert_eq(ptrs[45].id, UINT64_MAX);
	cr_assert_eq(ptrs[45].len, 16);

	cr_assert_eq(heap_allocator_free(&ha, &ptrs[45]), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_heap_init_multi_chunk) {
	cr_assert_eq(__malloc_count, __free_count);

	// setup ha with multi initial chunks
	FatPtr ptrs[81];
	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 2, 80};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	for (u64 i = 0; i < 80; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[i]), 0);
		cr_assert_eq(ptrs[i].id, i);
		cr_assert_eq(ptrs[i].len, 16);
	}

	// no more slabs
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[80]), 0);
	cr_assert_eq(ptrs[80].id, UINT64_MAX);

	for (u64 i = 0; i < 81; i++) {
		cr_assert_eq(heap_allocator_free(&ha, &ptrs[i]), 0);
	}

	for (u64 i = 0; i < 80; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[i]), 0);
		cr_assert_eq(ptrs[i].id, 79 - i);
		cr_assert_eq(ptrs[i].len, 16);
	}

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_invalid_configurations) {
	cr_assert_eq(__malloc_count, __free_count);

	cr_assert_neq(heap_allocator_build(NULL, NULL, 0), 0);
	HeapAllocator ha;
	FatPtr ptr, ptr2;
	HeapAllocatorConfig hconfig = {true, true};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 0), 0);
	// no_malloc is true so this fails
	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	HeapDataParamsConfig hdconfig1 = {16, 0, 0, 0};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	// can't create any slabs because max_slabs is 0 and no_malloc true
	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	// test freeing an invalid fptr
	HeapDataParamsConfig hdconfig2 = {16, 10, 1, 10};
	HeapDataParamsConfig hdconfig2a = {32, 10, 1, 10};
	cr_assert_eq(
	    heap_allocator_build(&ha, &hconfig, 2, hdconfig2, hdconfig2a), 0);
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptr2), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	HeapDataParamsConfig hdconfig3 = {16, 0, 0, 0};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig3), 0);

	// delete invalid fat ptrs from another ha
	cr_assert_neq(heap_allocator_free(&ha, &ptr), 0);
	cr_assert_neq(heap_allocator_free(&ha, &ptr2), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_heap_oom) {
	cr_assert_eq(__malloc_count, __free_count);

	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {true, true};
	HeapDataParamsConfig hdconfig1 = {32, 10, 1, 10};
	HeapDataParamsConfig hdconfig2 = {16, 10, 1, 10};
	__debug_build_allocator_malloc_fail1 = true;
	cr_assert_neq(heap_allocator_build(&ha, &hconfig, 0), 0);

	cr_assert_eq(__malloc_count, __free_count);

	__debug_build_allocator_malloc_fail1 = false;
	__debug_build_allocator_malloc_fail2 = true;
	cr_assert_neq(heap_allocator_build(&ha, &hconfig, 0), 0);

	cr_assert_eq(__malloc_count, __free_count);

	__debug_build_allocator_malloc_fail2 = false;
	__debug_build_allocator_malloc_fail3 = true;

	cr_assert_neq(
	    heap_allocator_build(&ha, &hconfig, 2, hdconfig1, hdconfig2), 0);

	cr_assert_eq(__malloc_count, __free_count);

	__debug_build_allocator_malloc_fail3 = false;
	__debug_build_allocator_malloc_fail4 = true;

	cr_assert_neq(
	    heap_allocator_build(&ha, &hconfig, 2, hdconfig1, hdconfig2), 0);

	cr_assert_eq(__malloc_count, __free_count);

	__debug_build_allocator_malloc_fail4 = false;
	__debug_build_allocator_malloc_fail5 = true;

	cr_assert_neq(
	    heap_allocator_build(&ha, &hconfig, 2, hdconfig1, hdconfig2), 0);

	cr_assert_eq(__malloc_count, __free_count);

	// make a valid heap for testing allocate failures
	__debug_build_allocator_malloc_fail5 = false;
	FatPtr ptr;
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	__debug_build_allocator_malloc_fail6 = true;
	cr_assert_neq(heap_allocator_allocate(&ha, 33, &ptr), 0);

	for (u64 i = 0; i < 10; i++)
		cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptr), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);

	// create a malloc enabled ha
	HeapAllocatorConfig hconfig2 = {false, false};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig2, 1, hdconfig1), 0);

	for (u64 i = 0; i < 10; i++)
		cr_assert_eq(heap_allocator_allocate(&ha, 32, &ptr), 0);

	// malloc configured to fail here
	cr_assert_neq(heap_allocator_allocate(&ha, 32, &ptr), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);

	// test malloc failure in resize
	HeapDataParamsConfig hdconfig3 = {16, 10, 1, 30};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig2, 1, hdconfig3), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}
