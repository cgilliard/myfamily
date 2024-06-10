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

#include <base/backtrace.h>
#include <base/tlmalloc.h>
#include <base/types.h>
#include <criterion/criterion.h>
#include <stdio.h>

Test(base, test_tlmalloc) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_realloc_count = realloc_count();
	u64 initial_free_count = free_count();
	u128 initial_bytes_alloc = cur_bytes_alloc();

	void *ptr1 = tlmalloc(10);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 10);
	cr_assert_eq(alloc_count(), initial_alloc_count + 1);
	cr_assert_eq(realloc_count(), initial_realloc_count);
	cr_assert_eq(free_count(), initial_free_count);

	void *ptr2 = tlmalloc(20);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 30);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count);
	cr_assert_eq(free_count(), initial_free_count);

	ptr2 = tlrealloc(ptr2, 30);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 60);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count + 1);
	cr_assert_eq(free_count(), initial_free_count);

	tlfree(ptr1);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 60);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count + 1);
	cr_assert_eq(free_count(), initial_free_count + 1);

	tlfree(ptr2);

	cr_assert_eq(cur_bytes_alloc(), initial_bytes_alloc + 60);
	cr_assert_eq(alloc_count(), initial_alloc_count + 2);
	cr_assert_eq(realloc_count(), initial_realloc_count + 1);
	cr_assert_eq(free_count(), initial_free_count + 2);
}

typedef struct CleanupTestPtr {
	char *s;
	int x;
} CleanupTestPtr;

void cleanup_test_free(CleanupTestPtr *ptr) {
	if (ptr->s) {
		tlfree(ptr->s);
		ptr->s = NULL;
	}
}

#define CleanupTest CleanupTestPtr CLEANUP(cleanup_test_free)

CleanupTest build_cleanup_test(char *s, int x) {

	CleanupTestPtr ret;
	ret.x = x;
	ret.s = tlmalloc((strlen(s) + 1) * sizeof(char));
	strcpy(ret.s, s);

	return ret;
}

Test(base, test_cleanup) {
	CleanupTestPtr *tmp;

	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;

	{
		CleanupTest ct = build_cleanup_test("this is a test", 5);
		cr_assert_eq(ct.x, 5);
		tmp = &ct;
		cr_assert(!strcmp("this is a test", ct.s));
		// tmp->s is not null
		cr_assert_neq(tmp->s, NULL);
	}

	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	cr_assert_eq(final_diff, initial_diff);
	// cleanup is called and tmp->s is now null
	cr_assert_eq(tmp->s, NULL);

	// it's ok to call cleanup twice
	cleanup_test_free(tmp);
}

Test(base, test_error) {
	u64 initial_alloc_count = alloc_count();
	u64 initial_free_count = free_count();

	u64 initial_diff = initial_alloc_count - initial_free_count;
	{
		Backtrace b = backtrace_generate(100);
		backtrace_print(&b, 0);
		cr_assert(b.count > 0);
		cr_assert(b.count <= 100);
	}
	u64 final_alloc_count = alloc_count();
	u64 final_free_count = free_count();
	u64 final_diff = final_alloc_count - final_free_count;

	cr_assert_eq(final_diff, initial_diff);
}
