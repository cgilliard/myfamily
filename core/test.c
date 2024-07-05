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

#include <core/resources.h>
#include <core/test.h>

MySuite(core);

MyTest(core, test_malloc) {
	u64 init_diff = mymalloc_sum() - myfree_sum();
	u64 init_realloc = myrealloc_sum();
	void *t1 = mymalloc(100);
	u64 incr_diff = mymalloc_sum() - myfree_sum();

	void *tmp = myrealloc(t1, 200);
	// shouldn't be problems allocating 200 bytes
	cr_assert(tmp != NULL);
	t1 = tmp;

	u64 final_realloc = myrealloc_sum();

	myfree(t1);
	u64 final_diff = mymalloc_sum() - myfree_sum();

	// since we free everything diffs should be equal
	cr_assert_eq(final_diff, init_diff);

	// and since it's thread local only we can do stuff so it's just a
	// single allocation greater
	cr_assert_eq(init_diff + 1, incr_diff);

	cr_assert_eq(final_realloc, init_realloc + 1);
}

MyTest(core, test_fopen) {
	u64 fopen_init_diff = myfopen_sum() - myfclose_sum();
	u64 fopen_init = myfopen_sum();
	u64 fclose_init = myfclose_sum();

	FILE *fp = myfopen("./resources/test.txt", "r");
	myfclose(fp);

	u64 fopen_final_diff = myfopen_sum() - myfclose_sum();

	cr_assert_eq(fopen_final_diff, fopen_init_diff);
	cr_assert_eq(fopen_init, myfopen_sum() - 1);
	cr_assert_eq(fclose_init, myfclose_sum() - 1);
}
