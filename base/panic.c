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

#include <base/backtrace.h>
#include <base/panic.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void exit(int value);

#define MAX_PANIC_MSG 1024

_Thread_local jmp_buf return_jmp;
_Thread_local bool jmp_return_set = false;
_Thread_local char panic_buf[MAX_PANIC_MSG];

void panic(const char *fmt, ...) {
	va_list args;

	fprintf(stderr, "application panicked: ");
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	vsnprintf(panic_buf, MAX_PANIC_MSG, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");

	Backtrace bt;
	backtrace_generate(&bt);
	backtrace_print(&bt);

	// If jump return has not been set, we resort to an exit with an error
	// status.
	if (!jmp_return_set)
		exit(-1);
	longjmp(return_jmp, THREAD_PANIC);
}

void signal_handler(int signal) {
	// Handle the signal and call the _on_panic function
	panic("Caught signal: %d", signal);
}

void set_on_panic(panic_handler_t on_panic) {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signal_handler;
	// Catching only error signals
	sigaction(SIGSEGV, &sa, NULL); // Segmentation fault
	sigaction(SIGABRT, &sa, NULL); // Abort signal
	sigaction(SIGTRAP, &sa, NULL); // Signal trap
	sigaction(SIGILL, &sa, NULL);  // Illegal instruction
	sigaction(SIGFPE, &sa, NULL);  // Arithmetic error
	sigaction(SIGBUS, &sa, NULL);  // Bus error

	jmp_return_set = true;
	int value = setjmp(return_jmp);
	if (value)
		on_panic(panic_buf);
}
