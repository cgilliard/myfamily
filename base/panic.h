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

#ifndef _CORE_PANIC__
#define _CORE_PANIC__

#include <base/types.h>
#include <setjmp.h>

extern _Thread_local jmp_buf return_jmp;
extern _Thread_local bool jmp_return_set;

#define THREAD_PANIC 1

#ifdef TEST
extern bool __debug_no_exit;
#endif // TEST

void panic(const char *format, ...);

#define PANIC_RETURN()                                                                             \
	({                                                                                             \
		jmp_return_set = true;                                                                     \
		int _value__ = setjmp(return_jmp);                                                         \
		_value__ != 0;                                                                             \
	})

#endif // _CORE_PANIC__
