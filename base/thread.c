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

/*
#include <base/hash.h>
#include <base/limits.h>
#include <base/lock.h>
#include <base/murmurhash.h>
#include <base/print_util.h>
#include <base/slabs.h>
#include <base/sys.h>
#include <base/thread.h>
#include <pthread.h>
#include <signal.h>

#define THREAD_GLOBAL_HASH_ARR_SIZE 128
#define MURMURHASHSEED 1337

typedef struct ThreadEntry {
	struct HashEntry *next;
	u32 key_len;
	u32 value_len;
	u64 tid;
	ThreadSignalHandler handler;
} ThreadEntry;

Hashtable thread_global_hash;
Lock thread_hash_lock = INIT_LOCK;
SlabAllocator thread_slab_allocator;

void __attribute__((constructor)) __init_thread_hash() {
	hash_init(&thread_global_hash, THREAD_GLOBAL_HASH_ARR_SIZE,
			  sizeof(ThreadEntry));
	let res = slab_allocator_init(&thread_slab_allocator, 128 - SLAB_LIST_SIZE,
								  UINT32_MAX, 128);
	if ($is_err(res))
		panic("Could not initialize thread slab allocator: {}", res);
}

typedef struct ThreadImpl {
	pthread_t th;
	u64 stack_size;
	void *stack;
	bool joined;
} ThreadImpl;

u32 thread_hash(const void *key) {
	return murmurhash((const byte *)key, sizeof(u64), MURMURHASHSEED);
}

bool thread_eq(const void *v1, const void *v2) {
	u64 *vv1 = v1;
	u64 *vv2 = v2;
	return *vv1 == *vv2;
}

Thread *thread_init(ThreadConfig *config) {
	if (config->stack_size % PAGE_SIZE || config->stack_size == 0) return NULL;
	ThreadImpl *ret = map(2 + config->stack_size / PAGE_SIZE);
	if (ret == NULL) return NULL;
	ret->stack_size = config->stack_size;
	ret->stack = ((byte *)ret) + PAGE_SIZE;
	return (Thread *)ret;
}

void thread_cleanup(Thread *th) {
	if (th == NULL) return;
	ThreadImpl *impl = (ThreadImpl *)th;
	unmap(impl, 2 + impl->stack_size / PAGE_SIZE);
	u64 tid = thread_id();
	HashEntry *he = hash_rem(&thread_global_hash, &tid, thread_hash, thread_eq);
	if (he) slab_allocator_free(&thread_slab_allocator, he);
}

int thread_start(Thread *th, ThreadFunction func, void *arg) {
	ThreadImpl *impl = (ThreadImpl *)th;
	pthread_attr_t attr;
	if (pthread_attr_init(&attr)) return -1;
	if (pthread_attr_setstack(&attr, impl->stack, impl->stack_size)) {
		pthread_attr_destroy(&attr);
		return -1;
	}

	int ret = pthread_create(&impl->th, &attr, func, arg);
	pthread_attr_destroy(&attr);
	return ret;
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
	u64 tid = thread_id();
	HashEntry *he = hash_get(&thread_global_hash, &tid, thread_hash, thread_eq);
	ThreadEntry *ent = (ThreadEntry *)he;
	if (ent) ent->handler();
}

int thread_register_handler(ThreadSignalHandler handler) {
	ThreadEntry *ent = slab_allocator_allocate(&thread_slab_allocator);
	if (ent == NULL) return -1;
	ent->handler = handler;
	ent->tid = thread_id();
	ent->key_len = sizeof(u64);
	ent->value_len = sizeof(ThreadSignalHandler);

	hash_put(&thread_global_hash, (HashEntry *)ent, thread_hash, thread_eq);

	struct sigaction sa;
	sa.sa_handler = thread_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);
	return 0;
}

int thread_signal(Thread *th) {
	ThreadImpl *impl = (ThreadImpl *)th;
	return pthread_kill(impl->th, SIGUSR1);
}

void *thread_join(Thread *th) {
	void *ret;
	ThreadImpl *impl = (ThreadImpl *)th;
	pthread_join(impl->th, &ret);
	impl->joined = true;
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
*/

#include <base/atomic.h>
#include <base/object.h>
#include <base/object_impl.h>
#include <base/print_util.h>
#include <base/sys.h>
#include <pthread.h>
#include <signal.h>

typedef enum ThreadState {
	ThreadRunnable,
	ThreadRunning,
	ThreadZombie,
	ThreadJoined,
} ThreadState;

__attribute__((aligned(32))) typedef struct ThreadImpl {
	pthread_t th;
	u64 stack_size;
	ObjectNc arg;
	ThreadFn start_fn;
	void (*handler)();
	int state;
	int ref_count;
	ObjectNc ref;
} ThreadImpl;

Object object_thread(u64 stack_size) {
	u64 pages = 1 + ((stack_size + sizeof(ThreadImpl)) - 1) / PAGE_SIZE;
	ThreadImpl *ti = map(pages);
	if (ti == NULL) return Err(AllocErr);
	ti->stack_size = stack_size;
	ASTORE(&ti->ref_count, 1);
	ObjectImpl val = {.type = Thread, .value.ptr_value = ti};
	return *(Object *)&val;
}

_Thread_local void (*tl_handler)();

void object_nop() {
}

void object_handler() {
	tl_handler();
	struct sigaction sa;
	sa.sa_handler = object_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGUSR1, &sa, NULL);

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_UNBLOCK, &set, NULL);
}

void *object_thread_start_fn(void *arg) {
	ThreadImpl *ti = arg;
	ThreadFn fn = ti->start_fn;
	tl_handler = ti->handler;
	if (!tl_handler) tl_handler = object_nop;

	struct sigaction sa;
	sa.sa_handler = object_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);

	Object varg = object_move(&ti->arg);
	ASTORE(&ti->state, ThreadRunning);
	ti->arg = fn(&varg);

	// disable signals once we finish
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	ASTORE(&ti->state, ThreadZombie);
	object_cleanup(&ti->ref);
	return NULL;
}

Object object_thread_start(Object *obj, Object *arg) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	ThreadImpl *ti = impl->value.ptr_value;

	let fn = get(*obj, "run");
	if ($is_err(fn)) return Err(NotFound);
	ThreadFn start_fn = $fn(fn);
	if (start_fn == NULL) return Err(IllegalState);
	let handler = get(*obj, "handler");
	void (*handler_fn)() = NULL;
	if (!$is_err(handler)) handler_fn = $fn(handler);
	if (handler_fn) ti->handler = handler_fn;

	ti->start_fn = start_fn;
	ti->arg = object_move(arg);
	ti->state = ThreadRunnable;
	ti->ref = object_ref(obj);

	pthread_attr_t attr;
	if (pthread_attr_init(&attr)) return Err(AllocErr);
	if (pthread_attr_setstacksize(&attr, ti->stack_size)) {
		pthread_attr_destroy(&attr);
		return Err(IllegalArgument);
	}

	int ret = pthread_create(&ti->th, &attr, object_thread_start_fn, ti);
	pthread_attr_destroy(&attr);
	while (ALOAD(&ti->state) == ThreadRunnable) sched_yield();
	return $(0);
}

Object object_thread_signal(const Object *obj) {
	if (obj == NULL) return Err(IllegalArgument);

	ObjectImpl *impl = (ObjectImpl *)obj;
	ThreadImpl *ti = impl->value.ptr_value;

	int code;
	if ((code = pthread_kill(ti->th, SIGUSR1))) println("code={}", code);
	return $(0);
}
Object object_thread_join(Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	ThreadImpl *ti = impl->value.ptr_value;
	pthread_join(ti->th, NULL);
	ASTORE(&ti->state, ThreadJoined);
	return ti->arg;
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

Object object_thread_ref(Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	ThreadImpl *ti = impl->value.ptr_value;
	AADD(&ti->ref_count, 1);
	return *obj;
}

void object_thread_cleanup(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	ThreadImpl *ti = impl->value.ptr_value;
	int ref_count = ASUB(&ti->ref_count, 1);
	if (ref_count == 1) {
		if (ALOAD(&ti->state) == ThreadZombie) pthread_detach(ti->th);
		u64 pages = 1 + ((ti->stack_size + sizeof(ThreadImpl)) - 1) / PAGE_SIZE;
		unmap(impl->value.ptr_value, pages);
	}
}
