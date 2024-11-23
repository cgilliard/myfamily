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

#ifndef _BASE_MACROS__
#define _BASE_MACROS__

#include <base/types.h>

// offsetof
#define offsetof(x, y) __builtin_offsetof(x, y)

// Lock macros
#define lock() (lock_create())
#define lockr(l) lock_read(l)
#define lockw(l) lock_write(l)
#define locku(l) lock_upgrade(l)
#define lockd(l) lock_downgrade(l)
#define unlock(l) lock_unlock(l)
#define rsync(l, exe)                          \
	({                                         \
		LockGuard l##_lg = lock_guard_read(l); \
		{ exe }                                \
	})
#define wsync(l, exe)                           \
	({                                          \
		LockGuard l##_lg = lock_guard_write(l); \
		{ exe }                                 \
	})

// CAS
#define CAS(...)                                                      \
	__atomic_compare_exchange_n(__VA_ARGS__, false, __ATOMIC_RELAXED, \
								__ATOMIC_RELAXED)
#define CAS_SEQ(...)                                                  \
	__atomic_compare_exchange_n(__VA_ARGS__, false, __ATOMIC_SEQ_CST, \
								__ATOMIC_SEQ_CST)
#define CAS_ACQUIRE(...)                                              \
	__atomic_compare_exchange_n(__VA_ARGS__, false, __ATOMIC_ACQUIRE, \
								__ATOMIC_ACQUIRE)
#define CAS_RELEASE(...)                                              \
	__atomic_compare_exchange_n(__VA_ARGS__, false, __ATOMIC_RELEASE, \
								__ATOMIC_RELAXED)
#define ALOAD(ptr) __atomic_load_n(ptr, __ATOMIC_ACQUIRE)
#define ASTORE(ptr, value) __atomic_store_n(ptr, value, __ATOMIC_RELEASE)
#define AADD(ptr, value) __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST)
#define ASUB(ptr, value) __atomic_fetch_sub(ptr, value, __ATOMIC_SEQ_CST)

#endif	// _BASE_MACROS__
