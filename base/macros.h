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

#include <base/macro_util.h>

// Type macros
#define Type(type)                                                                                 \
	typedef struct Type *type##Nc;                                                                 \
	void type##_cleanup(const type##Nc *ptr);
#define DefineType(type) type##Nc __attribute((warn_unused_result, cleanup(type##_cleanup)))

// Ptr macros
#define $(ptr) ptr_data(ptr)
#define $len(ptr) ptr_len(ptr)
#define nil(ptr) (ptr == NULL || (ptr_len(ptr) == UINT32_MAX && ptr_id(ptr) == 0))
#define ok(ptr) !nil(ptr)
#define initialized(ptr) (ptr != NULL && ptr_len(ptr) != UINT32_MAX)

// Object macros
#define $int(obj) *(int *)object_value_of(obj)
#define move(src)                                                                                  \
	({                                                                                             \
		ObjectNc _ret__ = object_move(src);                                                        \
		src = null;                                                                                \
		_ret__;                                                                                    \
	})
#define ref(src) object_ref(src)
#define weak(src) object_weak(src)
#define upgrade(src) object_upgrade(src)

// Lock macros
#define lock() (lock_create())
#define lockr(l) lock_read(l)
#define lockw(l) lock_write(l)
#define unlock(l) lock_unlock(l)
#define notify(l) lock_notify(l)
#define lwait(l, ...) __VA_OPT__(lock_wait_timeout(l, __VA_ARGS__) NONE)(lock_wait(l))
#define rsync(l, exe)                                                                              \
	({                                                                                             \
		LockGuard l##_lg = lock_guard_read(l);                                                     \
		{ exe }                                                                                    \
	})
#define wsync(l, exe)                                                                              \
	({                                                                                             \
		LockGuard l##_lg = lock_guard_write(l);                                                    \
		{ exe }                                                                                    \
	})

#endif // _BASE_MACROS__
