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
