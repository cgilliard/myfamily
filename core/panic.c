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

#include <core/chain_allocator.h>
#include <core/lock.h>
#include <core/panic.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

_Thread_local jmp_buf return_jmp;
_Thread_local bool jmp_return_set = false;

void panic(const char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "thread panicked: ");
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");

	Lock_mark_poisoned();

	if (__default_tl_heap_allocator != NULL)
	{
		heap_allocator_cleanup(__default_tl_heap_allocator);
	}

	// If jump return has not been set, we resort to an exit with an error
	// status.
	if (!jmp_return_set)
		exit(-1);
	longjmp(return_jmp, THREAD_PANIC);
}
