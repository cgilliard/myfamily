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

#include <base/chain_allocator.h>
#include <base/colors.h>
#include <base/resources.h>
#include <criterion/criterion.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define MySuite(name)                                                                              \
	void setup_suite(void)                                                                         \
	{                                                                                              \
		printf("[%s====%s] Running %s%s%s test suite...\n", BLUE, RESET, GREEN, #name, RESET);     \
	}                                                                                              \
	Test(init, init, .init = setup_suite)                                                          \
	{                                                                                              \
	}                                                                                              \
	static int test_count = 0;                                                                     \
	static u64 initial_alloc_diff;                                                                 \
	static u64 initial_file_diff;                                                                  \
	static char *cur_name = "";                                                                    \
	static int log_fd = -1;                                                                        \
	void tear_down()                                                                               \
	{                                                                                              \
		u64 cur_alloc_count = mymalloc_sum();                                                      \
		u64 cur_free_count = myfree_sum();                                                         \
		u64 diff = cur_alloc_count - cur_free_count;                                               \
		u64 file_diff = myfopen_sum() - myfclose_sum();                                            \
		if (diff != initial_alloc_diff) {                                                          \
			printf("[%s====%s] %sError in tear_down of test%s "                                    \
				   "'%s%s%s'.\n[%s====%s] Number of allocations not "                              \
				   "equal to number "                                                              \
				   "of frees. Memory leak?\n",                                                     \
				BLUE, RESET, RED, RESET, GREEN, cur_name, RESET, BLUE, RESET);                     \
			printf("[%s====%s] "                                                                   \
				   "initial_alloc_diff=%" PRIu64 ",diff=%" PRIu64 "\n",                            \
				BLUE, RESET, initial_alloc_diff, diff);                                            \
			pid_t iPid = getpid();                                                                 \
			kill(iPid, SIGINT); /* trigger failure */                                              \
		} else if (file_diff != initial_file_diff) {                                               \
			printf("[%s====%s] %sError in tear_down of test%s "                                    \
				   "'%s%s%s'.\n[%s====%s] Number of file opens not "                               \
				   "equal to number "                                                              \
				   "of file closes. File handle leak?\n",                                          \
				BLUE, RESET, RED, RESET, GREEN, cur_name, RESET, BLUE, RESET);                     \
			printf("[%s====%s] "                                                                   \
				   "initial_file_diff=%" PRIu64 ",diff=%" PRIu64 "\n",                             \
				BLUE, RESET, initial_file_diff, file_diff);                                        \
			pid_t iPid = getpid();                                                                 \
			kill(iPid, SIGINT); /* trigger failure */                                              \
		}                                                                                          \
		if (log_fd > 0)                                                                            \
			close(log_fd);                                                                         \
	}

#define MyTest(suite, test)                                                                        \
	void setup_##test(void)                                                                        \
	{                                                                                              \
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
	void test_##suite##_##test();                                                                  \
	Test(suite, test, .init = setup_##test, .fini = tear_down)                                     \
	{                                                                                              \
		HeapAllocator ha;                                                                          \
		build_default_heap_allocator(&ha);                                                         \
		ChainGuard cg = SCOPED_ALLOCATOR(&ha, false);                                              \
		test_##suite##_##test();                                                                   \
		u64 cur_slabs = heap_allocator_cur_slabs_allocated(&ha);                                   \
		if (cur_slabs != 0)                                                                        \
			printf("some slabs were not deallocated! (%" PRIu64 ")\n", cur_slabs);                 \
		cr_assert_eq(cur_slabs, 0);                                                                \
		cr_assert_eq(heap_allocator_cleanup(&ha), 0);                                              \
	}                                                                                              \
	void test_##suite##_##test()
