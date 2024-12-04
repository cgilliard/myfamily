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
#include <setjmp.h>

jmp_buf test_jmp;
int test_count = 0;
int fail_count = 0;
char test_names[MAX_TESTS][MAX_TEST_NAME + 1];
int test_itt;
char target_test[MAX_TEST_NAME + 1];
extern char **environ;
test_fn_ptr test_arr[MAX_TESTS + 1];

int execute_tests(char *suite_name) {
	__int128_t start, end;
	char success[test_count];
	start = getnanos();
	int test_exe_count = 0;

	for (int i = 0; i < test_count; i++) {
		test_itt = i;
		if (setjmp(test_jmp) == 0) {
			if (target_test[0] == 0 ||
				!cstring_compare(target_test, test_names[i])) {
				test_exe_count++;
				int test_name_len = cstring_len(test_names[i]);
				char test_dir[test_name_len + 100];
				copy_bytes(test_dir, "./.", 3);
				copy_bytes(test_dir + 3, test_names[i], test_name_len);
				copy_bytes(test_dir + 3 + test_name_len, ".fam", 4);
				test_dir[4 + 3 + test_name_len] = 0;
				test_arr[i](test_dir);
			}
		} else {
			println("{}FAIL:{} test '{}{}{}' failed!", BRIGHT_RED, RESET, GREEN,
					test_names[i], RESET);

			fail_count++;
		}
	}

	double time_ns = getnanos() - start;
	if (fail_count)
		println(
			"--------------------------------"
			"--------------------------------"
			"--------------------------------"
			"--------------------");
	println(
		"[{}===={}] Tested: {}{}{} | Passing: {}{}{} Failing: {}{}{} "
		"(Execution time: {}{}{}s)",
		BLUE, RESET, YELLOW, test_exe_count, RESET, GREEN,
		test_exe_count - fail_count, RESET, CYAN, fail_count, RESET, CYAN,
		time_ns / 1e9, RESET);

	println(
		"[{}================================"
		"================================================"
		"==================================={}]",
		BLUE, RESET);

	if (fail_count != 0)
		println("{}FAIL:{} Test suite {}{}{} failed!", BRIGHT_RED, RESET, GREEN,
				suite_name, RESET);

	return fail_count == 0;
}

void fail_assert() {
	const char *lt = last_trace();
	print("Assertion failure: {}", lt);
	if (lt) unmap(lt, 1);
	longjmp(test_jmp, 1);
}
