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

#include <base/types.h>
#include <base/util.h>

// note: assume non-null strings passed in
void copy_bytes(byte *dst, const byte *src, unsigned long long n) {
	for (unsigned long long i = 0; i < n; i++) {
		dst[i] = src[i];
	}
}

// note: assume non-null strings passed in
void set_bytes(byte *ptr, byte x, unsigned long long n) {
	for (unsigned long long i = 0; i < n; i++) ptr[i] = x;
}

// note: assume non-null strings passed in and null terminated
unsigned long long cstring_len(const char *S) {
	const char *s = S;
	while (*S) S++;
	return S - s;
}

// note: assume non-null strings passed in and null terminated
int cstring_compare(const char *X, const char *Y) {
	while (*X && *X == *Y) {
		X++;
		Y++;
	}
	if (*X > *Y) return 1;
	if (*Y > *X) return -1;
	return 0;
}

// we compare byte by byte for length n, even if null
int cstring_compare_n(const byte *X, const byte *Y, unsigned long long n) {
	while (n && *X == *Y) {
		n--;
		X++;
		Y++;
	}
	if (n == 0) return 0;
	if (*X > *Y) return 1;
	if (*Y > *X) return -1;
	return 0;
}