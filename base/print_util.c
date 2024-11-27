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
#include <base/object.h>
#include <base/print_util.h>
#include <base/sys.h>
#include <base/util.h>

// int vsnprintf(char *s, size_t n, const char *formt, va_list args);
int printf(const char *fmt, ...);
#define va_end(...) __builtin_va_end(__VA_ARGS__)
#define va_start(...) __builtin_va_start(__VA_ARGS__)
#define va_arg(...) __builtin_va_arg(__VA_ARGS__)

long long print_impl(Channel *channel, char *buffer, long long capacity,
					 int newline, int exit, const char *prefix, const char *fmt,
					 ...) {
	va_list args;
	va_start(args, fmt);
	Printable p;

	do {
		p = va_arg(args, Printable);
		printf("p.type=%i,", p.type);
		if (p.type == PrintTypeInt) printf("intv=%lli", p.value.int_value);
		if (p.type == PrintTypeUInt) printf("uintv=%llu", p.value.uint_value);
		if (p.type == PrintTypeFloat) printf("floatv=%f", p.value.float_value);
		if (p.type == PrintTypeString)
			printf("stringv=%s", p.value.string_value);
		printf("\n");

	} while (p.type != PrintTypeTerm);

	va_end(args);
	return 0;
}
