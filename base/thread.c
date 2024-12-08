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

#include <base/sys.h>
#include <base/thread.h>
#include <pthread.h>
#include <signal.h>

int gettid();

typedef struct ThreadImpl {
	pthread_t th;
	ThreadSignalHandler usr1_handler;
	ThreadSignalHandler usr2_handler;
	u64 guard_size;
	u64 stack_size;
	void *stack;
} ThreadImpl;

Thread *thread_init(ThreadConfig *config) {
	if (config->stack_size % PAGE_SIZE || config->stack_size == 0) return NULL;
	ThreadImpl *ret = map(1);
	if (ret == NULL) return NULL;
	ret->guard_size = config->guard_size < 0 ? PAGE_SIZE : config->guard_size;
	ret->stack_size = config->stack_size;
	ret->stack = config->stack_address ? config->stack_address
									   : map(ret->stack_size / PAGE_SIZE);
	return (Thread *)ret;
}
void thread_cleanup(Thread *th) {
	if (th == NULL) return;
	ThreadImpl *impl = (ThreadImpl *)th;
	if (impl->stack) unmap(impl->stack, impl->stack_size / PAGE_SIZE);
	unmap(impl, 1);
}
int thread_start(Thread *th, ThreadFunction func, void *arg) {
	ThreadImpl *impl = (ThreadImpl *)th;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setguardsize(&attr, impl->guard_size);
	pthread_attr_setstack(&attr, impl->stack, impl->stack_size);
	pthread_create(&impl->th, &attr, func, arg);
	pthread_attr_destroy(&attr);
	return 0;
}

void thread_handler(int sig) {
	struct sigaction sa;
	sa.sa_handler = thread_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void thread_register_handler(ThreadSignalHandler handler) {
	struct sigaction sa;
	sa.sa_handler = thread_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);
}

int thread_signal(Thread *th) {
	return 0;
}

void *thread_join(Thread *th) {
	void *ret;
	ThreadImpl *impl = (ThreadImpl *)th;
	pthread_join(impl->th, &ret);
	return ret;
}

u64 thread_id() {
	u64 thread_id = 0;
#ifdef __APPLE__
	pthread_threadid_np(NULL, &thread_id);
#elif defined(__linux__)
	thread_id = gettid();
#else
	println("WARN: unsupported platform! Cannot determine tid.");
#endif
	return thread_id;
}
