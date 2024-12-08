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
	return $(0);
}

Test(task_table) {
	/*
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
	*/

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

int fun_called = false;
void my_fun(int sig) {
	fun_called = true;
}

Test(alarm) {
	set_timer(my_fun, 140);
	os_sleep(10000);
	assert_eq(fun_called, true);
	unset_timer();
}

bool test_th_recv = false;
int test_return = 45;

void *start_test_th(void *arg) {
	assert_eq(*(int *)arg, 123);
	assert(!os_sleep(100));
	test_th_recv = true;
	return &test_return;
}

Test(thread) {
	Thread *th = thread_init(&THREAD_CONFIG_DEFAULT);
	int x = 123;
	thread_start(th, start_test_th, &x);
	int *check = thread_join(th);
	assert_eq(*check, 45);
	assert(test_th_recv);
	thread_cleanup(th);
}

u32 u64_hash(const void *value) {
	return murmurhash((const byte *)value, sizeof(u64), 0);
}

bool u64_equal(const void *v1, const void *v2) {
	u64 *vv1 = v1;
	u64 *vv2 = v2;
	return *vv1 == *vv2;
}

typedef struct HashEntryImpl {
	struct HashEntry *next;
	u32 key_len;
	u32 value_len;
	u64 key;
	u64 value;
} HashEntryImpl;

Test(hash) {
	HashEntryImpl he1 = {.key_len = sizeof(u64),
						 .value_len = sizeof(u64),
						 .key = 1234,
						 .value = 5678};
	HashEntryImpl he2 = {.key_len = sizeof(u64),
						 .value_len = sizeof(u64),
						 .key = 1235,
						 .value = 1111};

	HashEntryImpl he3 = {.key_len = sizeof(u64),
						 .value_len = sizeof(u64),
						 .key = 1237,
						 .value = 2222};

	Hashtable h1;
	hash_init(&h1, 10, 2 * sizeof(u64));
	hash_put(&h1, (HashEntry *)&he1, u64_hash, u64_equal);
	hash_put(&h1, (HashEntry *)&he2, u64_hash, u64_equal);
	hash_put(&h1, (HashEntry *)&he3, u64_hash, u64_equal);
	u64 value =
		((HashEntryImpl *)hash_get(&h1, &he1.key, u64_hash, u64_equal))->value;
	assert(value);
	assert_eq(value, 5678);
	value =
		((HashEntryImpl *)hash_get(&h1, &he2.key, u64_hash, u64_equal))->value;
	assert(value);
	assert_eq(value, 1111);
	value =
		((HashEntryImpl *)hash_get(&h1, &he3.key, u64_hash, u64_equal))->value;
	assert(value);
	assert_eq(value, 2222);
	u64 missing = 77;
	assert(!hash_get(&h1, &missing, u64_hash, u64_equal));

	value =
		((HashEntryImpl *)hash_rem(&h1, &he3.key, u64_hash, u64_equal))->value;
	assert(value);
	assert_eq(value, 2222);
	assert(!hash_rem(&h1, &he3.key, u64_hash, u64_equal));

	value =
		((HashEntryImpl *)hash_rem(&h1, &he1.key, u64_hash, u64_equal))->value;
	assert(value);
	assert_eq(value, 5678);
	assert(!hash_rem(&h1, &he1.key, u64_hash, u64_equal));

	value =
		((HashEntryImpl *)hash_rem(&h1, &he2.key, u64_hash, u64_equal))->value;
	assert(value);
	assert_eq(value, 1111);
	assert(!hash_rem(&h1, &he2.key, u64_hash, u64_equal));

	hash_cleanup(&h1);
}

int test_signal_state = 0;

void test_handler() {
	AADD(&test_signal_state, 1);
}

void *test_signal(void *arg) {
	assert(!thread_register_handler(test_handler));
	AADD(&test_signal_state, 1);
	os_sleep(10000);
	return NULL;
}

Test(thread_signal) {
	Thread *th = thread_init(&THREAD_CONFIG_DEFAULT);
	thread_start(th, test_signal, NULL);
	while (ALOAD(&test_signal_state) == 0);
	assert(!thread_signal(th));
	AADD(&test_signal_state, 1);
	while (ALOAD(&test_signal_state) != 3);
	thread_join(th);
	thread_cleanup(th);
	assert_eq(ALOAD(&test_signal_state), 3);
}
