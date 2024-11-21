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

Test(bitmap) {
	BitMap b1;
	bitmap_init(&b1, 10);
	for (int64 i = 0; i < PAGE_SIZE * 16; i++) {
		int64 v = bitmap_allocate(&b1);
		if (v != i) println("v=%lli,i=%lli", v, i);
		fam_assert_eq(v, i);
	}
	bitmap_free(&b1, 1);
	bitmap_free(&b1, 77);
	bitmap_free(&b1, PAGE_SIZE * 7);

	int64 v1 = bitmap_allocate(&b1);
	int64 v2 = bitmap_allocate(&b1);
	int64 v3 = bitmap_allocate(&b1);
	int64 v4 = bitmap_allocate(&b1);
	int64 v5 = bitmap_allocate(&b1);

	fam_assert_eq(v1, 1);
	fam_assert_eq(v2, 77);
	fam_assert_eq(v3, PAGE_SIZE * 7);
	fam_assert_eq(v4, PAGE_SIZE * 16);
	fam_assert_eq(v5, PAGE_SIZE * 16 + 1);

	bitmap_cleanup(&b1);
}
