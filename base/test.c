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

#include <base/base.h>
#include <base/test.h>

MySuite(base);

MyTest(base, test_mymalloc) {
	u64 begin_mymalloc_sum = mymalloc_sum();
	u64 begin_myfree_sum = myfree_sum();
	u64 begin_diff = begin_mymalloc_sum - begin_myfree_sum;
	u64 diff = mymalloc_sum - myfree_sum;
	void *tmp = mymalloc(100);
	u64 end_mymalloc_sum = mymalloc_sum();
	u64 end_myfree_sum = myfree_sum();
	u64 end_diff = end_mymalloc_sum - end_myfree_sum;
	cr_assert_neq(end_diff, begin_diff);
	myfree(tmp);
	end_mymalloc_sum = mymalloc_sum();
	end_myfree_sum = myfree_sum();
	end_diff = end_mymalloc_sum - end_myfree_sum;
	cr_assert_eq(end_diff, begin_diff);
}
