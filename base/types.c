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
#include <base/types.h>
#include <stdio.h>

void __attribute__((constructor)) __check_64bit_arch__() {
#if !defined(__x86_64__) && !defined(_M_X64) && !defined(__aarch64__)
	panic("Supported architectures: __x86_64__, _M_X64, and __aarch64__");
#endif // arch

	// check size_t
	if (__SIZEOF_SIZE_T__ != 8)
		panic("size_t must be 8 bytes. Invalid arch!");
	// check primitive types
	if (sizeof(byte) != 1)
		panic("byte must be 1 byte. Invalid arch!");

	if (sizeof(int64) != 8)
		panic("must be 8 bytes. Invalid arch!");

	if (sizeof(int) != 4)
		panic("int must be 4 bytes. Invalid arch!");

	if (sizeof(float64) != 8)
		panic("float64 must be 8 bytes. Invalid arch!");

	if (sizeof(bool) != 1)
		panic("bool must be 1 byte. Invalid arch!");

	if (sizeof(aint64) != 8)
		panic("aint64 must be 8 bytes. Invalid arch!");

	if (sizeof(abool) != 1)
		panic("abool must be 1 byte. Invalid arch!");

	// little endian check
	int test = 0x1;
	if (*(byte *)&test != 0x1) {
		panic("Big endian is not supported!");
	}
}
