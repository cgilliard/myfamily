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

void copy_bytes(byte *dst, const byte *src, uint64 n) {
	for (uint64 i = 0; i < n; i++) {
		dst[i] = src[i];
	}
}

void set_bytes(byte *ptr, byte x, uint64 n) {
	for (uint64 i = 0; i < n; i++) ptr[i] = x;
}

uint64 cstring_len(const char *S) {
	const char *s = S;
	while (*S) S++;
	return S - s;
}

int cstring_compare(const char *X, const char *Y) {
	while (*X && *X == *Y) {
		X++;
		Y++;
	}
	if (*X > *Y) return 1;
	if (*Y > *X) return -1;
	return 0;
}

int cstring_compare_n(const byte *X, const byte *Y, uint64 n) {
	while (n && *X == *Y) {
		n--;
		X++;
		Y++;
	}
	if (n == 0)
		return 0;
	else if (*X > *Y)
		return 1;
	else
		return -1;
}
