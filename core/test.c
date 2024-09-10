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
#include <core/test_encapsulation.h>
#include <core/test_server.h>
#include <criterion/criterion.h>

Test(core, test_heap)
{
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
	for (u64 i = 1; i < 40; i++)
	{
		cr_assert_eq(heap_allocator_allocate(&ha, i, &ptrs[i]), 0);
		if (i <= 8)
		{
			// id from first batch with no batch id
			cr_assert_eq(ptrs[i].id, i - 1);
			cr_assert_eq(ptrs[i].len, 8);
			last_size8_id = ptrs[i].id;
		}
		else if (i <= 16)
		{
			// id from the second batch with (1 << 56)
			cr_assert_eq(ptrs[i].id, (i - 9) | (0x1L << 56));
			cr_assert_eq(ptrs[i].len, 16);
			last_size16_id = ptrs[i].id;
		}
		else if (i <= 32)
		{
			// id from the third batch with (2 << 56)
			cr_assert_eq(ptrs[i].id, (i - 17) | (0x2L << 56));
			cr_assert_eq(ptrs[i].len, 32);
			last_size32_id = ptrs[i].id;
		}
		else
		{
			// these are allocating using malloc
			cr_assert_eq(ptrs[i].len, i);
			cr_assert_eq(ptrs[i].id, UINT64_MAX);
		}
	}

	// free all slabs
	for (u64 i = 1; i < 40; i++)
	{
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
	for (u64 i = 0; i < 13; i++)
	{
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

Test(core, test_heap_resize)
{
	cr_assert_eq(__malloc_count, __free_count);

	// setup an ha and 46 fat pointers
	FatPtr ptrs[46];
	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 1, 45};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	// allocate all slabs. The ha should resize twice and return ids in
	// sequential order
	for (u64 i = 0; i < 45; i++)
	{
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

Test(core, test_heap_free)
{
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

Test(core, test_heap_initial_size0)
{
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

	for (u64 i = 0; i < 45; i++)
	{
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

Test(core, test_heap_init_multi_chunk)
{
	cr_assert_eq(__malloc_count, __free_count);

	// setup ha with multi initial chunks
	FatPtr ptrs[81];
	HeapAllocator ha;
	HeapAllocatorConfig hconfig = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 2, 80};
	cr_assert_eq(heap_allocator_build(&ha, &hconfig, 1, hdconfig1), 0);

	for (u64 i = 0; i < 80; i++)
	{
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[i]), 0);
		cr_assert_eq(ptrs[i].id, i);
		cr_assert_eq(ptrs[i].len, 16);
	}

	// no more slabs
	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[80]), 0);
	cr_assert_eq(ptrs[80].id, UINT64_MAX);

	for (u64 i = 0; i < 81; i++)
	{
		cr_assert_eq(heap_allocator_free(&ha, &ptrs[i]), 0);
	}

	for (u64 i = 0; i < 80; i++)
	{
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptrs[i]), 0);
		cr_assert_eq(ptrs[i].id, 79 - i);
		cr_assert_eq(ptrs[i].len, 16);
	}

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_invalid_configurations)
{
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

Test(core, test_heap_oom)
{
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

	for (u64 i = 0; i < 10; i++)
	{
		cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	}

	cr_assert_neq(heap_allocator_allocate(&ha, 16, &ptr), 0);

	//__debug_build_allocator_malloc_fail4 = true;
	__debug_build_allocator_malloc_fail7 = false;

	// now we can create new slabs
	for (u64 i = 0; i < 10; i++)
	{
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

	for (u64 i = 0; i < 10; i++)
	{
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

Test(core, test_heap_zeroed)
{
	FatPtr ptr;
	HeapAllocator ha;
	HeapAllocatorConfig hconfig_zeroed = {true, true};
	HeapAllocatorConfig hconfig_non_zeroed = {false, false};
	HeapDataParamsConfig hdconfig1 = {16, 20, 1, 45};
	char* data;

	cr_assert_eq(__malloc_count, __free_count);

	// first try without zeroed
	cr_assert_eq(
	    heap_allocator_build(&ha, &hconfig_non_zeroed, 1, hdconfig1), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(fat_ptr_id(&ptr), 0);
	data = fat_ptr_data(&ptr);
	for (u64 i = 0; i < 16; i++)
	{
		data[i] = 1;
	}

	cr_assert_eq(heap_allocator_free(&ha, &ptr), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(fat_ptr_id(&ptr), 0);
	data = fat_ptr_data(&ptr);
	for (u64 i = 0; i < 16; i++)
	{
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
	for (u64 i = 0; i < 16; i++)
	{
		data[i] = 1;
	}

	cr_assert_eq(heap_allocator_free(&ha, &ptr), 0);

	cr_assert_eq(heap_allocator_allocate(&ha, 16, &ptr), 0);
	cr_assert_eq(fat_ptr_id(&ptr), 0);
	data = fat_ptr_data(&ptr);
	for (u64 i = 0; i < 16; i++)
	{
		// this time it's zeroed out
		cr_assert_eq(data[i], 0);
	}

	// cleanup ha
	heap_allocator_cleanup(&ha);

	cr_assert_eq(__malloc_count, __free_count);
}

Test(core, test_invalid_heap_configurations)
{
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

Test(core, test_panic)
{
	bool is_panic = false;
	if (PANIC_RETURN())
	{
		is_panic = true;
	}

	// first loop is not a panic
	if (panic_loop)
	{
		cr_assert(is_panic);
	}
	else
	{
		cr_assert(!is_panic);
	}
	panic_loop++;
	if (!is_panic)
		panic("panic test_panic");
}

Test(core, test_chained_allocator)
{
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
	for (int i = 0; i < 16; i++)
	{
		data[i] = i;
	}

	cr_assert_eq(chain_realloc(&ptr2, &ptr, 30), 0);
	cr_assert_eq(fat_ptr_len(&ptr2), 32);

	data2 = fat_ptr_data(&ptr2);
	data = fat_ptr_data(&ptr);
	for (int i = 0; i < 16; i++)
	{
		cr_assert_eq(data[i], i);
	}
	chain_free(&ptr2);

	global_sync_allocator_cleanup();
	thread_local_allocator_cleanup();
	heap_allocator_cleanup(&ha);
	heap_allocator_cleanup(&ha2);
}
Test(core, test_lock)
{
	Lock l1 = LOCK();
	{
		LockGuard lg01 = lock(&l1);
	}
	{
		LockGuard lg11 = lock(&l1);
	}
}

Lock th_lock;
int counter = 0;

void* start_thread(void* data)
{
	if (PANIC_RETURN())
	{
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

Test(core, test_lock_panic)
{
	pthread_t th;
	th_lock = LOCK();
	pthread_create(&th, NULL, &start_thread, NULL);
	pthread_join(th, NULL);
	bool isp = Lock_is_poisoned(&th_lock);
	cr_assert(isp);
	Lock_cleanup(&th_lock);
}

void start_thread_test(void* obj)
{
	int v = *((int*)obj);
	cr_assert_eq(v, 101);
}

void start_thread_panic(void* obj) { panic("panic start_thread_panic"); }

Test(core, test_threads)
{
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

void cond_thread(void* obj)
{
	int v = *((int*)obj);
	{
		LockGuard lg = lock(&cond_lock);
		while (!check_condition)
			Lock_wait(&cond_lock, 0);
		cond_count++;
	}
}

Test(core, test_wait_notify)
{
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

void sync_thread(void* obj)
{
	// clang-format off
	synchronized(sync_lock, {
		while(!sync_cond)
			Lock_wait(&sync_lock, 0);
	})
	sync_count++;
	// clang-format on
}

Test(core, test_synchronized)
{
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
Builder(TestType, Config(u64, x_in), Config(u32, y_in));
Type(TestType, Field(u64, x), Field(u32, y));

#define TestTrait                                                           \
	DefineTrait(TestTrait, Required(Const, u64, get_x),                 \
		    Required(Const, u32, get_y), Required(Var, void, incr), \
		    Required(Var, void, add_x, Param(u64)),                 \
		    Required(Var, void, sub_y, Param(u32)),                 \
		    Required(Var, u64, sub_both, Param(u64), Param(u32)))
TraitImpl(TestTrait);

Impl(TestType, Build);
Impl(TestType, Drop);
Impl(TestType, TestTrait);

// declare a drop count variable for testing
int drop_count = 0;

// The IMPL value defines which type is being implemented. It is required to use
// the $ and $Var macros which are used to access the internal data of the Type.
// If $Var is accessed in a function that is immutable (Const), a thread panic
// will occur.
#define IMPL TestType
// This function which is required by the TestTrait returns the value of x. It
// uses the $ (const) macro to access the value of x. Any function (mutable or
// immutable) may use the '$' operator. It cannot modify values as it the
// pointer it references is declared as 'const'.
u64 TestType_get_x() { return $(x); }
// Likewise for this similar required function.
u32 TestType_get_y() { return $(y); }
// This function is mutable and can therefore safely use the $Var macro to
// modify an internal value of the Type. In this case, it subtracts the passed
// in value from the value of y and sets the variable to this value.
void TestType_sub_y(u32 value) { $Var(y) -= value; }
// This mutable function increments both parameters.
void TestType_incr()
{
	$Var(x)++;
	$Var(y)++;
}
// This function is similar to the other mutable functions. It add updates x by
// value.
void TestType_add_x(u64 value) { $Var(x) += value; }
// This is the drop handler for this type. As mentioned above drop is a special
// function that is automatically called by the system to cleanup when the
// variable goes out of scope.
void TestType_drop() { drop_count++; }
// subtract the specified values from both fields. Return the value of x.
u64 TestType_sub_both(u64 value1, u32 value2)
{
	$Var(x) -= value1;
	$Var(y) -= value2;
	return $(x);
}
void TestType_build(const TestTypeConfig* config)
{
	$Var(x) = config->x_in;
	$Var(y) = config->y_in;
}
#undef IMPL
// This ends the implementation of TestType. The $ and $Var operators can no
// longer be used outside this block.
Test(core, test_type)
{
	{
		// declare a new instance of the TestType type using the 'new'
		// macro. This instance is delcared as 'var' so it may call
		// mutable methods of TestType. It is decleared with x and y
		// both set to the specified values.
		var v1 = new (TestType, With(x_in, 4), With(y_in, 5));
		// call the polymorphic version of get_x/get_y on the v1
		// instance. Assert the expected values.
		cr_assert_eq(get_x(&v1), 4);
		cr_assert_eq(get_y(&v1), 5);
		// call the polymorphic version of incr on the v1 instance.
		incr(&v1);
		// call the polymorphic version of get_x/get_y on the v1
		// instance. Assert the expected values.
		cr_assert_eq(get_x(&v1), 5);
		cr_assert_eq(get_y(&v1), 6);
		// call the polymorphic version of add_x on the v1 instance and
		// assert the expected values.
		add_x(&v1, 100);
		cr_assert_eq(get_x(&v1), 105);
		cr_assert_eq(get_y(&v1), 6);

		u64 ret = sub_both(&v1, 1, 3);
		cr_assert_eq(get_x(&v1), 104);
		cr_assert_eq(get_y(&v1), 3);
		cr_assert_eq(ret, 104);

		// declare a second instance of TestType with the specified
		// initial values and do the coresponding assertions.
		let v2 = new (TestType, With(y_in, 50), With(x_in, 43));
		cr_assert_eq(get_x(&v2), 43);
		cr_assert_eq(get_y(&v2), 50);

		// cannot do, not immutable
		// incr(&v2);
		// add_x(&v2, 10);

		// assert that the drop handler has not been called yet.
		cr_assert_eq(drop_count, 0);
	}

	// assert that the drop handler was called for both objects
	cr_assert_eq(drop_count, 2);
}

// Test the Build trait. If implemented, the 'Build' trait's build function is
// called when the new macro is executed. It can be used to validate
// configuration, set default values, and do any other initialization procedures
// for a type. Below is a simple example of a 'TestServer' Build trait that does
// some input validation and sets default values. The Build trait can only
// panic, but once we have Result implemented, we will also provide a 'TryBuild'
// trait which returns a result which will allow for additional capabilities.
Type(TestServer, Field(u16, port), Field(char*, host), Field(u16, threads));
Builder(TestServer, Config(u16, port), Config(char*, host), Config(u16, threads));

// define implemented traits 'Build' and 'Drop'
Impl(TestServer, Build);
Impl(TestServer, Drop);

// Implement traits
#define IMPL TestServer

// do input validation and set defaults for our 'TestServer'
void TestServer_build(const TestServerConfig* config)
{
	// Check if threads are equal to 0. This is a misconfiguration. Panic if
	// that's the case.
	if (config->threads == 0)
		panic("Threads must be greater than 0!");
	$Var(threads) = config->threads;

	// If host is NULL (not configured) use the default setting of
	// 127.0.0.1.
	if (config->host == NULL)
	{
		$Var(host) = "127.0.0.1";
	}
	else
		$Var(host) = config->host;

	// If port is 0 (not configured) use the default setting of 80.
	if (config->port == 0)
	{
		$Var(port) = 80;
	}
	else
		$Var(port) = config->port;

	// print out the configuration and return.
	printf("Calling build with host='%s',port=%u,threads=%u\n", $(host),
	       $(port), $(threads));
}

// The drop handler just prints out a message in this case, but could be used to
// deallocate resources/close connections/etc.
void TestServer_drop()
{
	printf("Calling drop with host='%s',port=%u,threads=%u\n", $(host),
	       $(port), $(threads));
}
#undef IMPL

Test(core, test_build)
{

	// Create three server instanes with varying settings and mutability.
	var server1 = new (TestServer, With(port, 8080),
			   With(host, "127.0.0.1"), With(threads, 10));
	let server2 = new (TestServer, With(port, 9000),
			   With(host, "localhost"), With(threads, 4));
	let server3 = new (TestServer, With(threads, 6), With(host, NULL));

	// Output of this test:
	// ./bin/test --timeout 60 -f
	// Calling build with host='127.0.0.1',port=8080,threads=10
	// Calling build with host='localhost',port=9000,threads=4
	// Calling build with host='127.0.0.1',port=80,threads=6
	// Calling drop with host='127.0.0.1',port=80,threads=6
	// Calling drop with host='localhost',port=9000,threads=4
	// Calling drop with host='127.0.0.1',port=8080,threads=10
}

Type(TestMove, Field(char*, s), Field(u64, len));
Builder(TestMove, Config(char*, s), Config(u64, len));

#define AccessTestMove                                                \
	DefineTrait(AccessTestMove, Required(Const, char*, get_tm_s), \
		    Required(Const, u64, get_tm_len))

TraitImpl(AccessTestMove);

Impl(TestMove, Drop);
Impl(TestMove, Build);
Impl(TestMove, AccessTestMove);

int tm_drop_count = 0;

#define IMPL TestMove
void TestMove_build(const TestMoveConfig* config)
{
	if (config->s == NULL)
		panic("TestMove: s must not be NULL");
	char* s = malloc(sizeof(char) * (strlen(config->s) + 1));
	strcpy(s, config->s);
	$Var(s) = s;
	$Var(len) = config->len;
}
void TestMove_drop()
{
	tm_drop_count++;
	free($(s));
}
u64 TestMove_get_tm_len() { return $(len); }
char* TestMove_get_tm_s() { return $(s); }
#undef IMPL

Test(core, test_move)
{
	var tm1 = new (TestMove, With(s, "test"), With(len, 4));
	cr_assert_eq(get_tm_len(&tm1), 4);
	cr_assert(!strcmp(get_tm_s(&tm1), "test"));
	var tm2 = OBJECT_INIT;
	Move(&tm2, &tm1);
	cr_assert_eq(get_tm_len(&tm2), 4);
	cr_assert(!strcmp(get_tm_s(&tm2), "test"));

	// would result in panic because tm1 has already been consumed.
	// cr_assert_eq(get_tm_len(&tm1), 4);

	var tm4 = OBJECT_INIT;
	var tm3 = new (TestMove, With(s, "abc"), With(len, 3));
	cr_assert_eq(get_tm_len(&tm3), 3);
	cr_assert(!strcmp(get_tm_s(&tm3), "abc"));
	Move(&tm4, &tm3);
	cr_assert_eq(get_tm_len(&tm4), 3);
	cr_assert(!strcmp(get_tm_s(&tm4), "abc"));

	// would result in panic because tm3 has already been consumed.
	// cr_assert_eq(get_tm_len(&tm3), 3);

	var tm5 = new (TestType);
	// this would result in a panic due to differing types
	// Move(&tm4, &tm5);

	// this is ok
	Move(&tm3, &tm4);
}

Test(core, test_use_after_drop)
{
	tm_drop_count = 0;
	{
		var tm1 = new (TestMove, With(s, "test"), With(len, 4));
		cr_assert_eq(get_tm_len(&tm1), 4);
		cr_assert_eq(tm_drop_count, 0);
		drop(&tm1);
		cr_assert_eq(tm_drop_count, 1);
		// would result in panic due to drop being called. Obj is
		// consumed.
		// cr_assert_eq(get_tm_len(&tm1), 4);
	}
	// assert drop not called again as tm1 goes out of scope
	cr_assert_eq(tm_drop_count, 1);
}

#define DEF_IMPL_TRAIT                                                        \
	DefineTrait(DEF_IMPL_TRAIT, Required(Const, u64, testx2),             \
		    RequiredWithDefault(my_default_testx1, Var, u64, testx1), \
		    Super(Drop))
TraitImpl(DEF_IMPL_TRAIT);
Impl(TestMove, DEF_IMPL_TRAIT);

// In default implementations $Var() (mutable reference available in mutable
// methods) and $() (immutable reference available in all methods) may be used
// as a reference to the unknown type for calling other methods within the trait
// or super traits.
u64 my_default_testx1()
{
	u64 ret = testx2($Var());
	return ret;
}

#define IMPL TestMove
u64 TestMove_testx2()
{
	return $(len);
}
#undef IMPL

Test(core, test_defaults)
{
	var tm1 = new (TestMove, With(s, "testing"), With(len, 7));
	u64 ret1 = testx1(&tm1);
	u64 ret2 = testx2(&tm1);
	cr_assert_eq(ret1, 7);
	cr_assert_eq(ret2, 7);
}

Type(InnerType, Field(u64, value));
Builder(InnerType, Config(u64, value));
Type(CompositeTest, Field(u64, x), Field(u32, y), Field(Obj, z));
Builder(CompositeTest, Config(u64, x), Config(u32, y));

#define InnerValue DefineTrait(InnerValue, Required(Const, u64, inner_value))
TraitImpl(InnerValue);

Impl(InnerType, Drop);
Impl(InnerType, InnerValue);
Impl(InnerType, Build);
Impl(CompositeTest, Drop);
Impl(CompositeTest, Build);

#define SetCompTrait                                                  \
	DefineTrait(SetCompTrait, Required(Var, void, set_comp_value, \
					   Param(const Obj*)))

TraitImpl(SetCompTrait);
Impl(CompositeTest, SetCompTrait);

int inner_drops = 0;
int comp_drops = 0;

#define IMPL InnerType
u64 InnerType_inner_value()
{
	return $(value);
}
void InnerType_drop()
{
	printf("drop inner type value = %" PRIu64 "\n", $(value));
	inner_drops += 1;
}
void InnerType_build(const InnerTypeConfig* config)
{
	$Var(value) = config->value;
}
#undef IMPL

// TODO: create a macro 'Object' which allows for conveinent initialization of
// Objs (Setting it to OBJECT_INIT. It would allow for a type param
// (and later traits)). It would also automatically call Obj_cleanup when its
// own cleanup is called. Also review what 'with' is doing here. We need to call
// Move instead if possible.
#define IMPL CompositeTest
void CompositeTest_drop()
{
	printf("drop composite type type value(x) = %" PRIu64 "\n", $(x));
	printf("drop composite type type value(y) = %u\n", $(y));
	Obj_cleanup(&$Var(z));
	comp_drops += 1;
}

void CompositeTest_set_comp_value(const Obj* value)
{
	Move(&$Var(z), value);
}
void CompositeTest_build(const CompositeTestConfig* config) { $Var(z) = OBJECT_INIT; }
#undef IMPL

Test(core, test_composites)
{
	{
		var comp = new (InnerType, With(value, 10));
		var comp2 = new (InnerType, With(value, 11));
		var ct1 = new (CompositeTest, With(x, 123), With(y, 333));
		// comp is moved over to ct1 transferring ownership
		set_comp_value(&ct1, &comp);
		// comp2 is moved over to ct1 overwriting comp, thus calling its
		// drop handler
		set_comp_value(&ct1, &comp2);
		// ct1 goes out of scope calling its drop handler which in turn
		// calls comp2's drop handler
	}
	cr_assert_eq(inner_drops, 2);
	cr_assert_eq(comp_drops, 1);
}

#define AdvCompSetBoth                                                \
	DefineTrait(AdvCompSetBoth, Required(Var, void, set_both_adv, \
					     Param(u64), Param(Obj*)))
TraitImpl(AdvCompSetBoth);

Type(AdvComp, Field(u64, x), Object(InnerType, holder));
Builder(AdvComp, Config(u64, x_in));
Impl(AdvComp, SetCompTrait);
Impl(AdvComp, AdvCompSetBoth);
Impl(AdvComp, Build);

#define IMPL AdvComp
void AdvComp_set_comp_value(const Obj* value)
{
	Move(&$Var(holder), value);
}
void AdvComp_set_both_adv(u64 v1, Obj* ptr)
{
	$Var(x) = v1;
	Move(&$Var(holder), ptr);
}
void AdvComp_build(const AdvCompConfig* config)
{
	$Var(x) = config->x_in;
}
#undef IMPL

Test(core, test_obj_macro)
{
	inner_drops = 0;
	{
		var inner = new (InnerType, With(value, 123));
		{
			cr_assert_eq(inner_value(&inner), 123);
			var advcomp1 = new (AdvComp, With(x_in, 0));
			// set_comp_value(&advcomp1, &inner);
			var inner2 = new (InnerType, With(value, 999));
			set_both_adv(&advcomp1, 1, &inner);

			// would result in panic because inner has already been
			// consumed cr_assert_eq(inner_value(&inner), 123);
			cr_assert_eq(inner_drops, 0);
		}
		// assert that both of the inner type was dropped
		printf("inner drops = %i\n", inner_drops);
		cr_assert_eq(inner_drops, 2);

		// would result in panic because inner has already been consumed
		// cr_assert_eq(inner_value(&inner), 123);
	}
	// check no double drops
	cr_assert_eq(inner_drops, 2);
}

Test(core, test_hidden)
{
	var hidden = new (Hidden, With(capacity, 1234));
	set_value(&hidden, 122);
	cr_assert_eq(get_value(&hidden), 122);
	cr_assert_eq(get_capacity_impl(&hidden), 1234);
}

Type(ServerComponent, Field(u64, value), Field(void*, ptr));
Builder(ServerComponent, Config(u64, value));

Impl(ServerComponent, Build);
Impl(ServerComponent, Drop);

int sc_drops = 0;

#define IMPL ServerComponent
void ServerComponent_build(const ServerComponentConfig* config)
{
	$Var(value) = config->value;
	$Var(ptr) = malloc(100);
}
void ServerComponent_drop()
{
	sc_drops++;
	free($Var(ptr));
}
#undef IMPL

Builder(Server, Config(u32, threads), Config(u16, port), Config(char*, host));
Type(Server, Field(u64, state), Field(bool, is_started), Field(ServerConfig, config), Object(ServerComponent, sc));

#define ServerApi DefineTrait(ServerApi, Required(Var, bool, start_test_server), Required(Const, bool, is_test_started))
TraitImpl(ServerApi);

Impl(Server, Build);
Impl(Server, Drop);
Impl(Server, ServerApi);

#define IMPL Server
void Server_drop() {}
void Server_build(const ServerConfig* config)
{
	$Var(config) = *config;
	$Var(state) = 0;
	$Var(sc) = new (ServerComponent, With(value, 10));
}
bool Server_start_test_server()
{
	if ($(is_started))
		return false;
	$Var(is_started) = true;
	$Var(state) += 1;
	return true;
}
bool Server_is_test_started() { return $(is_started); }
#undef IMPL

Test(core, test_sample)
{
	{
		var server = new (Server, With(port, 8080));
		cr_assert(!is_test_started(&server));
		start_test_server(&server);
		cr_assert(is_test_started(&server));
	}
	cr_assert_eq(sc_drops, 1);
}

// Define a basic type
Builder(TestOver, Config(u64, value));
Type(TestOver, Field(u64, value));

// define a trait with a single default implemented function
#define OverrideTest DefineTrait(OverrideTest, RequiredWithDefault(override_test_default, Const, u64, get_overt))
// call TraitImpl to declare and generate function calls
TraitImpl(OverrideTest);

// Require implementation of Build trait
Impl(TestOver, Build);
// Require implementation of OverrideTest trait for TestOver
Impl(TestOver, OverrideTest);

// define the default override test impl
u64 override_test_default() { return 100; }

// implement our override function
#define IMPL TestOver
u64 my_over_fn() { return $(value); }
void TestOver_build(const TestOverConfig* config)
{
	$Var(value) = config->value;
}
#undef IMPL

// call override macro to override with our own impl
Override(TestOver, get_overt, my_over_fn);

Test(core, test_override)
{
	// create a TestOver object
	let to = new (TestOver, With(value, 300));
	// assert that our override function is being used
	cr_assert_eq(get_overt(&to), 300);
}

Builder(TestFixedArr, Config(char*, buf), Config(u64, p1), Config(u32, p2), Config(u16, p3));
Type(TestFixedArr, Field(u64, p1), Field(char, buf[100]), Field(u32, p2), Field(u16, p3));

Impl(TestFixedArr, Build);

#define TestFixedArrImpl DefineTrait(             \
    TestFixedArrImpl,                             \
    Required(Const, void, get_buf, Param(char*)), \
    Required(Const, u64, get_p1),                 \
    Required(Const, u32, get_p2),                 \
    Required(Const, u16, get_p3))
TraitImpl(TestFixedArrImpl);
Impl(TestFixedArr, TestFixedArrImpl);

#define IMPL TestFixedArr
void TestFixedArr_build(const TestFixedArrConfig* config)
{
	u64 len = strlen(config->buf);
	if (len >= 100)
		len = 99;
	memcpy($Var(buf), config->buf, len);
	$Var(buf)[len] = 0;
	$Var(p1) = config->p1;
	$Var(p2) = config->p2;
	$Var(p3) = config->p3;
}
void TestFixedArr_get_buf(char* buf_out)
{
	u64 len = strlen($(buf));
	memcpy(buf_out, $(buf), len);
	buf_out[len] = 0;
}
u64 TestFixedArr_get_p1()
{
	return $(p1);
}
u32 TestFixedArr_get_p2()
{
	return $(p2);
}
u16 TestFixedArr_get_p3()
{
	return $(p3);
}
#undef IMPL

Test(core, test_fixed_array)
{
	let fa = new (TestFixedArr, With(buf, "testing123"), With(p1, 10), With(p2, 20), With(p3, 30));
	char buf[100];
	get_buf(&fa, buf);
	cr_assert(!strcmp(buf, "testing123"));
	cr_assert_eq(get_p1(&fa), 10);
	cr_assert_eq(get_p2(&fa), 20);
	cr_assert_eq(get_p3(&fa), 30);
}

// Define a trait which has a single required function as well as a required super-trait (Drop in this case).
#define SuperTest DefineTrait(SuperTest, Super(Drop), Required(Const, u64, get_super_value))
TraitImpl(SuperTest);

// Define another trait which has a single required function as well as a required super-trait (SuperTest in this case).
#define SuperDuperTest DefineTrait(SuperDuperTest, Super(SuperTest), Required(Const, u64, get_super_duper_value))
TraitImpl(SuperDuperTest);

// Create a new type 'SuperImpl'.
Type(SuperImpl, Field(u64, value));
Builder(SuperImpl, Config(u64, value));

// Implement SuperDuperTest trait (which in turn requires SuperTest, which in turn requires Drop). Additionally we'll implement 'Build'.
Impl(SuperImpl, Drop);
Impl(SuperImpl, SuperTest);
Impl(SuperImpl, SuperDuperTest);
Impl(SuperImpl, Build);

// Do implementations which require 4 functions total.
#define IMPL SuperImpl
void SuperImpl_build(const SuperImplConfig* config)
{
	$Var(value) = config->value;
}
void SuperImpl_drop()
{
}
u64 SuperImpl_get_super_value()
{
	return $(value);
}
u64 SuperImpl_get_super_duper_value()
{
	return $(value) + 1;
}
#undef IMPL

Test(core, test_super)
{
	// instantiate an instance of SuperImpl with the specified initial value.
	let super_test = new (SuperImpl, With(value, 10));

	// do assertions based on expected values.
	cr_assert_eq(get_super_value(&super_test), 10);
	cr_assert_eq(get_super_duper_value(&super_test), 11);
}

Type(NoDrop);
Builder(NoDrop);
Type(WithDrop);
Builder(WithDrop);

Impl(WithDrop, Drop);
Impl(WithDrop, Build);

#define IMPL WithDrop
void WithDrop_drop() { printf("Droping withdrop %p\n", $()); }
void WithDrop_build(const WithDropConfig* ptr) {}
#undef IMPL

Type(MyObject);
Type(XType, Where(T, TraitBound(Drop), TraitBound(Build)), Field(u64, x), Generic(T, wd));
Builder(XType);

#define XTypeApi DefineTrait(XTypeApi, Required(Var, void, set_wd_value, Param(Obj*)))
TraitImpl(XTypeApi);

Impl(XType, XTypeApi);

#define IMPL XType
void XType_set_wd_value(Obj* ptr)
{
	Move(&$Var(wd), ptr);
}
#undef IMPL

Type(MyObjectBuildDrop);
Builder(MyObjectBuildDrop);

Impl(MyObjectBuildDrop, Build);
Impl(MyObjectBuildDrop, Drop);

#define IMPL MyObjectBuildDrop
void MyObjectBuildDrop_drop() {}
void MyObjectBuildDrop_build(const MyObjectBuildDropConfig* config) {}
#undef IMPL

Test(core, test_where)
{
	var y = new (WithDrop);
	var x = new (XType);
	set_wd_value(&x, &y);
	var z = new (MyObjectBuildDrop);
	// this would panic because the binding to type 'WithDrop' has already occurred.
	// set_wd_value(&x, &z);
}

// Demonstrate a test of the test_server
Test(core, test_server)
{
	{
		printf("test_server\n");
		// create a mutable instance
		var server = new (HttpServer, With(threads, 2));
		// assert that it's not started.
		cr_assert(!is_started(&server));
		// start the server.
		cr_assert(start_server(&server));
		// now assert that the server has started.
		cr_assert(is_started(&server));
		// a second attempt to start the server will result in a false return value.
		cr_assert(!start_server(&server));
		// we print a couple of messages to show that the drop handlers are called at
		// the right time.
		printf("end scope, begin drop handlers\n");
	}
	printf("drop handlers should be complete.\n");
}

// Output:
// ./bin/test --timeout 60 -f
// test_server
// building http server component. Alloc 0x60000163c000
// Server started on 127.0.0.1:8080 with 2 threads
// -----------------------------------------------------
// Current stats: is_started: 1
// -----------------------------------------------------
// end scope, begin drop handlers
// dropping http server component. Free 0x60000163c000
// dropping can drop
// dropping http server
// drop handlers should be complete.

Type(EqTest, Field(u64, value));
Builder(EqTest, Config(u64, value));

Impl(EqTest, Build);
Impl(EqTest, Equal);

#define IMPL EqTest
void EqTest_build(const EqTestConfig* config)
{
	$Var(value) = config->value;
}
bool EqTest_equal(const Obj* rhs)
{
	return $(value) == $Context(rhs, EqTest, value);
}
#undef IMPL

Type(EqTest2, Field(u64, value));
Builder(EqTest2, Config(u64, value));

Impl(EqTest2, Build);
Impl(EqTest2, Equal);

#define IMPL EqTest2
void EqTest2_build(const EqTest2Config* config)
{
	$Var(value) = config->value;
}
bool EqTest2_equal(const Obj* rhs)
{
	return $(value) == $Context(rhs, EqTest2, value);
}
#undef IMPL

Test(core, test_equal)
{
	let x = new (EqTest, With(value, 1));
	let y = new (EqTest, With(value, 1));
	let z = new (EqTest, With(value, 2));
	cr_assert(equal(&x, &y));
	cr_assert(!equal(&x, &z));

	let a = new (EqTest2, With(value, 7));
	let b = new (EqTest2, With(value, 7));
	let c = new (EqTest2, With(value, 8));
	cr_assert(equal(&a, &b));
	cr_assert(!equal(&a, &c));

	// would cause panic because mismatched types.
	// cr_assert(!equal(&a, &x));
}

// Define a type for testing the Clone trait
Type(CloneTest, Field(u64, value));
Builder(CloneTest, Config(u64, value));

// Implement Build/Drop/Clone
Impl(CloneTest, Build);
Impl(CloneTest, Clone);
Impl(CloneTest, Drop);

// Define a trait for incrementing the value so we can verify clones are independent.
#define IncrTrait DefineTrait(IncrTrait, Required(Var, void, incr_value))
TraitImpl(IncrTrait);
Impl(CloneTest, IncrTrait);

// variable to count calls to the drop handler at specific points in execution.
int clone_test_drop_count = 0;

#define IMPL CloneTest
// standard build to allow setting of the internal value.
void CloneTest_build(const CloneTestConfig* config)
{
	$Var(value) = config->value;
}
// standard drop just to count drops.
void CloneTest_drop()
{
	clone_test_drop_count++;
}

// implement clone
Obj CloneTest_klone()
{
	// create a new instance with the same internal value.
	let ret = new (CloneTest, With(value, $(value)));
	// call the ReturnObj macro to properly handle returning of an Object
	// this includes the handling of marking the previous value for proper
	// cleanup and conumption flags. It also does the actual return statement
	// so this is the last statement needed in the function.
	ReturnObj(ret);
}
// simple function to increment the internal value.
void CloneTest_incr_value()
{
	$Var(value)++;
}
#undef IMPL

Test(core, test_clone)
{
	{
		// create a CloneTest with value of 100.
		var x = new (CloneTest, With(value, 100));
		// clone to y.
		let y = klone(&x);
		// assert that y's internal value is also 100.
		cr_assert_eq($Context((&y), CloneTest, value), 100);
		// clone to z.
		let z = klone(&y);
		// assert value for z
		cr_assert_eq($Context((&z), CloneTest, value), 100);
		// ensure no drops occurred
		cr_assert_eq(clone_test_drop_count, 0);
		// increment x's value
		incr_value(&x);
		// do assertions to verify values are independent from one another.
		cr_assert_eq($Context((&x), CloneTest, value), 101);
		cr_assert_eq($Context((&y), CloneTest, value), 100);
		cr_assert_eq($Context((&z), CloneTest, value), 100);
		// create an uninitialized instance of a. This is needed so that clone_from
		// can be called. Without this, behavior is undefined.
		var a = new (CloneTest);
		// use the default implemented clone_from function.
		clone_from(&a, &x);
		// do assertions to verify clone occurred.
		cr_assert_eq($Context((&x), CloneTest, value), 101);
		cr_assert_eq($Context((&a), CloneTest, value), 101);
		// initial 'a' dropped when we call clone_from
		cr_assert_eq(clone_test_drop_count, 1);
	}
	// we have four objects, but also the initial value for a was consumed and
	// cleanup/drop called when we called clone_from so 5 drops occurred.
	cr_assert_eq(clone_test_drop_count, 5);
}

// Implement a rudimentary Option. This will be improved once we have Enumerations implemented, but for now
// this will demonstrate the Iterator trait.
Type(SimpleOption, Field(bool, is_some), Field(int, value));
Builder(SimpleOption, Config(bool, is_some), Config(int, value));

// implement Build
Impl(SimpleOption, Build);

// define an accessor trait
#define SimpleOptionImpl DefineTrait(SimpleOptionImpl, Required(Const, int, option_value), Required(Const, bool, is_some))
TraitImpl(SimpleOptionImpl);

// implement it in SimpleOption
Impl(SimpleOption, SimpleOptionImpl);

// Do implementation (fairly self explanitory)
#define IMPL SimpleOption
void SimpleOption_build(const SimpleOptionConfig* config)
{
	$Var(is_some) = config->is_some;
	$Var(value) = config->value;
}
bool SimpleOption_is_some() { return $(is_some); }
int SimpleOption_option_value() { return $(value); }
#undef IMPL

// Implement a type that implements Iterator. This will be a simple int array
// iterator.
Type(IttTest, Field(int*, arr), Field(u64, len), Field(u64, cur));
Builder(IttTest, Config(int*, arr), Config(u64, len));

Impl(IttTest, Build);
Impl(IttTest, Iterator);

#define IMPL IttTest
// builder initializes values
void IttTest_build(const IttTestConfig* config)
{
	$Var(arr) = config->arr;
	$Var(len) = config->len;
	$Var(cur) = 0;
}

// next reutrns a 'next' value until 'cur' is greater than or equal to 'len'.
Obj IttTest_next()
{
	if ($(cur) >= $(len))
	{
		// We've finished iterating return 'None'.
		let ret = new (SimpleOption, With(is_some, false));
		ReturnObj(ret);
	}
	// Get the next value and create a SimpleOption instance to return
	let ret = new (SimpleOption, With(is_some, true), With(value, $(arr)[$(cur)]));
	$Var(cur)++;	// increment the counter
	ReturnObj(ret); // return our Option
}
#undef IMPL

// Iterator test
Test(core, test_iterator)
{
	// create an array with 10 elements
	int myarr[10];
	// initialize them to <index> + 10
	for (int i = 0; i < 10; i++)
		myarr[i] = i + 10;
	// create an IttTest with our array.
	var itt = new (IttTest, With(arr, myarr), With(len, 10));
	// initialize the couter to 0.
	int counter = 0;

	// loop until we get 'None'
	loop
	{
		// get the next value for this iterator
		let v = next(&itt);
		// if it's 'None', break
		if (!is_some(&v))
			break;
		// we got an actual value. Assert it's <index> + 10
		cr_assert_eq(option_value(&v), counter + 10);

		// increment our counter
		counter++;
	}

	// assert that there were 10 items returned
	cr_assert_eq(counter, 10);
}

#define TEST_BOX(type, value) ({   \
	type v_in = value;         \
	let v_box = Box(v_in);     \
	type v_out;                \
	Unbox(v_box, v_out);       \
	cr_assert_eq(v_in, v_out); \
})

#define TEST_BOX2(type, value) ({  \
	type v_in = value;         \
	let v_box = Box2(v_in);    \
	type v_out;                \
	Unbox(v_box, v_out);       \
	cr_assert_eq(v_in, v_out); \
})

Test(core, test_prim)
{
	TEST_BOX(i8, 10);
	TEST_BOX(i16, -12);
	TEST_BOX(i32, -11);
	TEST_BOX(i64, 1234);
	TEST_BOX(i128, 10);
	TEST_BOX(u8, 10);
	TEST_BOX(u16, 12);
	TEST_BOX(u32, 11);
	TEST_BOX(u64, 1234);
	TEST_BOX(u128, 10);
	TEST_BOX(f32, 11.3);
	TEST_BOX(f64, 1234.4);
	TEST_BOX(bool, false);
	TEST_BOX(bool, true);

	TEST_BOX2(i8, 10);
	TEST_BOX2(i16, -12);
	TEST_BOX2(i32, -11);
	TEST_BOX2(i64, 1234);
	TEST_BOX2(i128, 10);
	TEST_BOX2(u8, 10);
	TEST_BOX2(u16, 12);
	TEST_BOX2(u32, 11);
	TEST_BOX2(u64, 1234);
	TEST_BOX2(u128, 10);
	TEST_BOX2(f32, 11.3);
	TEST_BOX2(f64, 1234.4);
	TEST_BOX2(bool, false);
	TEST_BOX2(bool, true);

	i32 vb2 = 12;
	let v = Box2(vb2);
	i32 v_out;
	Unbox(v, v_out);
	cr_assert_eq(v_out, 12);
	let v2 = new (SimpleOption, With(value, 7));
	let v2_box = Box2(v2);
	i32 v2_out = option_value(&v2_box);
	cr_assert_eq(v2_out, 7);

	let x1 = Box(100);
	let x2 = Box(100);
	let x3 = Box(200);
	cr_assert(equal(&x1, &x2));
	cr_assert(!equal(&x1, &x3));
	cr_assert(!equal(&x2, &x3));
}

// Create a simple Type that we can use to test Rc.
Type(TestRc, Field(i32, value));
Builder(TestRc, Config(i32, value));

// Implement Drop and Build and IncrTrait to demonstrate Rc
Impl(TestRc, Drop);
Impl(TestRc, Build);
Impl(TestRc, IncrTrait);

// intialize drop count to 0.
int test_rc_drop_count = 0;

// do the implementation
#define IMPL TestRc
void TestRc_build(const TestRcConfig* config) { $Var(value) = config->value; }
void TestRc_drop() { test_rc_drop_count++; }
void TestRc_incr_value() { $Var(value)++; }
#undef IMPL

// test rc
Test(core, test_rc)
{
	// first test without unwrapping any rcs. With two rcs only a single drop should occur.
	{
		let v1 = new (TestRc, With(value, 1234));
		var rc1 = new (Rc, With(value, &v1));
		var rc2 = klone(&rc1);
		cr_assert_eq(test_rc_drop_count, 0);
	}
	cr_assert_eq(test_rc_drop_count, 1);

	// next test with one unwrap. Still just a single drop should occur.
	// also test multiple klones.
	{
		let v1 = new (TestRc, With(value, 5678));
		var rc1 = new (Rc, With(value, &v1));
		var rc2 = klone(&rc1);
		let rc3 = klone(&rc2);
		let rc4 = klone(&rc1);

		let v = unwrap(&rc1);
		cr_assert_eq($Context(&v, TestRc, value), 5678);

		cr_assert_eq(test_rc_drop_count, 1);
	}

	cr_assert_eq(test_rc_drop_count, 2);

	// finally test with both rcs unwrapped.
	{
		let v1 = new (TestRc, With(value, 9999));
		var rc1 = new (Rc, With(value, &v1));
		var rc2 = klone(&rc1);

		var v = unwrap(&rc1);
		cr_assert_eq($Context(&v, TestRc, value), 9999);

		// modify the value in 'v', this should be visible in 'x' below as well.
		incr_value(&v);

		let x = unwrap(&rc2);
		// verify the increment occurred.
		cr_assert_eq($Context(&x, TestRc, value), 10000);

		cr_assert_eq(test_rc_drop_count, 2);
	}
	cr_assert_eq(test_rc_drop_count, 3);
}

Test(core, test_enum_encap)
{
	// declare variables for testing purposes
	u32 v_out;
	u64 v_out64;
	u64 v2 = 1000;
	var hidden = new (Hidden, With(capacity, 1234));
	set_value(&hidden, 101);
	printf("value=%" PRIu64 "\n", get_value(&hidden));

	// create two instances of our Enum with appropriate values.
	let var1 = _(HiddenEnum, HiddenVar1, 123);
	let var2 = _(HiddenEnum, HiddenVar2, v2);
	let var3 = _obj(HiddenEnum, HiddenVar3, hidden);

	// match on var1. Create a U32 of the specified value.
	let m1 = match(var1, (HiddenVar1, new (U32, With(value, 1))), (HiddenVar2, new (U32, With(value, 2))), (new (U32, With(value, 3))));
	Unbox(m1, v_out);	// unbox
	cr_assert_eq(v_out, 1); // we should have 1.

	// match on var2. Create a U32 of the specified value.
	let m2 = match(var2, (HiddenVar1, new (U32, With(value, 1))), (HiddenVar2, new (U32, With(value, 2))), (new (U32, With(value, 3))));
	Unbox(m2, v_out);	// unbox
	cr_assert_eq(v_out, 2); // we should have 2.

	// match on var3. Create a U32 of the specified value. (default match).
	let m3 = match(var3, (HiddenVar1, new (U32, With(value, 1))), (HiddenVar2, new (U32, With(value, 2))), (new (U32, With(value, 3))));
	Unbox(m3, v_out);	// unbox
	cr_assert_eq(v_out, 3); // we should have 3.

	// mutable match (demonstration of mutable match
	let m4 = match(var3, (HiddenVar1, new (U64, With(value, 0))), (HiddenVar2, new (U64, With(value, 0))), (HiddenVar3, mut v, {
			       u64 cur = get_value(&v);
			       set_value(&v, 999);
			       new (U64, With(value, cur));
		       }));
	Unbox(m4, v_out64);
	cr_assert_eq(v_out64, 101);

	// read the value now
	let m5 = match(var3, (HiddenVar1, new (U64, With(value, 0))), (HiddenVar2, new (U64, With(value, 0))), (HiddenVar3, v, {
			       u64 cur = get_value(&v);
			       new (U64, With(value, cur));
		       }));

	// unbox and assert that it's changed to the updated value.
	Unbox(m5, v_out64);
	cr_assert_eq(v_out64, 999);

	// This would result in a panic because hidden has already been consumed
	// v_out64 = get_value(&hidden);
}

// Create a helper type for testing enums
Builder(MutTest, Config(i32, value));
Type(MutTest, Field(i32, value));
Impl(MutTest, Build);
Impl(MutTest, IncrTrait);
Impl(MutTest, ValueOf);

#define IMPL MutTest
void MutTest_build(const MutTestConfig* config)
{
	$Var(value) = config->value;
}
void MutTest_incr_value()
{
	$Var(value)++;
}
void MutTest_value_of(void* dst)
{
	*(i32*)dst = $(value);
}
#undef IMPL

// Create our main enum
Enum(PetsEnum, (bird, i32), (cat, u64), (dog, SimpleOption), (snake, bool), (hamster, MutTest));
EnumImpl(PetsEnum);

// The trait system can be used by the Enums just like Types. This enum will implement the Equal trait.
Impl(PetsEnum, Equal);

// Implement the trait
#define IMPL PetsEnum
bool PetsEnum_equal(const Obj* rhs)
{
	// If the variant ids are not equal we know they are not equal
	if (variant_id($()) != variant_id(rhs))
		return false;

	// The variant ids are equal so get the value of the right hand side.
	let vrhs = as_ref(rhs);

	// Match on self. We know the rhs is also the same variant.
	let x = match(*($()), (bird, v, new (Bool, With(value, equal(&v, &vrhs)))),
		      (cat, v, new (Bool, With(value, equal(&v, &vrhs)))),
		      (dog, v, {
			      // Dogs (SimpleOption) does not implement Equal so we have to use another method.
			      // In this case, we call 'value_of' and compare.
			      i32 self_i32;
			      i32 rhs_i32;
			      value_of(&v, &self_i32);
			      value_of(&vrhs, &rhs_i32);
			      // return true if the values are equal, otherwise return false
			      new (Bool, With(value, rhs_i32 == self_i32));
		      }),
		      (snake, v, new (Bool, With(value, equal(&v, &vrhs)))), (hamster, v, {
			      // hamster also does not implement equal, so use the value_of trait implementation
			      // to compare the underlying values.
			      i32 self_i32;
			      i32 rhs_i32;
			      value_of(&v, &self_i32);
			      value_of(&vrhs, &rhs_i32);
			      new (Bool, With(value, rhs_i32 == self_i32));
		      }));

	// The returned value in 'x' is our return value. Unbox it and return it.
	bool ret;
	Unbox(x, ret);
	return ret;
}
#undef IMPL

Test(core, test_enum)
{
	// create some enums of various types.
	let bird1 = _(PetsEnum, bird, 10);
	let bird2 = _(PetsEnum, bird, 10);
	u64 c1 = 13;
	u64 c2 = 30;
	u64 c3 = 30;
	let cat1 = _(PetsEnum, cat, c1);
	let cat2 = _(PetsEnum, cat, c2);
	let cat3 = _(PetsEnum, cat, c3);
	let simple_option = new (SimpleOption, With(is_some, true), With(value, 123));
	let dog1 = _obj(PetsEnum, dog, simple_option);
	let mut_test = new (MutTest, With(value, 100));
	let ham1 = _obj(PetsEnum, hamster, mut_test);
	let mut_test2 = new (MutTest, With(value, 100));
	let ham2 = _obj(PetsEnum, hamster, mut_test2);

	cr_assert(!equal(&cat1, &cat3));
	cr_assert(equal(&cat2, &cat3));
	cr_assert(!equal(&cat2, &dog1));
	cr_assert(equal(&ham1, &ham2));
	cr_assert(!equal(&dog1, &bird1));
	cr_assert(equal(&bird1, &bird2));
}
