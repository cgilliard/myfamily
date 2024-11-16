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

#include <base/test.h>

Suite(base);

Test(test_colors) {
	byte buf[1024];
	int len;

	len = sprint(buf, 1024, "Colors: %sgreen%s %sdimmed%s %sred%s %smagenta%s",
				 GREEN, RESET, DIMMED, RESET, RED, RESET, MAGENTA, RESET);
	fam_assert_eq(len, 67);
	_debug_no_color__ = true;
	len = sprint(buf, 1024,
				 "Colors: %sgreen%s %sdimmed%s %sred%s %smagenta%s %sbred%s "
				 "%syellow%s %scyan%s %sblue%s",
				 GREEN, RESET, DIMMED, RESET, RED, RESET, MAGENTA, RESET,
				 BRIGHT_RED, RESET, YELLOW, RESET, CYAN, RESET, BLUE, RESET);
	fam_assert_eq(len, 54);
	test_reset_colors();
}

Test(test_fam_err) {
	fam_err = NoErrors;
	fam_assert_eq(fam_err, NoErrors);
	SetErr(IllegalArgument);
	fam_assert_eq(fam_err, IllegalArgument);
	const byte *err = get_err();
	fam_assert(!cstring_compare(err, "IllegalArgument"));
	_debug_print_util_disable__ = true;
	print("test");
	print_err("test");
	panic("test");
	_debug_print_util_disable__ = false;
}

Test(test_lock) {
	Lock l1 = lock();
	lock_read(&l1);
	// upper bits incremented by 1.
	fam_assert_eq(l1 >> 32, 1);
	// lower bits also incremeneted by 1.
	fam_assert_eq(l1 & 0xFFFFFFFF, 1);

	lock_unlock(&l1);
	// upper bits incremented by 1.
	fam_assert_eq(l1 >> 32, 2);
	// lower bits also decremented by 1.
	fam_assert_eq(l1 & 0xFFFFFFFF, 0);

	lock_write(&l1);
	// upper bits incremented by 2.
	fam_assert_eq(l1 >> 32, 4);
	// lower bits high bit set
	fam_assert_eq(l1 & 0xFFFFFFFF, 0x80000000);

	lock_unlock(&l1);

	// upper bits incremented by 1.
	fam_assert_eq(l1 >> 32, 5);
	// lower bits are set to 0 now
	fam_assert_eq(l1 & 0xFFFFFFFF, 0x00000000);
}

Test(test_lock_macros) {
	int x = 0;
	Lock l = INIT_LOCK;
	wsync(l, { x++; });
	fam_assert(x == 1);
	int y;
	rsync(l, { y = x; });
	fam_assert(y == 1);
}

Test(test_mmap) {
	int *test = mmap_allocate(sizeof(int *) * 100);
	for (int i = 0; i < 100; i++) test[i] = i;
	for (int i = 0; i < 100; i++) fam_assert_eq(test[i], i);

	mmap_free(test, sizeof(int *) * 100);
}

Test(test_misc) {
	fam_assert(!cstring_compare_n("test1234", "test4566", 4));
	byte buf[10] = {};
	set_bytes(buf, 'a', 10);
	for (int i = 0; i < 10; i++) fam_assert_eq('a', buf[i]);
}

Test(test_memmap) {
	MemMap mm1;
	int size = 64;
	int count = 3000;
	memmap_init(&mm1, size);

	for (int i = 0; i < count; i++) {
		Ptr p = memmap_allocate(&mm1);
		if (p != i + 2) println("p=%u,i+1=%i", p, i + 2);
		fam_assert_eq(p, i + 2);
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			data[j] = 'a' + (j % 26);
		}
	}

	for (int i = 0; i < count; i++) {
		Ptr p = i + 2;
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], 'a' + (j % 26));
		}
	}

	memmap_free(&mm1, 7);
	memmap_free(&mm1, 8);
	memmap_free(&mm1, 167);

	Ptr p = memmap_allocate(&mm1);
	fam_assert_eq(p, 7);
	p = memmap_allocate(&mm1);
	fam_assert_eq(p, 8);
	p = memmap_allocate(&mm1);
	fam_assert_eq(p, 167);

	p = memmap_allocate(&mm1);
	fam_assert_eq(p, count + 2);

	memmap_cleanup(&mm1);
}

static int alloc_size = 64;
static int size = 1024 * 10;
static int count = 50;

Test(test_memmap_recycle) {
	MemMap mm1;
	memmap_init(&mm1, alloc_size);
	Slab slabs[count];

	for (long long i = 0; i < size; i++) {
		for (long long j = 0; j < count; j++) {
			slabs[j] = memmap_allocate_slab(&mm1);
			for (int k = 0; k < alloc_size; k++)
				((byte *)slabs[j].data)[k] = 'a' + ((i + j) % 26);
		}

		for (long long j = 0; j < count; j++) {
			for (int k = 0; k < alloc_size; k++)
				fam_assert_eq(((byte *)slabs[j].data)[k], 'a' + ((i + j) % 26));
			memmap_free(&mm1, slabs[j].ptr);
		}
	}
	memmap_cleanup(&mm1);
}

Test(test_slab_allocator_recycle) {
	SlabAllocator sa1;

	fam_assert(!slab_allocator_init(&sa1, alloc_size, count + 5, count + 5));
	Ptr ptrs[count];

	for (long long i = 0; i < size; i++) {
		for (long long j = 0; j < count; j++) {
			ptrs[j] = slab_allocator_allocate(&sa1);
			byte *data = slab_get(&sa1, ptrs[j]);
			for (int k = 0; k < alloc_size; k++) data[k] = 'a' + ((i + j) % 26);
		}

		for (long long j = 0; j < count; j++) {
			byte *data = slab_get(&sa1, ptrs[j]);
			for (int k = 0; k < alloc_size; k++)
				fam_assert_eq(data[k], 'a' + ((i + j) % 26));
			slab_allocator_free(&sa1, ptrs[j]);
		}
	}

	slab_allocator_cleanup(&sa1);
}

#include <stdlib.h>

Test(test_malloc_recycle) {
	byte *slabs[count];

	for (long long i = 0; i < size; i++) {
		for (long long j = 0; j < count; j++) {
			slabs[j] = malloc(alloc_size);
			for (int k = 0; k < alloc_size; k++) {
				slabs[j][k] = 'a' + (k + j) % 26;
			}
		}

		for (long long j = 0; j < count; j++) {
			for (int k = 0; k < alloc_size; k++) {
				fam_assert_eq(slabs[j][k], 'a' + (k + j) % 26);
			}
			free(slabs[j]);
		}
	}
}

Test(test_memmap_cap_exceed) {
	memmap_reset();
	int size = 64;
	for (int i = 0; i < MAX_MEMMAPS + 1; i++) {
		MemMap mm1;
		fam_assert(!memmap_init(&mm1, size));
		memmap_cleanup(&mm1);
	}
	MemMap mm1;
	fam_assert(memmap_init(&mm1, size));
	memmap_reset();
}

Test(test_large_memmap_kval) {
	MemMap mm1;
	int size = 64;
	int count = 10;
	int itt = 100;
	Ptr ptrs[itt];
	memmap_init(&mm1, size);
	memmap_setijkl(0, 31, 511, 511, 7);
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < itt; j++) {
			ptrs[j] = memmap_allocate(&mm1);
			byte *data = memmap_data(&mm1, ptrs[j]);
			data[0] = 'a' + (j % 26);
		}

		for (int j = 0; j < itt; j++) {
			byte *data = memmap_data(&mm1, ptrs[j]);
			fam_assert_eq(data[0], 'a' + (j % 26));
			memmap_free(&mm1, ptrs[j]);
		}
	}
	mmemap_force_cleanup(&mm1);
}

Test(test_big_memmap) {
	memmap_reset();
	MemMap mm1;
	// int size = 8;
	// unsigned long long count = (1024ULL * 1024ULL * 1024ULL * 4ULL) - 2;
	int size = 128;
	unsigned long long count = 100ULL * 1000ULL;
	memmap_init(&mm1, size);
	Ptr *ptrs = mmap_allocate(count * sizeof(Ptr));

	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm1);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm1, ptrs[i]);
	}

	mmap_free(ptrs, count * sizeof(Ptr));
	memmap_cleanup(&mm1);
}

Test(test_mmap_fail) {
	set_mmap_fail(0);
	void *tmp = mmap_allocate(100);
	fam_assert_eq(tmp, NULL);

	tmp = mmap_allocate(100);
	fam_assert(tmp != NULL);
	mmap_free(tmp, 100);
}

Test(test_memmap_cas_fail) {
	MemMap mm1;
	int size = 128;
	unsigned long long count = 1ULL;
	memmap_init(&mm1, size);

	Ptr *ptrs = mmap_allocate(count * sizeof(Ptr));
	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm1);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm1, ptrs[i]);
	}

	memmap_cleanup(&mm1);

	memmap_init(&mm1, size);

	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm1);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm1, ptrs[i]);
	}
	memmap_cleanup(&mm1);

	memmap_init(&mm1, size);

	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm1);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm1, ptrs[i]);
	}
	memmap_cleanup(&mm1);

	memmap_init(&mm1, size);

	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm1);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm1, ptrs[i]);
	}
	memmap_cleanup(&mm1);

	mmap_free(ptrs, count * sizeof(Ptr));
}

Test(test_memmap_alloc_fail) {
	MemMap mm1;
	int size = 128;
	unsigned long long count = 100ULL;
	memmap_init(&mm1, size);

	set_mmap_fail(1);
	fam_assert(!memmap_allocate(&mm1));
	fam_assert_eq(fam_err, AllocErr);

	Ptr *ptrs = mmap_allocate(count * sizeof(Ptr));
	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm1);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm1, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm1, ptrs[i]);
	}

	mmap_free(ptrs, count * sizeof(Ptr));
	memmap_cleanup(&mm1);

	fam_err = NoErrors;
	MemMap mm2;
	memmap_init(&mm2, size);
	set_mmap_fail(0);
	fam_assert(!memmap_allocate(&mm2));
	fam_assert_eq(fam_err, AllocErr);

	ptrs = mmap_allocate(count * sizeof(Ptr));
	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm2);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm2, ptrs[i]);
	}

	mmap_free(ptrs, count * sizeof(Ptr));
	memmap_cleanup(&mm2);

	fam_err = NoErrors;
	memmap_init(&mm2, size);
	set_mmap_fail(2);
	fam_assert(!memmap_allocate(&mm2));
	fam_assert_eq(fam_err, AllocErr);

	ptrs = mmap_allocate(count * sizeof(Ptr));
	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm2);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm2, ptrs[i]);
	}

	mmap_free(ptrs, count * sizeof(Ptr));
	memmap_cleanup(&mm2);

	fam_err = NoErrors;
	memmap_init(&mm2, size);
	set_mmap_fail(3);
	fam_assert(!memmap_allocate(&mm2));
	fam_assert_eq(fam_err, AllocErr);

	ptrs = mmap_allocate(count * sizeof(Ptr));
	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm2);
		fam_assert(ptrs[i] == i + 2);
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm2, ptrs[i]);
	}

	mmap_free(ptrs, count * sizeof(Ptr));
	memmap_cleanup(&mm2);

	fam_err = NoErrors;
	memmap_init(&mm2, size);
	ptrs = mmap_allocate(count * sizeof(Ptr));

	_debug_capacity_exceeded = true;
	fam_err = NoErrors;
	count = 62;
	for (unsigned long long i = 0; i < count; i++) {
		ptrs[i] = memmap_allocate(&mm2);
		fam_assert(ptrs[i]);
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			data[j] = ((i + j) % 26) + 'a';
		}
	}

	fam_assert(!memmap_allocate(&mm2));
	_debug_capacity_exceeded = false;

	for (unsigned long long i = 0; i < count; i++) {
		byte *data = memmap_data(&mm2, ptrs[i]);
		for (int j = 0; j < size; j++) {
			fam_assert_eq(data[j], ((i + j) % 26) + 'a');
		}
		memmap_free(&mm2, ptrs[i]);
	}

	mmap_free(ptrs, count * sizeof(Ptr));
	memmap_cleanup(&mm2);
}

Test(test_slab_allocator) {
	SlabAllocator sa1;
	fam_assert(!slab_allocator_init(&sa1, 16, 100, 200));

	int size = 5;
	Ptr arr[size];
	for (int i = 0; i < size; i++) {
		Ptr p = slab_allocator_allocate(&sa1);
		arr[i] = p;
		byte *parr = slab_get(&sa1, p);
		parr[0] = i + 2;
	}

	for (int i = 0; i < size; i++) {
		byte *parr = slab_get(&sa1, arr[i]);
		fam_assert_eq(parr[0], i + 2);
		slab_allocator_free(&sa1, arr[i]);
	}

	slab_allocator_cleanup(&sa1);
}

Test(test_slab_allocator_adv) {
	SlabAllocator sa1;
	int slab_len = 128;
	fam_assert(!slab_allocator_init(&sa1, slab_len, 100000, 2000000));
	int size = 50000;
	Ptr *arr = mmap_allocate(sizeof(Ptr) * size);

	for (int i = 0; i < size; i++) {
		Ptr p = slab_allocator_allocate(&sa1);
		arr[i] = p;
		byte *parr = slab_get(&sa1, p);
		parr[0] = (i + 2) % 256;
		for (int j = 1; j < slab_len; j++) {
			parr[j] = (((i + 2) + j) % 26) + 'a';
		}
	}

	for (int i = 0; i < size; i++) {
		byte *parr = slab_get(&sa1, arr[i]);
		fam_assert_eq(parr[0], (i + 2) % 256);
		for (int j = 1; j < slab_len; j++)
			fam_assert_eq(parr[j], (((i + 2) + j) % 26) + 'a');
		slab_allocator_free(&sa1, arr[i]);
	}

	slab_allocator_cleanup(&sa1);
	mmap_free(arr, sizeof(Ptr) * size);
}

Test(test_slab_allocator_err_checks) {
	fam_err = NoErrors;
	fam_assert(!slab_get(NULL, 0));
	fam_assert_eq(fam_err, IllegalArgument);

	SlabAllocator sa;
	fam_assert(slab_allocator_init(&sa, 8, 3, 3));

	fam_err = NoErrors;
	fam_assert(!slab_allocator_allocate(NULL));
	fam_assert_eq(fam_err, IllegalArgument);

	fam_assert(!slab_allocator_init(&sa, 8, 15, 5));

	for (int i = 0; i < 5; i++) {
		fam_assert(slab_allocator_allocate(&sa));
	}

	// sa is full
	fam_assert(!slab_allocator_allocate(&sa));

	slab_allocator_cleanup(&sa);

	fam_assert(!slab_allocator_init(&sa, 128, 5, 100));
	Ptr arr[10];

	for (int i = 0; i < 10; i++) {
		fam_assert(arr[i] = slab_allocator_allocate(&sa));
	}

	// The michael-scott queue has to have a single free slab
	fam_assert_eq(slab_allocator_total_slabs(&sa), 11);

	for (int i = 0; i < 10; i++) {
		slab_allocator_free(&sa, arr[i]);
	}

	int fs = slab_allocator_free_size(&sa);
	int ts = slab_allocator_total_slabs(&sa);
	fam_assert_eq(slab_allocator_free_size(&sa), 5);

	slab_allocator_cleanup(&sa);
}

Test(test_slab_allocator_other_situations) {
	Ptr ptr;
	set_debug_memmap_data(0);
	SlabAllocator sa1;
	fam_assert(slab_allocator_init(&sa1, 16, 100, 200));

	set_debug_memmap_data(1);
	fam_assert(slab_allocator_init(&sa1, 16, 100, 200));

	fam_assert(!slab_allocator_init(&sa1, 16, 100, 200));
	ptr = slab_allocator_allocate(&sa1);
	set_debug_memmap_data(0);
	fam_assert(!slab_get(&sa1, ptr));
	set_debug_memmap_data(0);
	fam_assert(!slab_allocator_allocate(&sa1));
	slab_allocator_free(&sa1, ptr);
	// diable panic
	_debug_print_util_disable__ = true;
	slab_allocator_free(&sa1, ptr);
	_debug_print_util_disable__ = false;
	slab_allocator_cleanup(&sa1);

	SlabAllocator sa2;
	fam_assert(!slab_allocator_init(&sa2, 16, 100, 200));
	ptr = slab_allocator_allocate(&sa2);
	slab_allocator_free(&sa2, ptr);

	set_debug_sa1();
	fam_assert(ptr = slab_allocator_allocate(&sa2));
	set_debug_sa2();
	slab_allocator_free(&sa2, ptr);

	slab_allocator_cleanup(&sa2);
}
