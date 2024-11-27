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

Object test_fun1(int y) {
	return $(y + 10);
}

Object test_th1(Channel channel) {
	return 0;
}

Object test_th2(Channel channel) {
	return 0;
}

Object test_th3(Channel channel) {
	return 0;
}

Object test_init(Channel channel) {
	run(test_th1);
	run(test_th2);
	run(test_th3);
	return 0;
}

Test(fam_init) {
	init(test_init, 4);
	assert(1);
}

Test(object) {
	var obj = $(1);
	long long v = *(long long *)value_of(&obj);
	assert_eq(v, 1);
	assert_eq($int(obj), 1);

	$int(obj)++;
	assert_eq($int(obj), 2);

	let obj2 = $(-1LL);
	long long v2 = *(long long *)value_of(&obj2);
	assert_eq(v2, -1LL);
	assert_eq($int(obj2), -1);

	double d = 1.4;
	let obj3 = object_float(d);
	double v3 = *(double *)value_of(&obj3);
	assert_eq(v3, 1.4);
	assert_eq($float(obj3), 1.4);

	float f = 1.5;
	let obj4 = object_float(f);
	double v4 = *(double *)value_of(&obj4);
	assert_eq(v4, 1.5);
	assert_eq($float(obj4), 1.5);

	let obj5 = $(1.6);
	double v5 = *(double *)value_of(&obj5);
	assert_eq(v5, 1.6);
	assert_eq($float(obj5), 1.6);

	let obj6 = $(1234ULL);
	unsigned long long v6 = *(unsigned long long *)value_of(&obj6);
	assert_eq(v6, 1234);
	assert_eq($uint(obj6), 1234);

	let obj7 = $(test_fun1);
	const Object (*v7_fn)(int) = $fn(&obj7);
	Object v7 = v7_fn(3);
	assert_eq($int(v7), 13);
}

Test(test_lock) {
	Lock l1 = INIT_LOCK;
	lock_read(&l1);
	// upper bits incremented by 1.
	assert_eq(l1 >> 32, 1);
	// lower bits also incremeneted by 1.
	assert_eq(l1 & 0xFFFFFFFF, 1);

	lock_unlock(&l1);
	// upper bits incremented by 1.
	assert_eq(l1 >> 32, 2);
	// lower bits also decremented by 1.
	assert_eq(l1 & 0xFFFFFFFF, 0);

	lock_write(&l1);
	// upper bits incremented by 2.
	assert_eq(l1 >> 32, 4);
	// lower bits high bit set
	assert_eq(l1 & 0xFFFFFFFF, 0x80000000);

	lock_unlock(&l1);

	// upper bits incremented by 1.
	assert_eq(l1 >> 32, 5);
	// lower bits are set to 0 now
	assert_eq(l1 & 0xFFFFFFFF, 0x00000000);
}
