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

#define _XOPEN_SOURCE
#include <base/fam.h>
#include <base/print_util.h>
#include <base/proc.h>
#include <base/sys.h>
#include <pthread.h>
#include <ucontext.h>

ProcTable proc_tab;

Object system_idle_task(Channel channel) {
	loop sched_yield();
}
#define SYSTEM_IDLE_STACK_SIZE 1024
byte system_idle_stack_base[SYSTEM_IDLE_STACK_SIZE];
Process system_idle = {.task = system_idle_task,
					   .stack_base = system_idle_stack_base,
					   .stack_size = SYSTEM_IDLE_STACK_SIZE,
					   .state = Runnable};

void *fam_start_thread() {
	return 0;
}

Object send(Channel channel, Object object) {
	let ret = $(0);
	return ret;
}

Channel run(Object (*task)(Channel channel)) {
	Channel channel;
	return channel;
}

Object recv(Channel channel, int timeout_millis) {
	let ret = $(0);
	return ret;
}

Object init(Object (*task)(Channel channel), int threads) {
	ObjectNc res1 = proctable_init(&proc_tab);
	if ($is_err(res1)) return res1;

	ObjectNc res2 = proctable_add_process(&proc_tab, &system_idle);
	if ($is_err(res2)) return res2;

	for (int i = 0; i < threads; i++) {
		pthread_t th;
		pthread_create(&th, 0, fam_start_thread, 0);
	}
	Object ret = $(0);
	return ret;
}

void halt(int code) {
	proctable_cleanup(&proc_tab);
#ifndef TEST
	_exit(code);
#endif	// TEST
}
