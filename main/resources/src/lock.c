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

#include <core/lock.h>
#include <core/panic.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_LOCKS 100
_Thread_local Lock *__active_locks_[MAX_LOCKS];
_Thread_local u64 __active_lock_count_ = 0;

void insert_active_lock(Lock *ptr)
{
	if (__active_lock_count_ >= MAX_LOCKS)
		panic("too many locks!");
	__active_locks_[__active_lock_count_] = ptr;
	__active_lock_count_++;
}
void delete_active_lock(Lock *ptr)
{
	// note: locks are removed in reverse order
	// It's possible that cleanup occurs in slightly different order
	// but the effect is the same each lock would be invalidated at the end
	// of a scope. For simplicity we just decrement here.
	if (__active_lock_count_ == 0)
		panic("active lock stack underflow. Unexpected error!");
	__active_lock_count_--;
}

Lock Lock_build()
{
	LockPtr ret;
	pthread_mutex_init(&ret.lock, NULL);
	pthread_cond_init(&ret.cond, NULL);
	atomic_init(&ret.poison, false);
	atomic_init(&ret.is_locked, false);
	atomic_init(&ret.tid, 0);
	return ret;
}

void Lock_cleanup(LockPtr *ptr)
{
	pthread_mutex_destroy(&ptr->lock);
	pthread_cond_destroy(&ptr->cond);
}

void Lock_set_poison(Lock *ptr)
{
	atomic_exchange(&ptr->poison, true);
}
bool Lock_is_poisoned(Lock *ptr)
{
	return atomic_load(&ptr->poison);
}
void Lock_clear_poison(Lock *ptr)
{
	atomic_exchange(&ptr->poison, false);
}
u64 Lock_get_tid()
{
	u64 tid;
#ifdef __APPLE__
	pthread_threadid_np(NULL, &tid);
#else
	tid = gettid();
#endif // tid code
	return tid;
}

LockGuard lock(Lock *ptr)
{
	if (atomic_load(&ptr->poison))
		panic("Lock %p: poisoned!", ptr);

	u64 tid = Lock_get_tid();

	// check if this would be a deadlock
	if (atomic_load(&ptr->is_locked) && atomic_load(&ptr->tid) == tid)
		panic("Lock %p: attempt to lock would deadlock!", lock);

	// obtain lock
	pthread_mutex_lock(&ptr->lock);
	if (atomic_load(&ptr->poison))
		panic("Lock %p: poisoned!", ptr);

	insert_active_lock(ptr);

	// set tid/is_locked and return
	atomic_exchange(&ptr->tid, tid);
	atomic_exchange(&ptr->is_locked, true);
	LockGuardPtr ret = { ptr };
	return ret;
}

void Lockguard_cleanup(LockGuardPtr *ptr)
{
	if (ptr && ptr->ref) {
		atomic_exchange(&ptr->ref->is_locked, false);
		pthread_mutex_unlock(&ptr->ref->lock);
		delete_active_lock(ptr->ref);
	}
}

void Lock_mark_poisoned()
{
	if (__active_lock_count_) {
		u64 i = __active_lock_count_ - 1;
		while (true) {
			Lock_set_poison(__active_locks_[i]);
			atomic_exchange(&__active_locks_[i]->is_locked, false);
			pthread_mutex_unlock(&__active_locks_[i]->lock);
			if (i == 0)
				break;
			i--;
		}
	}
}

void Lock_wait(Lock *ptr, u64 nanoseconds)
{
	u64 tid = Lock_get_tid();
	if (!(atomic_load(&ptr->is_locked) && atomic_load(&ptr->tid) == tid))
		panic("Attempt to wait on lock %p without first obtaining the "
			  "lock!",
			ptr);
	if (nanoseconds == 0) {
		pthread_cond_wait(&ptr->cond, &ptr->lock);
	} else {
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);

		// Convert the wait time into seconds and nanoseconds
		ts.tv_sec += nanoseconds / 1000000000;
		ts.tv_nsec += nanoseconds % 1000000000;

		// Normalize the timespec structure in case of overflow in
		// nanoseconds
		if (ts.tv_nsec >= 1000000000) {
			ts.tv_sec += ts.tv_nsec / 1000000000;
			ts.tv_nsec %= 1000000000;
		}
		pthread_cond_timedwait(&ptr->cond, &ptr->lock, &ts);
	}
}

void Lock_notify(Lock *ptr)
{
	pthread_cond_signal(&ptr->cond);
}

void Lock_notify_all(Lock *ptr)
{
	pthread_cond_broadcast(&ptr->cond);
}
