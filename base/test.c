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

MyTest(base, test_lock) {
	Lock l1 = lock_create();
	lock_read(&l1);
	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 1);
	// lower bits also incremeneted by 1.
	cr_assert_eq(l1 & 0xFFFFFFFF, 1);

	lock_unlock(&l1);
	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 2);
	// lower bits also decremented by 1.
	cr_assert_eq(l1 & 0xFFFFFFFF, 0);

	lock_write(&l1);
	// upper bits incremented by 2.
	cr_assert_eq(l1 >> 32, 4);
	// lower bits high bit set
	cr_assert_eq(l1 & 0xFFFFFFFF, 0x80000000);

	lock_unlock(&l1);

	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 5);
	// lower bits are set to 0 now
	cr_assert_eq(l1 & 0xFFFFFFFF, 0x00000000);
}

MyTest(base, test_lock_macros) {
	int x = 0;
	Lock l = lock();
	wsync(l, { x++; });
	cr_assert(x == 1);
	int y;
	rsync(l, { y = x; });
	cr_assert(y == 1);
}
