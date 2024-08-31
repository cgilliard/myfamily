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

/*
#define TEST1(arg1, x) [ arg1, x ]
#define PROC_TEST(...) FOR_EACH(TEST1, myarg, (;), __VA_ARGS__)
PROC_TEST(abc, def, ghi, aaa, zzz, mmm, xxx)
*/

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

// Define a Testtype with two fields x and y which are u64 and u32 respectively
Type(TestType, Field(u64, x), Field(u32, y));

// Define a trait with required functions which are fairly self explanitory.
// The first parameter in the Required macro is the mutability. It must be
// either Const or Var. The second parameter is the return type. The third
// parameter is the function name. After that there may be a variable number of
// Parameter arguments. The Param macro specifies these arguments with the type
// of the parameter only.
#define TestTrait                                                           \
	DefineTrait(TestTrait, Required(Const, u64, get_x),                 \
		    Required(Const, u32, get_y), Required(Var, void, incr), \
		    Required(Var, void, add_x, Param(u64)),                 \
		    Required(Var, void, sub_y, Param(u32)),                 \
		    Required(Var, u64, sub_both, Param(u64), Param(u32)))

// The call to TraitImpl is mandatory for all traits. It will generate the
// function calls for the trait. In this case, it generates the following
// functions: get_x, get_y, incr, add_x, sub_y, and sub_both.
TraitImpl(TestTrait);

// Specify that TestType implements the TestTrait trait. This will modify
// TestType's vtable and create function prototypes with the correct signature
// for each required function for the trait.
Impl(TestType, TestTrait);
// Specify that TestType implements the Drop trait. This trait is an internal
// trait that is called by the system automatically when the variable goes out
// of scope. Note that this function is a mutable function and may be called
// even on an immutable variable.
Impl(TestType, Drop);

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
		var v1 = new (TestType, With(x, 4), With(y, 5));
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
		let v2 = new (TestType, With(y, 50), With(x, 43));
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

// define implemented traits 'Build' and 'Drop'
Impl(TestServer, Build);
Impl(TestServer, Drop);

// Implement traits
#define IMPL TestServer

// do input validation and set defaults for our 'TestServer'
void TestServer_build()
{
	// Check if threads are equal to 0. This is a misconfiguration. Panic if
	// that's the case.
	if ($(threads) == 0)
		panic("Threads must be greater than 0!");

	// If host is NULL (not configured) use the default setting of
	// 127.0.0.1.
	if ($(host) == NULL)
	{
		$Var(host) = "127.0.0.1";
	}

	// If port is 0 (not configured) use the default setting of 80.
	if ($(port) == 0)
	{
		$Var(port) = 80;
	}

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

#define AccessTestMove                                                \
	DefineTrait(AccessTestMove, Required(Const, char*, get_tm_s), \
		    Required(Const, u64, get_tm_len))

TraitImpl(AccessTestMove);

Impl(TestMove, Drop);
Impl(TestMove, Build);
Impl(TestMove, AccessTestMove);

int tm_drop_count = 0;

#define IMPL TestMove
void TestMove_build()
{
	if ($(s) == NULL)
		panic("TestMove: s must not be NULL");
	char* s = malloc(sizeof(char) * (strlen($(s)) + 1));
	strcpy(s, $(s));
	$Var(s) = s;
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
		// would result in panic due to drop being called. Object is
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
Type(CompositeTest, Field(u64, x), Field(u32, y), Field(Object, z));

#define InnerValue DefineTrait(InnerValue, Required(Const, u64, inner_value))
TraitImpl(InnerValue);

Impl(InnerType, Drop);
Impl(InnerType, InnerValue);
Impl(CompositeTest, Drop);
Impl(CompositeTest, Build);

#define SetCompTrait                                                  \
	DefineTrait(SetCompTrait, Required(Var, void, set_comp_value, \
					   Param(const Object*)))

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
#undef IMPL

// TODO: create a macro 'Obj' which allows for conveinent initialization of
// Objects (Setting it to OBJECT_INIT. It would allow for a type param
// (and later traits)). It would also automatically call Object_cleanup when its
// own cleanup is called. Also review what 'with' is doing here. We need to call
// Move instead if possible.
#define IMPL CompositeTest
void CompositeTest_drop()
{
	printf("drop composite type type value(x) = %" PRIu64 "\n", $(x));
	printf("drop composite type type value(y) = %u\n", $(y));
	Object_cleanup(&$Var(z));
	comp_drops += 1;
}

void CompositeTest_set_comp_value(const Object* value)
{
	Move(&$Var(z), value);
}
void CompositeTest_build() { $Var(z) = OBJECT_INIT; }
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
					     Param(u64), Param(Object*)))
TraitImpl(AdvCompSetBoth);

Type(AdvComp, Field(u64, x), Obj(InnerType, holder));
Impl(AdvComp, SetCompTrait);
Impl(AdvComp, AdvCompSetBoth);

#define IMPL AdvComp
void AdvComp_set_comp_value(const Object* value)
{
	Move(&$Var(holder), value);
}
void AdvComp_set_both_adv(u64 v1, Object* ptr) {}
#undef IMPL

Test(core, test_obj_macro)
{
	inner_drops = 0;
	{
		var inner = new (InnerType, With(value, 123));
		{
			cr_assert_eq(inner_value(&inner), 123);
			// var advcomp1 = new (AdvComp, With(x, 0), With(holder,
			// inner));
			var advcomp1 =
			    new (AdvComp, With(x, 0), WithObj(holder, inner));
			// set_comp_value(&advcomp1, &inner);
			var inner2 = new (InnerType, With(value, 999));
			set_both_adv(&advcomp1, 1, &inner2);

			// would result in panic because inner has already been
			// consumed cr_assert_eq(inner_value(&inner), 123);
			cr_assert_eq(inner_drops, 0);
		}
		// assert that both of the inner type was dropped
		cr_assert_eq(inner_drops, 2);

		// would result in panic because inner has already been consumed
		// cr_assert_eq(inner_value(&inner), 123);
	}
	// check no double drops
	cr_assert_eq(inner_drops, 2);
}

Type(ServerConfig, Field(u32, threads), Field(u16, port), Field(char*, host));
Type(ServerTest2, Obj(ServerConfig, config), Field(u64, state), Field(i32, server_fd));

#define ServerImpl DefineTrait(ServerImpl, Required(Const, char*, get_server_host))
TraitImpl(ServerImpl);

Impl(ServerConfig, Build);
Impl(ServerConfig, Drop);

Impl(ServerTest2, Build);
Impl(ServerTest2, Drop);
Impl(ServerTest2, ServerImpl);

Getter(ServerConfig, threads);
Setter(ServerConfig, threads);

Getter(ServerConfig, host);
Setter(ServerConfig, host);

#define IMPL ServerConfig
void ServerConfig_build()
{
	if ($(threads) == 0)
		$Var(threads) = 10;
	if ($(host) == NULL)
		$Var(host) = "127.0.0.1";
}
void ServerConfig_drop()
{
	printf("sc drop\n");
}
#undef IMPL

#define IMPL ServerTest2
char* ServerTest2_get_server_host()
{
	return get(ServerConfig, $(config), host);
}
#undef IMPL

#define IMPL ServerTest2
void ServerTest2_build()
{
}
void ServerTest2_drop() { printf("drop servertest2\n"); }
#undef IMPL

Test(core, test_sample)
{
	var config1 = new (ServerConfig, With(threads, 1));
	var config2 = new (ServerConfig, With(host, "localhost"));

	cr_assert_eq(get(ServerConfig, config1, threads), 1);
	cr_assert_eq(get(ServerConfig, config2, threads), 10);

	set(ServerConfig, config1, threads, 3);
	cr_assert_eq(get(ServerConfig, config1, threads), 3);
	cr_assert(!strcmp(get(ServerConfig, config1, host), "127.0.0.1"));
	cr_assert(!strcmp(get(ServerConfig, config2, host), "localhost"));

	let server = new (ServerTest2, WithObj(config, config1));
	cr_assert(!strcmp(get_server_host(&server), "127.0.0.1"));
}
