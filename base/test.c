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

Object test_init_task_table(Channel *ch) {
	println("init tt");
	return $(0);
}

Test(task_table) {
	TaskTable table1 = INIT_TASK_TABLE;
	Channel ch1 = {0}, ch2 = {1}, ch3 = {2};

	Task task1 = {.id = ch1}, task2 = {.id = ch2}, task3 = {.id = ch3};
	let r1 = tasktable_add_task(&table1, &task1);
	assert(!$is_err(r1));
	let r2 = tasktable_add_task(&table1, &task2);

	Task *t2_out = tasktable_remove_task(&table1, &task2.id);
	assert(channel_equal(&t2_out->id, &task2.id));

	let r3 = tasktable_add_task(&table1, &task3);

	tasktable_cleanup(&table1);

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

Object test_init_tasks(Channel *ch) {
	return $(0);
}

Test(task) {
	let res = init(test_init_tasks, 1);
	assert(!$is_err(res));
	halt(0);
}
