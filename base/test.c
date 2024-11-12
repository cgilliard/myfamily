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

Test(test_util) {
	fam_assert(1);
	// fam_assert(0);
}

Test(test_colors) {
}

Test(test_2) {
	// fam_assert(0);
}

Test(test3) {
	fam_assert(1);
}

Test(test4) {
	int x = 0;
	int y = 0;
	int z = 0;
	int a = 2;
	int v = a + x;
	/*
		fam_assert(0);
		fam_assert(0);
		fam_assert(0);
		fam_assert(0);
		fam_assert(0);
		fam_assert(0);
		fam_assert(0);
		fam_assert(0);
		fam_assert(0);
	*/
}
Test(test5) {
}

Test(test6) {
}

Test(test7) {
	int x = 0;
	int y = 0;
	int a = 2;
	int v = a + x;
	fam_assert(1);

	int v2 = 1;
	int v3 = 3;
}

Test(x) {
	int x = 1;
	fam_assert_eq(x, 1);
}
