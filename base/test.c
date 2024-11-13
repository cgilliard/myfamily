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

Test(test_memmap_recycle) {
	MemMap mm1;
	int size = 64;
	int count = 30000;
	int itt = 4;
	memmap_init(&mm1, size);
	Ptr ptrs[itt];

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < itt; j++) {
			ptrs[j] = memmap_allocate(&mm1);
			byte *data = memmap_data(&mm1, ptrs[j]);
			data[0] = 'a' + (j % 26);
			fam_assert((ptrs[j] == 2 + j));
		}

		for (int j = 0; j < itt; j++) {
			byte *data = memmap_data(&mm1, ptrs[j]);
			fam_assert_eq(data[0], 'a' + (j % 26));
			fam_assert(ptrs[j] == 2 + j);
			memmap_free(&mm1, ptrs[j]);
		}
	}
	memmap_cleanup(&mm1);
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
	memmap_cleanup(&mm1);
}

Test(test_big_memmap) {
	memmap_reset();
	MemMap mm1;
	// int size = 8;
	// unsigned long long count = 1024ULL * 1024ULL * 1024ULL * 2ULL;
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
