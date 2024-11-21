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

#ifdef __linux__
#define _XOPEN_SOURCE 500
#endif	// __linux__
#include <base/lib.h>
#include <base/test.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <ftw.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_BACKTRACE_ENTRIES 128

int test_count = 0;
test_fn_ptr test_arr[MAX_TESTS + 1];
byte test_names[MAX_TESTS][MAX_TEST_NAME + 1];
static jmp_buf test_jmp;
static int test_itt;
int fail_count = 0;
static byte target_test[MAX_TEST_NAME + 1];
extern char **environ;
extern int _gfd;

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag,
			  struct FTW *ftwbuf) {
	int rv = remove(fpath);

	if (rv) perror(fpath);

	return rv;
}

int rmrf(char *path) {
	return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

static void __attribute__((constructor)) get_target_test() {
	target_test[0] = 0;
	for (int i = 0; environ[i] != NULL; i++) {
		byte *env_var = environ[i];
		if (!cstring_compare_n("TEST_FILTER=", env_var, 12)) {
			int env_var_len = cstring_len(env_var);
			if (env_var_len > 12) {
				int len = env_var_len + 12;
				if (len > MAX_TEST_NAME) len = MAX_TEST_NAME;
				copy_bytes(target_test, env_var + 12, len);
				break;
			}
		}
	}
}

bool execute_tests(byte *name) {
	struct timespec start, end;
	bool success[test_count];
	clock_gettime(CLOCK_MONOTONIC, &start);
	int test_exe_count = 0;

	for (int i = 0; i < test_count; i++) {
		test_itt = i;
		if (setjmp(test_jmp) == 0) {
			if (target_test[0] == 0 ||
				!cstring_compare(target_test, test_names[i])) {
				test_exe_count++;
				int64 start_alloc = _alloc_sum;
				int test_name_len = cstring_len(test_names[i]);
				char test_dir[test_name_len + 100];
				copy_bytes(test_dir, "./.", 3);
				copy_bytes(test_dir + 3, test_names[i], test_name_len);
				copy_bytes(test_dir + 3 + test_name_len, ".fam", 4);
				test_dir[4 + 3 + test_name_len] = 0;
				rmrf(test_dir);
				mkdir(test_dir, 0700);
				char gfile[test_name_len + 100];
				copy_bytes(gfile, test_dir, test_name_len + 4 + 3);
				copy_bytes(gfile + test_name_len + 4 + 3, "/.fam.dat", 9);
				gfile[test_name_len + 4 + 3 + 9] = 0;
				init_sys(gfile);
				test_arr[i](test_dir);
				shutdown_sys();
				if (_alloc_sum != start_alloc)
					println("%sFAIL%s: alloc_diff=%lli (Memory leak?)",
							BRIGHT_RED, RESET, _alloc_sum - start_alloc);
				fam_assert_eq(_alloc_sum, start_alloc);
				// rmrf(test_dir);
			}
		} else {
			println("%sFAIL:%s test '%s%s%s' failed!", BRIGHT_RED, RESET, GREEN,
					test_names[i], RESET);
			fail_count++;
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	double time_ns =
		(end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

	if (fail_count)
		println(
			"--------------------------------"
			"--------------------------------"
			"--------------------------------"
			"--------------------");
	println(
		"[%s====%s] Tested: %s%i%s | Passing: %s%i%s | Failing: %s%i%s "
		"(Execution time: %s%f%ss)",
		BLUE, RESET, YELLOW, test_exe_count, RESET, GREEN,
		test_exe_count - fail_count, RESET, CYAN, fail_count, RESET, CYAN,
		time_ns / 1e9, RESET);

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
			unsigned long long address = strtoul(addr, NULL, 16);
			address -= 8;

			char command[256];
			snprintf(command, sizeof(command),
					 "addr2line -f -e ./.bin/test %llx", address);

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
					 "atos -fullPath -o ./.bin/test -l 0x100000000 %s",
					 address);

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
