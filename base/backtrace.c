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

#include <base/backtrace.h>
#include <base/fam_err.h>
#include <base/print_util.h>
#include <base/types.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <inttypes.h>
#include <stdio.h>

// int backtrace(void *buffer, int size);
//  byte **backtrace_symbols(void *const buffer, int size);
void free(void *);

#define MAX_ENTRIES 128
#define MAX_ENTRY_SIZE 1024

typedef struct BacktraceEntry {
	unsigned int start_bin;
	unsigned int start_addr;
	unsigned int start_file_path;
	unsigned int start_line_num;
	byte data[MAX_ENTRY_SIZE + 1];
} BacktraceEntry;

#define BACKTRACE_IMPL_SIZE 1024
typedef struct BacktraceImpl {
	int count;
} BacktraceImpl;

int backtrace_generate(Backtrace *ptr) {
	if (ptr == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	void *array[MAX_ENTRIES];
	int size = backtrace(array, MAX_ENTRIES);
	if (size < 0) {
		SetErr(BacktraceErr);
		return -1;
	}

	char **strings = backtrace_symbols(array, size);

	if (strings == NULL) size = 0;

	for (int i = 0; i < size; i++) {
		char address[30];
		Dl_info info;
		dladdr(array[i], &info);
		unsigned long long addr =
			0x0000000100000000 + info.dli_saddr - info.dli_fbase;
		snprintf(address, 30, "0x%" PRIx64 "", addr);
		println("%s %s\n", strings[i], address);
	}

	if (strings) free(strings);

	return 0;
}

void backtrace_print(const Backtrace *ptr) {
}
