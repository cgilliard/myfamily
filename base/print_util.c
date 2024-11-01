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

#include <base/osdef.h>
#include <base/print_util.h>

void __attribute__((no_return)) panic(const char *fmt, ...) {
	char buf[1024];
	__builtin_va_list args;
	print("Panic: ");
	__builtin_va_start(args, fmt);
	// vfprintf(stderr, fmt, args);
	// vsnprintf(buf, 1024, fmt, args);
	__builtin_va_end(args);
	print("\n");

	exit(-1);
}

int println(const char *text) {
	int size = 0;
	while (text && text[size])
		size++;
	if (write(1, text, size) < 0)
		return -1;
	if (write(1, "\n", 1) < 0)
		return -1;
	return 0;
}

int print(const char *text) {
	int size = 0;
	while (text && text[size])
		size++;
	if (write(1, text, size) < 0)
		return -1;
	return 0;
}
