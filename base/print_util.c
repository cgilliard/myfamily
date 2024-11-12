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

#include <base/print_util.h>

// currently using stdio, will move to write
#include <stdio.h>

bool _debug_print_util_disable__ = false;

void exit(int);

void panic(const byte *fmt, ...) {
	byte buf[1024];
	__builtin_va_list args;
	print("Panic: ");
	__builtin_va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	__builtin_va_end(args);
	fprintf(stderr, "\n");

	exit(-1);
}

int println(const byte *fmt, ...) {
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	int ret;
	if (!_debug_print_util_disable__) ret = vfprintf(stdout, fmt, args);
	__builtin_va_end(args);
	if (!_debug_print_util_disable__) fprintf(stdout, "\n");
	return ret;
}

int print(const byte *fmt, ...) {
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	int ret = 0;
	if (!_debug_print_util_disable__) ret = vfprintf(stdout, fmt, args);
	__builtin_va_end(args);
	return ret;
}

int sprint(byte *str, unsigned long long capacity, const byte *fmt, ...) {
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	int ret = 0;
	if (!_debug_print_util_disable__) ret = vsnprintf(str, capacity, fmt, args);
	__builtin_va_end(args);
	return ret;
}
