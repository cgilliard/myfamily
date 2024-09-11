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

#include <assert.h>
#include <base/chain_allocator.h>
#include <base/panic.h>
#include <core/thread.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

int PANIC_TRUE = 1;
int PANIC_FALSE = 0;

typedef struct ThreadArgsWrapper {
	void (*start_routine)(void *);
	void *args;
	FatPtr self;
} ThreadArgsWrapper;

typedef struct ThreadImpl {
	pthread_t pth;
	u64 tid;
	FatPtr self;
} ThreadImpl;

void Thread_cleanup(Thread *ptr) {
	if (chain_free(&ptr->impl))
		panic("Could not free Thread");
}

void *Thread_proc_start(void *arg) {
	// set jmp return point for panics
	if (PANIC_RETURN()) {
		pthread_exit(&PANIC_TRUE);
	}

	ThreadArgsWrapper *taw = arg;
	(taw->start_routine)(taw->args);

	ChainGuard guard = GLOBAL_SYNC_ALLOCATOR();
	if (chain_free(&taw->self))
		panic("Could not free ThreadArgsWrapper");

	if (__default_tl_heap_allocator != NULL) {
		heap_allocator_cleanup(__default_tl_heap_allocator);
	}

	pthread_exit(&PANIC_FALSE);
}

int Thread_start(Thread *th, void (*start_routine)(void *), void *args) {
	int ret = 0;
	ret = chain_malloc(&th->impl, sizeof(ThreadImpl));

	if (!ret) {

		ChainGuard guard = GLOBAL_SYNC_ALLOCATOR();
		ThreadImpl *ti = th->impl.data;

		FatPtr tawptr;
		ret = chain_malloc(&tawptr, sizeof(ThreadArgsWrapper));

		if (!ret) {
			ThreadArgsWrapper *data = tawptr.data;
			data->start_routine = start_routine;
			data->args = args;
			data->self = tawptr;

			ret = pthread_create(&ti->pth, NULL, Thread_proc_start, data);
		}
	}
	return ret;
}

JoinResult Thread_join(Thread *ptr) {
	int result = -1;
	int *status = &PANIC_FALSE;

	if (ptr == NULL)
		errno = EINVAL;
	else {
		ThreadImpl *ti = ptr->impl.data;
		result = pthread_join(ti->pth, (void *)&status);
	}
	JoinResult jr = {*status == PANIC_TRUE, result != 0};
	return jr;
}

bool JoinResult_is_panic(JoinResult *jr) {
	return jr->is_panic;
}

bool JoinResult_is_error(JoinResult *jr) {
	return jr->is_error;
}
