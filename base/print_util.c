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

void exit(int);

void panic(const char *fmt, ...) {
	char buf[1024];
	__builtin_va_list args;
	print("Panic: ");
	__builtin_va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	__builtin_va_end(args);
	fprintf(stderr, "\n");

	exit(-1);
}

int println(const char *fmt, ...) {
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	__builtin_va_end(args);
	fprintf(stdout, "\n");
	return 0;
}

int print(const char *fmt, ...) {
	__builtin_va_list args;
	__builtin_va_start(args, fmt);
	vfprintf(stdout, fmt, args);
	__builtin_va_end(args);
	return 0;
}
