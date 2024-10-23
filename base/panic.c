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

// #include <base/backtrace.h>
#include <base/mem_util.h>
#include <base/panic.h>
#include <base/print_util.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>

void exit(int value);

void panic(const u8 *fmt, ...) {
	va_list args;
	va_list args_copy;
	va_copy(args_copy, args);

	va_start(args, fmt);
	int len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	u8 buf[len + 1];
	va_start(args_copy, fmt);
	vsnprintf(buf, len + 1, fmt, args);
	va_end(args_copy);
	printf("Panic: %s\n", buf);

	/*
		Backtrace bt;
		backtrace_generate(&bt);
		backtrace_print(&bt);
	*/

	exit(-1);
}

void signal_handler(int signal) {
	// Handle the signal and call the panic function
	panic("Caught signal: %d", signal);
}

void __attribute__((constructor)) __set_on_panic__() {
	struct sigaction sa;
	memzero(&sa, sizeof(sa));
	sa.sa_handler = signal_handler;
	// Catching only error signals
	sigaction(SIGSEGV, &sa, NULL); // Segmentation fault
	sigaction(SIGABRT, &sa, NULL); // Abort signal
	sigaction(SIGTRAP, &sa, NULL); // Signal trap
	sigaction(SIGILL, &sa, NULL);  // Illegal instruction
	sigaction(SIGFPE, &sa, NULL);  // Arithmetic error
	sigaction(SIGBUS, &sa, NULL);  // Bus error
}
