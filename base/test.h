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
#include <base/os.h>
#include <base/string.h>
#include <criterion/criterion.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#undef true
#define true (_Bool)1
#undef false
#define false (_Bool)0

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	int rv = remove(fpath);

	if (rv)
		perror(fpath);

	return rv;
}

int rmrf(char *path) {
	return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

#define MySuite(name)                                                                              \
	void setup_suite(void) {                                                                       \
		printf("[%s====%s] Running %s%s%s test suite...\n", BLUE, RESET, GREEN, #name, RESET);     \
	}                                                                                              \
	Test(init, init, .init = setup_suite) {                                                        \
	}                                                                                              \
	static number test_count = 0;                                                                  \
	static number initial_alloc_diff;                                                              \
	static number initial_file_diff;                                                               \
	static char *cur_name = "";                                                                    \
	static number log_fd = -1;                                                                     \
	void tear_down() {                                                                             \
		number cur_alloc_count = alloc_sum();                                                      \
		number cur_free_count = release_sum();                                                     \
		number diff = cur_alloc_count - cur_free_count;                                            \
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
		}                                                                                          \
		if (log_fd > 0)                                                                            \
			close(log_fd);                                                                         \
	}

#define MyTest(suite, test)                                                                        \
	void setup_##test(void) {                                                                      \
		number cur_alloc_count = alloc_sum();                                                      \
		number cur_free_count = release_sum();                                                     \
		number diff = cur_alloc_count - cur_free_count;                                            \
		initial_alloc_diff = diff;                                                                 \
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
	void test_##suite##_##test(const byte *test_dir, const byte *resources_dir);                   \
	Test(suite, test, .init = setup_##test, .fini = tear_down) {                                   \
		{                                                                                          \
			char path[PATH_MAX];                                                                   \
			strcpy(path, "./.");                                                                   \
			strcat(path, #suite);                                                                  \
			strcat(path, "_");                                                                     \
			strcat(path, #test);                                                                   \
			strcat(path, ".fam");                                                                  \
			char resources[PATH_MAX];                                                              \
			strcpy(resources, ".");                                                                \
			strcat(resources, "/resources");                                                       \
			rmrf(path);                                                                            \
			mkdir(path, 0700);                                                                     \
			test_##suite##_##test(path, resources);                                                \
			rmrf(path);                                                                            \
		}                                                                                          \
	}                                                                                              \
	void test_##suite##_##test(const byte *test_dir, const byte *resources_dir)
