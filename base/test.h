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

#include <base/colors.h>
#include <base/path.h>
#include <base/resources.h>
#include <base/string.h>
#include <criterion/criterion.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#define MySuite(name)                                                                              \
	void setup_suite(void) {                                                                       \
		printf("[%s====%s] Running %s%s%s test suite...\n", BLUE, RESET, GREEN, #name, RESET);     \
	}                                                                                              \
	Test(init, init, .init = setup_suite) {                                                        \
	}                                                                                              \
	static u64 test_count = 0;                                                                     \
	static u64 initial_alloc_diff;                                                                 \
	static u64 initial_file_diff;                                                                  \
	static char *cur_name = "";                                                                    \
	static u64 log_fd = -1;                                                                        \
	void tear_down() {                                                                             \
		u64 cur_alloc_count = mymalloc_sum();                                                      \
		u64 cur_free_count = myfree_sum();                                                         \
		u64 diff = cur_alloc_count - cur_free_count;                                               \
		u64 file_diff = myfopen_sum() - myfclose_sum();                                            \
		if (diff != initial_alloc_diff) {                                                          \
			printf("[%s====%s] %sError in tear_down of test%s "                                    \
				   "'%s%s%s'.\n[%s====%s] Number of allocations not "                              \
				   "equal to number "                                                              \
				   "of frees. Memory leak?\n",                                                     \
				   BLUE, RESET, RED, RESET, GREEN, cur_name, RESET, BLUE, RESET);                  \
			printf("[%s====%s] "                                                                   \
				   "initial_alloc_diff=%llu,diff=%llu\n",                                          \
				   BLUE, RESET, initial_alloc_diff, diff);                                         \
			pid_t iPid = getpid();                                                                 \
			kill(iPid, SIGINT); /* trigger failure */                                              \
		} else if (file_diff != initial_file_diff) {                                               \
			printf("[%s====%s] %sError in tear_down of test%s "                                    \
				   "'%s%s%s'.\n[%s====%s] Number of file opens not "                               \
				   "equal to number "                                                              \
				   "of file closes. File handle leak?\n",                                          \
				   BLUE, RESET, RED, RESET, GREEN, cur_name, RESET, BLUE, RESET);                  \
			printf("[%s====%s] "                                                                   \
				   "initial_file_diff=%llu,diff=%llu\n",                                           \
				   BLUE, RESET, initial_file_diff, file_diff);                                     \
			pid_t iPid = getpid();                                                                 \
			kill(iPid, SIGINT); /* trigger failure */                                              \
		}                                                                                          \
		if (log_fd > 0)                                                                            \
			close(log_fd);                                                                         \
	}

#define MyTest(suite, test)                                                                        \
	void setup_##test(void) {                                                                      \
		u64 cur_alloc_count = mymalloc_sum();                                                      \
		u64 cur_free_count = myfree_sum();                                                         \
		u64 diff = cur_alloc_count - cur_free_count;                                               \
		initial_alloc_diff = diff;                                                                 \
		initial_file_diff = myfopen_sum() - myfclose_sum();                                        \
		u64 initial_file_diff = myfopen_sum() - myfclose_sum();                                    \
		cur_name = #test;                                                                          \
		if (access("bin/nocapture", F_OK) != 0) {                                                  \
			char s[100];                                                                           \
			snprintf(s, 100, "bin/output_%s.log", #test);                                          \
			log_fd = open(s, O_WRONLY | O_CREAT | O_TRUNC, 0644);                                  \
			dup2(log_fd, STDOUT_FILENO);                                                           \
			dup2(log_fd, STDERR_FILENO);                                                           \
		} else {                                                                                   \
			log_fd = -1;                                                                           \
		}                                                                                          \
	}                                                                                              \
	void test_##suite##_##test(const Path *test_dir, const Path *resources_dir);                   \
	Test(suite, test, .init = setup_##test, .fini = tear_down) {                                   \
		{                                                                                          \
			char path[1024];                                                                       \
			strcpy(path, ".");                                                                     \
			strcat(path, #suite);                                                                  \
			strcat(path, "_");                                                                     \
			strcat(path, #test);                                                                   \
			strcat(path, ".fam");                                                                  \
			Path dir;                                                                              \
			path_for(&dir, path);                                                                  \
			remove_directory(&dir, false);                                                         \
			path_mkdir(&dir, 0700, false);                                                         \
			Path res;                                                                              \
			path_for(&res, ".");                                                                   \
			path_push(&res, "resources");                                                          \
			test_##suite##_##test(&dir, &res);                                                     \
			remove_directory(&dir, false);                                                         \
		}                                                                                          \
	}                                                                                              \
	void test_##suite##_##test(const Path *test_dir, const Path *resources_dir)
