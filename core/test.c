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
#include <unistd.h>

Test(core, test_heap) {
	cr_assert_eq(__malloc_count, __free_count);

	// setup some fat pointers and assert len and data function
	FatPtr ptrs[40];
	FatPtr ptr1 = {1, NULL, 2};
	cr_assert_eq(fat_ptr_len(&ptr1), 2);
	cr_assert_eq(fat_ptr_data(&ptr1), NULL);
	cr_assert_eq(fat_ptr_id(&ptr1), 1);

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
	HeapDataParamsConfig hdconfig3 = {16, 10, 1, 30};

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

	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig3), 0);

	__debug_build_allocator_malloc_fail7 = true;
	__debug_build_allocator_malloc_fail6 = false;

	for (u64 i = 0; i < 10; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	}

	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	//__debug_build_allocator_malloc_fail4 = true;
	__debug_build_allocator_malloc_fail7 = false;

	// now we can create new slabs
	for (u64 i = 0; i < 10; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	}

	// set fail4 which fails in the init free list function
	__debug_build_allocator_malloc_fail4 = true;

	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	// use failure 5 which is a little later in the function
	// should be the same result
	__debug_build_allocator_malloc_fail5 = true;
	__debug_build_allocator_malloc_fail4 = false;

	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	__debug_build_allocator_malloc_fail5 = false;

	for (u64 i = 0; i < 10; i++) {
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	}

	// no more slabs and we have mo_malloc configured
	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);

	__debug_build_allocator_malloc_fail8 = true;

	cr_assert_neq(heap_allocator_build(&ha, &hconfig, 1, hdconfig3), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);

	// test failure6 faiure of malloc after no slabs left
	__debug_build_allocator_malloc_fail8 = false;
	__debug_build_allocator_malloc_fail6 = true;

	// build ok
	cr_assert_eq(heap_allocator_build(&ha, &hconfig2, 0), 0);
	// allocation failure
	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_heap_zeroed) {
	FatPtr ptr;
	HeapAllocator ha;
	HeapAllocatorConfig hconfig_zeroed = {true, true};
	HeapAllocatorConfig hconfig_non_zeroed = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 1, 45};
	char *data;

	cr_assert_eq(__malloc_count, __free_count);

	// first try without zeroed
	cr_assert_eq(
	    heap_allocator_build(&ha, &hconfig_non_zeroed, 1, hdconfig1), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(fat_ptr_id(&ptr), 0);
	data = fat_ptr_data(&ptr);
	for (u64 i = 0; i < 16; i++) {
		data[i] = 1;
	}

	cr_assert_eq(heap_allocator_free(&ha, &ptr), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(fat_ptr_id(&ptr), 0);
	data = fat_ptr_data(&ptr);
	for (u64 i = 0; i < 16; i++) {
		// since it's not zeroed, value will be '1' which was set before
		// freeing/allocating
		cr_assert_eq(data[i], 1);
	}

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);

	// setup ha with a single size with zeroed enabled
	cr_assert_eq(heap_allocator_build(&ha, &hconfig_zeroed, 1, hdconfig1),
		     0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(fat_ptr_id(&ptr), 0);
	data = fat_ptr_data(&ptr);
	for (u64 i = 0; i < 16; i++) {
		data[i] = 1;
	}

	cr_assert_eq(heap_allocator_free(&ha, &ptr), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(fat_ptr_id(&ptr), 0);
	data = fat_ptr_data(&ptr);
	for (u64 i = 0; i < 16; i++) {
		// this time it's zeroed out
		cr_assert_eq(data[i], 0);
	}

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_invalid_heap_configurations) {
	cr_assert_eq(__malloc_count, __free_count);

	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {true, true};
	HeapDataParamsConfig hdconfig1 = {0, 10, 1, 10};
	HeapDataParamsConfig hdconfig2 = {16, 10, 1, 10};

	cr_assert_neq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);

	cr_assert_neq(
	    heap_allocator_build(&ha, &hconfig, 2, hdconfig2, hdconfig2), 0);

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

int panic_loop = 0;

Test(core, test_panic) {
	bool is_panic = false;
	if (PANIC_RETURN()) {
		is_panic = true;
	}

	// first loop is not a panic
	if (panic_loop) {
		cr_assert(is_panic);
	} else {
		cr_assert(!is_panic);
	}
	panic_loop++;
	if (!is_panic)
		panic("panic test_panic");
}

Test(core, test_chained_allocator) {
	FatPtr ptr;

	// build 2 slab allocators with varying size slabs
	HeapAllocator ha, ha2;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 10, 1, 10};
	HeapDataParamsConfig hdconfig2 = {32, 10, 1, 10};
	cr_assert_eq(
	    heap_allocator_build(&ha, &hconfig, 2, hdconfig1, hdconfig2), 0);
	cr_assert_eq(heap_allocator_build(&ha2, &hconfig, 1, hdconfig1), 0);

	// set a custom default thread local slab allocator with these values
	HeapDataParamsConfig arr[] = {{16, 10, 0, UINT32_MAX},
				      {32, 10, 0, UINT32_MAX},
				      {48, 10, 0, UINT32_MAX},
				      {64, 10, 0, UINT32_MAX}};
	set_default_hdpc_arr(arr, sizeof(arr) / sizeof(arr[0]));

	// allocate before adding these allocators to the chain should get a
	// 48 byte block because the default heap allocator uses slabs that
	// are 16 bytes apart in size.
	cr_assert_eq(chain_malloc(&ptr, 33), 0);

	cr_assert_eq(fat_ptr_len(&ptr), 48);
	chain_free(&ptr);
	{
		// now set our first ha to the chain
		ChainGuard ca = SCOPED_ALLOCATOR(&ha, false);

		// blocks over 32 bytes use malloc and have the exact size
		cr_assert_eq(chain_malloc(&ptr, 33), 0);
		cr_assert_eq(fat_ptr_len(&ptr), 33);
		chain_free(&ptr);

		// blocks between 16 and 32 round up to 32 byte slabs
		cr_assert_eq(chain_malloc(&ptr, 17), 0);
		cr_assert_eq(fat_ptr_len(&ptr), 32);
		chain_free(&ptr);

		{
			// Use ha2 which has only 16 byte slabs. This 17 byte
			// FatPtr uses malloc and is exactly 17 bytes.
			ChainGuard ca2 = SCOPED_ALLOCATOR(&ha2, false);
			cr_assert_eq(chain_malloc(&ptr, 17), 0);
			cr_assert_eq(fat_ptr_len(&ptr), 17);
			chain_free(&ptr);
		}

		// with our first ha (since ha2 is now out of scope) the 17 byte
		// request maps to 32 byte slabs
		cr_assert_eq(chain_malloc(&ptr, 17), 0);
		cr_assert_eq(fat_ptr_len(&ptr), 32);
		chain_free(&ptr);
	}

	// now a 33 byte request maps to the default allocator which has 48 byte
	// slabs
	cr_assert_eq(chain_malloc(&ptr, 33), 0);
	cr_assert_eq(fat_ptr_len(&ptr), 48);
	chain_free(&ptr);

	// test the global sync allocator

	// Thread local allocator has a max slab size of 64 bytes so 87 is
	// allocated using malloc at the exact size
	cr_assert_eq(chain_malloc(&ptr, 87), 0);
	cr_assert_eq(fat_ptr_len(&ptr), 87);
	chain_free(&ptr);

	// reset the global allocator with 128 byte slabs
	HeapDataParamsConfig arr2[] = {{16, 10, 0, UINT32_MAX},
				       {32, 10, 0, UINT32_MAX},
				       {48, 10, 0, UINT32_MAX},
				       {64, 10, 0, UINT32_MAX},
				       {128, 10, 0, UINT32_MAX}};
	set_default_hdpc_arr(arr2, sizeof(arr2) / sizeof(arr2[0]));

	// now try with the global sync allocator which has 128 byte slabs.
	{
		ChainGuard guard = GLOBAL_SYNC_ALLOCATOR();
		cr_assert_eq(chain_malloc(&ptr, 87), 0);
		cr_assert_eq(fat_ptr_len(&ptr), 128);
		chain_free(&ptr);
	}

	// now it's out of scope we should be back at 87 bytes
	cr_assert_eq(chain_malloc(&ptr, 87), 0);
	cr_assert_eq(fat_ptr_len(&ptr), 87);
	chain_free(&ptr);

	// test realloc
	FatPtr ptr2;
	cr_assert_eq(chain_malloc(&ptr, 16), 0);
	cr_assert_eq(fat_ptr_len(&ptr), 16);
	char *data, *data2;

	data = fat_ptr_data(&ptr);
	for (int i = 0; i < 16; i++) {
		data[i] = i;
	}

	cr_assert_eq(chain_realloc(&ptr2, &ptr, 30), 0);
	cr_assert_eq(fat_ptr_len(&ptr2), 32);

	data2 = fat_ptr_data(&ptr2);
	data = fat_ptr_data(&ptr);
	for (int i = 0; i < 16; i++) {
		cr_assert_eq(data[i], i);
	}
	chain_free(&ptr2);

	global_sync_allocator_cleanup();
	thread_local_allocator_cleanup();
	heap_allocator_cleanup(&ha);
	heap_allocator_cleanup(&ha2);
}
Test(core, test_lock) {
	Lock l1 = LOCK();
	{ LockGuard lg01 = lock(&l1); }
	{ LockGuard lg11 = lock(&l1); }
}

Lock th_lock;
int counter = 0;

void *start_thread(void *data) {
	if (PANIC_RETURN()) {
		return NULL;
	}
	{
		LockGuard lg = lock(&th_lock);
		counter++;
		cr_assert_eq(counter, 1);
	}
	{
		LockGuard lg = lock(&th_lock);
		panic("panic start_thread");
	}

	return NULL;
}

Test(core, test_lock_panic) {
	pthread_t th;
	th_lock = LOCK();
	pthread_create(&th, NULL, &start_thread, NULL);
	pthread_join(th, NULL);
	bool isp = Lock_is_poisoned(&th_lock);
	cr_assert(isp);
	Lock_cleanup(&th_lock);
}

void start_thread_test(void *obj) {
	int v = *((int *)obj);
	cr_assert_eq(v, 101);
}

void start_thread_panic(void *obj) { panic("panic start_thread_panic"); }

Test(core, test_threads) {
	int y = 101;
	Thread th;
	Thread_start(&th, start_thread_test, &y);
	JoinResult jr = Thread_join(&th);
	cr_assert(!JoinResult_is_panic(&jr));
	cr_assert(!JoinResult_is_error(&jr));

	Thread th2;
	Thread_start(&th2, start_thread_panic, &y);
	JoinResult jr2 = Thread_join(&th2);
	cr_assert(JoinResult_is_panic(&jr2));
	cr_assert(!JoinResult_is_error(&jr2));
}

Lock cond_lock;
bool check_condition = false;
int cond_count = 0;

void cond_thread(void *obj) {
	int v = *((int *)obj);
	{
		LockGuard lg = lock(&cond_lock);
		while (!check_condition)
			Lock_wait(&cond_lock, 0);
		cond_count++;
	}
}

Test(core, test_wait_notify) {
	cond_lock = LOCK();
	int v = 1234;
	Thread th;
	Thread_start(&th, cond_thread, &v);
	check_condition = true;
	Lock_notify(&cond_lock);
	JoinResult jr = Thread_join(&th);
	cr_assert(cond_count == 1);
	cr_assert(!JoinResult_is_panic(&jr));
	cr_assert(!JoinResult_is_error(&jr));
	Lock_cleanup(&cond_lock);
}

Lock sync_lock;
bool sync_cond = false;
u64 sync_count = 0;

void sync_thread(void *obj) {
	// clang-format off
	synchronized(sync_lock, {
		while(!sync_cond)
			Lock_wait(&sync_lock, 0);
	})
	sync_count++;
	// clang-format on
}

Test(core, test_synchronized) {
	sync_lock = LOCK();
	Thread th;
	Thread_start(&th, sync_thread, NULL);
	sync_cond = true;
	Lock_notify(&sync_lock);
	JoinResult jr = Thread_join(&th);
	cr_assert_eq(sync_count, 1);
	cr_assert(!JoinResult_is_panic(&jr));
	cr_assert(!JoinResult_is_error(&jr));
	Lock_cleanup(&sync_lock);
}

Type(TestType, Field(u64, x), Field(u32, y));

#define AccessTestType(T)                                                      \
	Required(T, Const, u64, get_x) Required(T, Const, u32, get_y)
#define Drop(T) Required(T, Mut, void, drop)
#define Print(T)                                                               \
	Required(T, Const, void, print) Required(T, Mut, void, print_incr)
TraitImpl(Print, Const, void, print);
TraitImpl(Print, Mut, void, print_incr);
TraitImpl(Drop, Mut, void, drop);
TraitImpl(AcecssTestType, Const, u64, get_x);
TraitImpl(AcecssTestType, Const, u64, get_y);

Impl(TestType, Print);
Impl(TestType, Drop);
Impl(TestType, AccessTestType);

#define IMPL TestType
u64 TestType_get_x() { return $(x); }
u32 TestType_get_y() { return $(y); }
void TestType_print() { printf("x=%" PRIu64 ",y=%u\n", $(x), $(y)); }

void TestType_print_incr() {
	$Mut(x)++;
	$Mut(y)++;
	printf("x=%" PRIu64 ",y=%u\n", $(x), $(y));
}

void TestType_drop() { printf("drop with x=%" PRIu64 ",y=%u\n", $(x), $(y)); }
#undef IMPL

Test(core, test_type) {
	var v1 = new (TestType, With(x, 1), With(y, 5));
	print(&v1);
	cr_assert_eq(get_x(&v1), 1);
	cr_assert_eq(get_y(&v1), 5);
	print_incr(&v1);
	cr_assert_eq(get_x(&v1), 2);
	cr_assert_eq(get_y(&v1), 6);

	let v2 = new (TestType, With(x, 100), With(y, 500));
	print(&v2);
	cr_assert_eq(get_x(&v2), 100);
	cr_assert_eq(get_y(&v2), 500);

	// This line would be a compiler error due to the immutable access of a
	// mutable function.
	// print_incr(&v2);
}
