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

Suite(Base);

void test1() {
	// assert_eq(1, 10);
}

Test(last_trace) {
	// const char *lt = last_trace();
	// assert_eq(1, 0);
	test1();
}

Test(proc_table) {
	ProcTable table1 = INIT_PROC_TABLE;

	Process proc1 = {};
	let r1 = proctable_add_process(&table1, &proc1);
	assert(!$is_err(r1));

	proctable_cleanup(&table1);

	/*
		let r2 = Err(AllocErr);
		println("err={}", r2);
	*/
	/*
		char *bt = backtrace_full();
		print("{}", bt);
		unmap(bt, 3);
	*/
}

Test(object) {
	let obj1 = $(1);
	let obj2 = box(1000);
	let obj3 = Err(AllocErr);
}

Object test_init_tasks(Channel ch) {
	return $(0);
}

Test(proc) {
	let res = init(test_init_tasks, 4);
	assert(!$is_err(res));
	halt(0);
}
