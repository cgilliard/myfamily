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

// Type macros
#define Type(type)                 \
	typedef struct Type *type##Nc; \
	void type##_cleanup(const type##Nc *ptr);
#define DefineType(type) \
	type##Nc __attribute((warn_unused_result, cleanup(type##_cleanup)))

// Ptr macros
#define nil(ptr) (ptr <= ptr_reserved)
#define ok(ptr) !nil(ptr)

// Lock macros
#define lock() (lock_create())
#define lockr(l) lock_read(l)
#define lockw(l) lock_write(l)
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
								__ATOMIC_RELAXED)
#define CAS_ACQUIRE(...)                                              \
	__atomic_compare_exchange_n(__VA_ARGS__, false, __ATOMIC_ACQUIRE, \
								__ATOMIC_RELAXED)
#define CAS_RELEASE(...)                                              \
	__atomic_compare_exchange_n(__VA_ARGS__, false, __ATOMIC_RELEASE, \
								__ATOMIC_RELAXED)
#define ALOAD(ptr) __atomic_load_n(ptr, __ATOMIC_RELAXED)
#define ASTORE(ptr, value) __atomic_store_n(ptr, value, __ATOMIC_RELAXED)
#define AADD(ptr, value) __atomic_fetch_add(ptr, value, __ATOMIC_RELAXED)
#define ASUB(ptr, value) __atomic_fetch_sub(ptr, value, __ATOMIC_RELAXED)

// MMAP
#define MMAP_ALIGNED_SIZE(size)                                    \
	({                                                             \
		unsigned int page_size = getpagesize();                    \
		(((unsigned int)size) + page_size - 1) & ~(page_size - 1); \
	})
#define MMAP(size)                                                  \
	({                                                              \
		mmap(NULL, MMAP_ALIGNED_SIZE(size), PROT_READ | PROT_WRITE, \
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);                   \
	})
#define MUNMAP(ptr, size) munmap(ptr, MMAP_ALIGNED_SIZE(size))

// SPECTRE PROTECT
#if defined(__GNUC__)
#define SPECTRE_PROTECT __attribute__((noclone, nospeculative_load_hardening))
#elif defined(__clang__)
#define SPECTRE_PROTECT \
	__attribute__((noclone))  // Clang doesn't have nospeculative_load_hardening
#else
#define SPECTRE_PROTECT	 // No special attribute for other compilers
#endif

#endif	// _BASE_MACROS__
