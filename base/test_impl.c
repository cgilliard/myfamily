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
#include <base/print_util.h>
#include <base/test_impl.h>
#include <criterion/criterion.h>
#include <fcntl.h>
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int test_log_fd = -1;

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag,
			  struct FTW *ftwbuf) {
	int rv = remove(fpath);

	if (rv) perror(fpath);

	return rv;
}

int rmrf(char *path) {
	return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void test_init_dev_resources() {
	// fam_alloc_init();
}
void test_confirm_dev_resources() {
	/*
		int64 final_alloc_sum = alloc_sum();
		int64 final_release_sum = release_sum();
		if (final_alloc_sum != final_release_sum) {
			println(
				"[%s----%s] dev resource check %lli (allocations) / %lli "
				"(deallocations). Memory leak?",
				BLUE, RESET, final_alloc_sum, final_release_sum);
			cr_assert_eq(final_alloc_sum, final_release_sum);
		}
	*/
}
void test_cleanup_dev_resources() {
	// before cleanup verify that fam_alloc does not have any remaining slabs.
	/*
		int64 slabs = fam_alloc_count_global_allocator();
		if (slabs != 0) {
			println(
				"[%s----%s] dev resource check: %lli slab(s) remain. "
				"Memory leak?",
				BLUE, RESET, slabs);
			cr_assert_eq(slabs, 0);
		}
		fam_alloc_cleanup();
	*/
}

void test_init_test_resources(byte *suite, byte *test, byte *path,
							  byte *resources) {
	if (access("bin/nocapture", F_OK) != 0) {
		char s[101];
		snprintf(s, 100, "bin/output_%s.log", test);
		test_log_fd = open(s, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		dup2(test_log_fd, STDOUT_FILENO);
		dup2(test_log_fd, STDERR_FILENO);
	} else {
		test_log_fd = -1;
	}
	strcpy(path, "./.");
	strcat(path, suite);
	strcat(path, "_");
	strcat(path, test);
	strcat(path, ".fam");
	strcpy(resources, ".");
	strcat(resources, "/resources");
	rmrf(path);
	mkdir(path, 0700);
}

void test_cleanup_test_resources(byte *path) {
	rmrf(path);
}
