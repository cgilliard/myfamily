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

int thread_test_complete = 0;

Object test_run(Object *arg) {
	// confirm passed in param
	assert_eq($int(*arg), 3);
	// wait to ensure join works
	os_sleep(300);
	// update global var to indicate completion
	ASTORE(&thread_test_complete, 1);
	// return 99 to test return
	return $(99);
}

Test(thread) {
	// create a thread with 16384 byte stack
	var th1 = $thread(16384);
	// set the 'run' function for the thread
	let res1 = set(th1, "run", test_run);
	// check for error
	assert(!$is_err(res1));
	// create an argument
	let arg = $(3);
	// confirm that the complete variable is 0
	assert_eq(ALOAD(&thread_test_complete), 0);
	// start the thread with the specified argument
	let res2 = $start(th1, arg);
	// check for error
	assert(!$is_err(res2));

	// join the thread
	let res3 = $join(th1);
	// check for error
	assert(!$is_err(res3));
	// assert that the value returned is as expected
	assert_eq($int(res3), 99);
	// confirm the complete variable was set
	assert_eq(ALOAD(&thread_test_complete), 1);

	// create thread
	var th2 = $thread(16384);
	// try to start thread without specifying 'run' function.
	let res4 = $start(th2, arg);
	// error occurs
	assert($is_err(res4));
	// kind NotFound
	assert_eq($kind(res4), NotFound);
}

int test_handler_count = 0;

Object test_run2(Object *arg) {
	while (ALOAD(&test_handler_count) != 2) sched_yield();
	return $(0);
}

void test_handler() {
	AADD(&test_handler_count, 1);
}

Test(thread_signal) {
	var th1 = $thread(16384);
	let res1 = set(th1, "run", test_run2);
	let res2 = set(th1, "handler", test_handler);
	let arg = $(0);
	$start(th1, arg);
	assert_eq(ALOAD(&test_handler_count), 0);
	let res3 = $signal(th1);
	while (ALOAD(&test_handler_count) != 1) sched_yield();
	let res4 = $signal(th1);
	let res5 = $join(th1);
	assert_eq(ALOAD(&test_handler_count), 2);
}
