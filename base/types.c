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
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

bool __is_little_endian() {
	u16 test = 0x1;
	return (*(u8 *)&test == 0x1);
}

void __attribute__((constructor)) __check_64bit_arch__() {
	if (sizeof(size_t) != 8) {
		fprintf(stderr, "Error: This program requires a 64-bit architecture to run.\n");
		exit(EXIT_FAILURE);
	}

	if (sizeof(u8) != 1 || sizeof(u16) != 2 || sizeof(u32) != 4 || sizeof(u64) != 8 ||
		sizeof(u128) != 16 || sizeof(i8) != 1 || sizeof(i16) != 2 || sizeof(i32) != 4 ||
		sizeof(i64) != 8 || sizeof(i128) != 16 || sizeof(f32) != 4 || sizeof(f64) != 8 ||
		sizeof(bool) != 1) {
		fprintf(stderr, "Invalid data type size! Check your c compiler configuration options.\n");
		exit(EXIT_FAILURE);
	}

	if (!__is_little_endian()) {
		fprintf(stderr, "Big endian systems not supported");
		exit(EXIT_FAILURE);
	}
}
