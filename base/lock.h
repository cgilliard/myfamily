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

#ifndef _BASE_LOCK__
#define _BASE_LOCK__

#define INIT_LOCK (lock_create())

typedef unsigned long long Lock;

Lock lock_create();
void lock_read(Lock *lock);
void lock_write(Lock *lock);
void lock_unlock(Lock *lock);

void lock_guard_cleanup(Lock *lg);

#define LockGuard \
	Lock __attribute__((warn_unused_result, cleanup(lock_guard_cleanup)))

LockGuard lock_guard_read(Lock l);
LockGuard lock_guard_write(Lock l);

#endif	// _BASE_LOCK__
