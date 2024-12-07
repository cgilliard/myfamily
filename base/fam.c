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
#include <base/lock.h>
#include <base/print_util.h>
#include <base/sys.h>
#include <base/task.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <ucontext.h>
#include <unistd.h>

#ifndef SA_RESTART
#define SA_RESTART 0x10000000  // The value that is often used for SA_RESTART
#endif

int pthread_kill(pthread_t thread, int sig);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);

Lock fam_lock = INIT_LOCK;
TaskTable task_tab;
pthread_t *task_threads;
u64 task_threads_count;

void system_idle_task(void *channel) {
	loop {
		sleep(1);
	}
}

#define SYSTEM_IDLE_STACK_SIZE 1024
byte system_idle_stack_base[SYSTEM_IDLE_STACK_SIZE];

Task system_idle = {.task = system_idle_task,
					.stack_base = system_idle_stack_base,
					.stack_size = SYSTEM_IDLE_STACK_SIZE,
					.state = Initializable,
					.id = {9}};

void fam_handle_alarm(int sig) {
	lockw(&fam_lock);
	println("got alarm: {}", sig);
	unlock(&fam_lock);
	for (u64 i = 0; i < task_threads_count; i++) {
		pthread_t worker_thread = task_threads[i];
		pthread_kill(worker_thread, SIGUSR1);
	}
}

void fam_signal_handler(int sig);

int perror(const char *);

void fam_test() {
	println("test");
}

void fam_sched() {
	ucontext_t scheduler_context;  // Declare a scheduler context

	// In your scheduler setup, save the scheduler context
	/*
		if (getcontext(&scheduler_context) == -1) {
			panic("getcontext failed for scheduler_context");
		}
	*/

	struct sigaction sa;
	sa.sa_handler = fam_signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);

	Task *next = tasktable_pick_random_runnable(&task_tab);
	lockw(&fam_lock);
	if (next->state == Initializable) {
		next->state = Runnable;
		// println("initializable");
		unlock(&fam_lock);

		/*
				if (next->stack_base == NULL) panic("allocation");
				ucontext_t current_context;
				next->uctx.uc_stack.ss_sp = map(4);
				next->uctx.uc_stack.ss_size = 1024 * 64;
				next->uctx.uc_stack.ss_flags = 0;
				next->uctx.uc_link = &current_context;

				makecontext(&next->uctx, fam_test, 0);
				if (getcontext(&current_context) == -1) panic("getcontext");
				errno = 0;
				if (swapcontext(&current_context, &next->uctx) == -1) {
					perror("error");
					panic("swapcontext failed 1");
				}
		*/
	} else {
		/*
				println("runnable");
				unlock(&fam_lock);
				// next->task(&next->id);
				ucontext_t current_context;
				getcontext(&current_context);  // Save the current scheduler
		   context if (swapcontext(&current_context, &next->uctx) == -1) {
					panic("swapcontext failed 2");
				}
		*/
	}
}

void fam_signal_handler(int sig) {
	lockw(&fam_lock);
	println("got sig: {}", sig);
	unlock(&fam_lock);
	fam_sched();
}

void *fam_start_thread() {
	struct sigaction sa;
	sa.sa_handler = fam_signal_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);

	fam_sched();

	return NULL;
}

Object send(Channel channel, Object object) {
	let ret = $(0);
	return ret;
}

Channel run(Object (*task)(Channel *channel)) {
	Channel channel;
	return channel;
}

Object recv(Channel channel, int timeout_millis) {
	let ret = $(0);
	return ret;
}

Object init(Object (*task)(Channel *channel), int threads) {
	ret res1 = tasktable_init(&task_tab);
	if ($is_err(res1)) return res1;

	ret res2 = tasktable_add_task(&task_tab, &system_idle);
	if ($is_err(res2)) return res2;

	Task *next = tasktable_pick_random_runnable(&task_tab);
	next = tasktable_pick_random_runnable(&task_tab);

	u64 pages = 1 + (threads * sizeof(pthread_t)) / PAGE_SIZE;
	task_threads = map(pages);
	task_threads_count = threads;

	struct sigaction sa;
	struct itimerval timer;

	sa.sa_handler = fam_handle_alarm;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGALRM, &sa, NULL) == -1) {
		panic("sigaction");
	}

	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 100000;  // 100ms
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 100000;	 // 100ms

	if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
		panic("setitimer");
	}

	for (u64 i = 0; i < threads; i++) {
		pthread_create(&task_threads[i], NULL, fam_start_thread, NULL);
	}

	ret r = $(0);
	return r;
}

void halt(int code) {
	for (u64 i = 0; i < task_threads_count; i++) {
		pthread_join(task_threads[i], NULL);
	}

	u64 pages = 1 + (task_threads_count * sizeof(pthread_t)) / PAGE_SIZE;
	unmap(task_threads, pages);

	tasktable_cleanup(&task_tab);

#ifndef TEST
	_exit(code);
#endif	// TEST
}
