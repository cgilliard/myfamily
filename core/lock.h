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

#ifndef _CORE_LOCK__
#define _CORE_LOCK__

#include <core/types.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct LockPtr {
	pthread_mutex_t lock;
	atomic_ullong tid;
	atomic_bool is_locked;
	atomic_bool poison;
} LockPtr;

void Lock_cleanup(LockPtr *ptr);

#define Lock LockPtr __attribute__((warn_unused_result, cleanup(Lock_cleanup)))

void Lock_set_poison(Lock *ptr);
bool Lock_is_poisoned(Lock *ptr);
void Lock_clear_poison(Lock *ptr);
void Lock_mark_poisoned();

typedef struct LockGuardPtr {
	LockPtr *ref;
} LockGuardPtr;

void Lockguard_cleanup(LockGuardPtr *ptr);
Lock Lock_build();

#define LockGuard                                                              \
	LockGuardPtr                                                           \
	    __attribute__((warn_unused_result, cleanup(Lockguard_cleanup)))

LockGuard lock(Lock *lock);

#define LOCK() Lock_build()
#define LOCK_GUARD_CLEANUP_SAFE {NULL};

#endif // _CORE_LOCK__
