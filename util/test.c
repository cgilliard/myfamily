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
#include <util/panic.h>
#include <util/slabs.h>

MySuite(util);

typedef struct TestData {
	u64 v1;
	u32 v2;
	i32 v3;
	char buf[10];
} TestData;

MyTest(util, test_fat_ptr) {
	FatPtr ptr1;
	// create a 64 bit test obj (the appropriate bit is set for this ID to be valid as 64 bit)
	fat_ptr_test_obj64(&ptr1, 1, sizeof(TestData));
	// do assertions and use allocated data
	cr_assert_eq(fat_ptr_len(&ptr1), sizeof(TestData));
	cr_assert_eq(fat_ptr_id(&ptr1), 1);
	TestData *td = fat_ptr_data(&ptr1);
	td->v1 = 1234;
	td->v2 = 5678;
	td->v3 = 9999;
	strcpy(td->buf, "hi");
	cr_assert_eq(td->v1, 1234);
	cr_assert_eq(td->v2, 5678);
	cr_assert_eq(td->v3, 9999);
	cr_assert(!strcmp(td->buf, "hi"));
	fat_ptr_free_test_obj64(&ptr1);

	// try a 32 bit
	FatPtr ptr2;
	// create a 32 bit test obj (the appropriate bit is NOT set for this ID to be valid as 32 bit,
	// it is less than 2^31)
	fat_ptr_test_obj32(&ptr2, 2, sizeof(TestData));
	// do assertions and use allocated data
	cr_assert_eq(fat_ptr_len(&ptr2), sizeof(TestData));
	cr_assert_eq(fat_ptr_id(&ptr2), 2);
	TestData *td2 = fat_ptr_data(&ptr2);
	td2->v1 = 1234;
	td2->v2 = 5678;
	td2->v3 = 9999;
	strcpy(td2->buf, "hi");
	cr_assert_eq(td2->v1, 1234);
	cr_assert_eq(td2->v2, 5678);
	cr_assert_eq(td2->v3, 9999);
	cr_assert(!strcmp(td2->buf, "hi"));
	fat_ptr_free_test_obj32(&ptr2);
}

__attribute__((noreturn)) void on_panic(const char *msg) {
	printf("on_panic = %s\n", msg);
	cr_assert(!strcmp(msg, "test7"));
	exit(0);
}

MyTest(util, test_panic) {
	set_on_panic(on_panic);
	panic("test7");
	//   ensure we never get here
	cr_assert(false);
}
