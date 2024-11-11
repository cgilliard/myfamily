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

#include <base/test.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BACKTRACE_ENTRIES 128

int test_count = 0;
test_fn_ptr test_arr[MAX_TESTS + 1];
byte test_names[MAX_TESTS][MAX_TEST_NAME + 1];
static jmp_buf test_jmp;
static int test_itt;
static int fail_count = 0;

bool execute_tests(byte *name) {
	for (int i = 0; i < test_count; i++) {
		test_itt = i;
		if (setjmp(test_jmp) == 0) {
			test_arr[i]("test_dir", "resources_dir");
		} else {
			println("%sFAIL:%s test '%s%s%s' failed!", BRIGHT_RED, RESET, GREEN,
					test_names[i], RESET);
			fail_count++;
		}
	}

	if (fail_count)
		println(
			"--------------------------------"
			"--------------------------------"
			"--------------------------------"
			"--------------------");
	println("[%s====%s] Tested: %s%i%s | Passing: %s%i%s | Failing: %s%i%s",
			BLUE, RESET, YELLOW, test_count, RESET, GREEN,
			test_count - fail_count, RESET, BRIGHT_RED, fail_count, RESET);

	println(
		"[%s================================"
		"================================================"
		"===================================%s]",
		BLUE, RESET);

	if (fail_count != 0)
		println("%sFAIL:%s Test suite %s%s%s failed!", BRIGHT_RED, RESET, GREEN,
				name, RESET);

	return fail_count == 0;
}

void fail_assert() {
	if (fail_count == 0)
		println(
			"--------------------------------"
			"--------------------------------"
			"--------------------------------"
			"--------------------");
	void *array[MAX_BACKTRACE_ENTRIES];
	int size = backtrace(array, MAX_BACKTRACE_ENTRIES);
	char **strings = backtrace_symbols(array, size);

	for (int i = 0; i < size; i++) {
		char address[256];
#ifdef __linux__  // LINUX
		int len = cstring_len(strings[i]);
		int last_plus = -1;
		while (len > 0) {
			if (strings[i][len] == '+') {
				last_plus = len;
				break;
			}
			len--;
		}
		if (last_plus > 0) {
			byte *addr = strings[i] + last_plus + 1;
			int itt = 0;
			while (addr[itt]) {
				if (addr[itt] == ')') {
					addr[itt] = 0;
					break;
				}
				itt++;
			}
			char command[256];
			snprintf(command, sizeof(command), "addr2line -f -e bin/test %s",
					 addr);

			FILE *fp = popen(command, "r");
			char buffer[128];
			char output[1024 * 2];
			int n = sizeof(output);
			strcpy(output, "");
			while (n && fgets(buffer, sizeof(buffer), fp) != NULL) {
				strncat(output, buffer, n);
				int buf_len = strlen(buffer);
				if (buf_len < n)
					n -= buf_len;
				else
					n = 0;
			}
			if (strstr(output, "_tfwork_"))
				print("Assertion failure: %s", output);
			pclose(fp);
		}
#else	// MACOS
		Dl_info info;
		dladdr(array[i], &info);
		unsigned long long addr =
			0x0000000100000000 + info.dli_saddr - info.dli_fbase;
		unsigned long long offset =
			(uintptr_t)array[i] - (uintptr_t)info.dli_saddr;
		addr += offset;
		addr -= 4;
		snprintf(address, sizeof(address), "0x%" PRIx64 "", addr);
		if (strstr(strings[i], "_tfwork_")) {
			char command[256];
			snprintf(command, sizeof(command),
					 "atos -fullPath -o bin/test -l 0x100000000 %s", address);

			FILE *fp = popen(command, "r");
			char buffer[128];
			print("Assertion failure: ");
			while (fgets(buffer, sizeof(buffer), fp) != NULL) {
				print("%s", buffer);  // Print each line from atos output
			}

			pclose(fp);
			break;
		}
#endif	// MACOS
	}

	free(strings);

	longjmp(test_jmp, 1);
}
