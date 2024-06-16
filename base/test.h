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
#include <base/macro_utils.h>
#include <base/tlmalloc.h>
#include <criterion/criterion.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define assert(x) cr_assert(x)
#define assert_eq_str(x, y) cr_assert(!strcmp(x, y))
#define assert_eq(x, y) cr_assert_eq(x, y)
#define assert_neq(x, y) cr_assert_neq(x, y)

#define FamSuite(name)                                                         \
	void setup_suite(void) {                                               \
		void *test = tlmalloc(1);                                      \
		printf("[" ANSI_COLOR_BLUE "====" ANSI_COLOR_RESET             \
		       "] Running " ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET     \
		       " test suite...\n",                                     \
		       #name);                                                 \
	}                                                                      \
	Test(init, init, .init = setup_suite) {}                               \
	static u64 initial_alloc_diff;                                         \
	static char *cur_name = "";                                            \
	static int log_fd = -1;                                                \
	void tear_down(void) {                                                 \
		u64 cur_alloc_count = alloc_count();                           \
		u64 cur_free_count = free_count();                             \
		u64 diff = cur_alloc_count - cur_free_count;                   \
		if (diff != initial_alloc_diff) {                              \
			printf("[" ANSI_COLOR_BLUE "====" ANSI_COLOR_RESET     \
			       "] " ANSI_COLOR_RED                             \
			       "Error in tear_down of test" ANSI_COLOR_RESET   \
			       " '" ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET     \
			       "'.\n[" ANSI_COLOR_BLUE "====" ANSI_COLOR_RESET \
			       "] Number of allocations not equal to number "  \
			       "of frees. Memory leak?\n",                     \
			       cur_name);                                      \
			printf("[" ANSI_COLOR_BLUE "====" ANSI_COLOR_RESET     \
			       "] initial_alloc_diff=%llu,diff=%llu\n",        \
			       initial_alloc_diff, diff);                      \
			pid_t iPid = getpid();                                 \
			kill(iPid, SIGINT);                                    \
		}                                                              \
		if (log_fd > 0)                                                \
			close(log_fd);                                         \
	}

#define FamTest(suite, test)                                                   \
	void setup_##test(void) {                                              \
		u64 cur_alloc_count = alloc_count();                           \
		u64 cur_free_count = free_count();                             \
		u64 diff = cur_alloc_count - cur_free_count;                   \
		initial_alloc_diff = diff;                                     \
		cur_name = #test;                                              \
		if (access("bin/nocapture", F_OK) != 0) {                      \
			char s[100];                                           \
			sprintf(s, "bin/output_%s.log", #test);                \
			log_fd = open(s, O_WRONLY | O_CREAT | O_TRUNC, 0644);  \
			dup2(log_fd, STDOUT_FILENO);                           \
			dup2(log_fd, STDERR_FILENO);                           \
		} else {                                                       \
			log_fd = -1;                                           \
		}                                                              \
	}                                                                      \
	Test(base, test, .init = setup_##test, .fini = tear_down)
