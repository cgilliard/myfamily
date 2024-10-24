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
#include <stdio.h>
#include <stdlib.h>

bool __is_little_endian() {
	u16 test = 0x1;
	return (*(u8 *)&test == 0x1);
}

void __attribute__((constructor)) __check_64bit_arch__() {
#if !defined(__x86_64__) && !defined(_M_X64) && !defined(__aarch64__)
	printf("Error: This program requires a 64-bit architecture to run.\n");
	exit(-1);
#endif // arch

	if (__SIZEOF_SIZE_T__ != 8) {
		printf("Error: This program requires a 64-bit architecture to run.");
		exit(-1);
	}
	if (sizeof(u8) != 1 || sizeof(u16) != 2 || sizeof(u32) != 4 || sizeof(u64) != 8 ||
		sizeof(u128) != 16 || sizeof(i8) != 1 || sizeof(i16) != 2 || sizeof(i32) != 4 ||
		sizeof(i64) != 8 || sizeof(i128) != 16 || sizeof(f32) != 4 || sizeof(f64) != 8 ||
		sizeof(bool) != 1) {
		printf("Invalid data type size! Check your c compiler configuration options.\n");
		exit(-1);
	}

	if (!__is_little_endian()) {
		printf("Big endian systems not supported");
		exit(-1);
	}

	if (sizeof(au64) != 8) {
		printf("Error: Unexpected size for atomic_ullong. Check your compiler and architecture.\n");
		exit(-1);
	}
	if (sizeof(au32) != 4) {
		printf("Error: Unexpected size for atomic_uint. Check your compiler and architecture.\n");
		exit(-1);
	}
}
