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

#include <base/fam.h>
#include <base/print_util.h>
#include <sys/types.h>	// for size_t

int vsnprintf(char *s, size_t n, const char *formt, va_list args);
#define va_end(...) __builtin_va_end(__VA_ARGS__)
#define va_start(...) __builtin_va_start(__VA_ARGS__)

int _debug_print_util_disable__ = 0;

#include <stdio.h>

long long prot_send(char *buf, long long len) {
	printf("%s", buf);
	return 0;
}

void panic(const char *fmt, ...) {
	prot_send("Panic: ", 7);
	va_list args;
	va_start(args, fmt);
	vprint(fmt, args);
	va_end(args);
	prot_send("\n", 1);
	if (!_debug_print_util_disable__) halt(-1);
}

int println(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vprint(fmt, args);
	va_end(args);
	prot_send("\n", 1);
	return ret;
}

int print(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vprint(fmt, args);
	va_end(args);
	return ret;
}

int vprint(const char *fmt, va_list args) {
	va_list args2;
	__builtin_va_copy(args2, args);
	int reqd = vsprint(0, 0, fmt, args);
	if (reqd < 0) return reqd;
	char buf[reqd + 1];
	int len = vsprint(buf, reqd + 1, fmt, args2);
	va_end(args2);
	return prot_send(buf, len);
}

int sprint(char *str, unsigned long long capacity, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vsprint(str, capacity, fmt, args);
	va_end(args);
	return ret;
}

int vsprint(char *str, unsigned long long capacity, const char *fmt,
			va_list args) {
	return vsnprintf(str, capacity, fmt, args);
}
