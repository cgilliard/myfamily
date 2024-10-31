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

#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/lock.h>
#include <base/osdef.h>
#include <base/print_util.h>

#include <errno.h>
#include <pthread.h>

#include <stdio.h>

typedef struct LockImpl {
	pthread_rwlock_t rw_lock;
	pthread_mutex_t mutex_lock;
	pthread_cond_t cond;
	bool condition;
	bool send;
} LockImpl;

Lock lock_create(bool send) {
	Ptr ret = fam_alloc(sizeof(LockImpl), send);
	errno = 0;
	if (nil(ret))
		return ret;
	LockImpl *l = $(ret);
	if (pthread_rwlock_init(&l->rw_lock, NULL)) {
		SetErr(LockInitErr);
		fam_release(&ret);
		return NULL;
	}
	if (pthread_mutex_init(&l->mutex_lock, NULL)) {
		SetErr(LockInitErr);
		pthread_mutex_destroy(&l->mutex_lock);
		fam_release(&ret);
		return NULL;
	}

	if (pthread_cond_init(&l->cond, NULL)) {
		SetErr(LockInitErr);
		pthread_mutex_destroy(&l->mutex_lock);
		pthread_rwlock_destroy(&l->rw_lock);
		fam_release(&ret);
		return NULL;
	}
	l->condition = false;
	l->send = send;

	return ret;
}

void lock_condtion_reset(Lock lock) {
	LockImpl *l = $(lock);
	l->condition = false;
}

void lock_read(Lock lock) {
	LockImpl *l = $(lock);
	int code;
	errno = 0;
	if ((code = pthread_rwlock_rdlock(&l->rw_lock)))
		panic("pthread_rwlock_rdlock: returned error: %i (%i)", code, errno);
}
void lock_write(Lock lock) {
	LockImpl *l = $(lock);
	int code;
	errno = 0;
	if ((code = pthread_rwlock_wrlock(&l->rw_lock)))
		panic("pthread_rwlock_wrlock: returned error: %i (%i)", code, errno);
}

void lock_unlock(Lock lock) {
	LockImpl *l = $(lock);
	int code;
	errno = 0;
	if ((code = pthread_rwlock_unlock(&l->rw_lock)))
		panic("pthread_rwlock_unlock: returned error: %i (%i)", code, errno);
}

void lock_notify(Lock lock) {
	LockImpl *l = $(lock);
	int code;
	l->condition = true;
	errno = 0;
	if ((code = pthread_cond_signal(&l->cond)))
		panic("pthread_cond_signal: returned error: %i (%i)", code, errno);
}

void lock_wait(Lock lock) {
	int code;
	LockImpl *l = $(lock);
	errno = 0;
	if ((code = pthread_mutex_lock(&l->mutex_lock)))
		panic("pthread_mutex_lock: returned error: %i (%i)", code, errno);
	while (!l->condition) {
		errno = 0;
		if ((code = pthread_rwlock_rdlock(&l->rw_lock)))
			panic("pthread_rwlock_rdlock: returned error: %i (%i)", code, errno);
		if ((code = pthread_cond_wait(&l->cond, &l->mutex_lock)))
			panic("pthread_cond_wait: returned error: %i (%i)", code, errno);
		if ((code = pthread_rwlock_unlock(&l->rw_lock)))
			panic("pthread_rwlock_unlock: returned error: %i (%i)", code, errno);
	}
	l->condition = false;
	errno = 0;
	if ((code = pthread_mutex_unlock(&l->mutex_lock)))
		panic("pthread_rwlock_unlock: returned error: %i (%i)", code, errno);
}

void lock_wait_timeout(Lock lock, unsigned int milliseconds) {
	int code;
	struct timespec timeout;
	if ((code = clock_gettime(CLOCK_REALTIME, &timeout)))
		panic("clock_gettime: returned error: %i (%i)", code, errno);

	// Calculate timeout in seconds and nanoseconds
	timeout.tv_sec += milliseconds / 1000;
	timeout.tv_nsec += (milliseconds % 1000) * 1000000;

	// Normalize the timespec structure (handle nanosecond overflow)
	if (timeout.tv_nsec >= 1000000000) {
		timeout.tv_sec++;
		timeout.tv_nsec -= 1000000000;
	}

	LockImpl *l = $(lock);
	if ((code = pthread_mutex_lock(&l->mutex_lock)))
		panic("pthread_mutex_lock: returned error: %i (%i)", code, errno);
	while (!l->condition) {
		if ((code = pthread_rwlock_rdlock(&l->rw_lock)))
			panic("pthread_rwlock_rdlock: returned error: %i (%i)", code, errno);
		if ((code = pthread_cond_timedwait(&l->cond, &l->mutex_lock, &timeout)))
			panic("pthread_cond_timedwait: returned error: %i (%i)", code, errno);
		if ((code = pthread_rwlock_unlock(&l->rw_lock)))
			panic("pthread_rwlock_unlock: returned error: %i (%i)", code, errno);
	}
	l->condition = false;
	if ((code = pthread_mutex_unlock(&l->mutex_lock)))
		panic("pthread_mutex_unlock: returned error: %i (%i)", code, errno);
}

typedef struct LockGuardImpl {
	Lock lock;
} LockGuardImpl;

LockGuard lock_guard_read(Lock l) {
	LockImpl *li = $(l);
	Ptr ret = fam_alloc(sizeof(LockGuardImpl), li->send);
	LockGuardImpl *lgi = $(ret);
	lgi->lock = l;
	lock_read(lgi->lock);
	return ret;
}
LockGuard lock_guard_write(Lock l) {
	LockImpl *li = $(l);
	Ptr ret = fam_alloc(sizeof(LockGuardImpl), li->send);
	LockGuardImpl *lgi = $(ret);
	lgi->lock = l;
	lock_write(lgi->lock);
	return ret;
}

// Functions that require override of const
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

void Lock_cleanup(const Lock *lock) {
	errno = 0;
	if (initialized(*lock)) {
		LockImpl *l = $(*lock);
		int code;
		if ((code = pthread_rwlock_destroy(&l->rw_lock)))
			panic("pthread_rwlock_destroy: returned error: %i (%i)", code, errno);
		if ((code = pthread_mutex_destroy(&l->mutex_lock)))
			panic("pthread_mutex_destroy: returned error: %i (%i)", code, errno);
		if ((code = pthread_cond_destroy(&l->cond)))
			panic("pthread_cond_destroy: returned error: %i (%i)", code, errno);
		fam_release(lock);
	}
}

void LockGuard_cleanup(const LockGuard *lg) {
	if (initialized(*lg)) {
		LockGuardImpl *lgi = $(*lg);
		lock_unlock(lgi->lock);
		fam_release(lg);
	}
}
