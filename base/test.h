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

#include <base/lib.h>
#include <base/test_impl.h>
#include <criterion/criterion.h>

#define PATH_MAX 1024

#define Suite(name)                                                      \
	void setup_suite(void) {                                             \
		printf("[%s====%s] Running %s%s%s test suite...\n", BLUE, RESET, \
			   GREEN, #name, RESET);                                     \
	}                                                                    \
	Test(name, name##_init, .init = setup_suite) {                       \
	}

#define MyTest(suite, test_name)                                       \
	void setup_##test_name(void) {                                     \
	}                                                                  \
	void test_##suite##_##test_name(const byte *test_dir,              \
									const byte *resources_dir);        \
                                                                       \
	Test(suite, test_name, .init = setup_##test_name) {                \
		byte path[PATH_MAX];                                           \
		byte resources[PATH_MAX];                                      \
		test_init_dev_resources();                                     \
		test_init_test_resources(#suite, #test_name, path, resources); \
		test_##suite##_##test_name(path, resources);                   \
		test_cleanup_test_resources(path);                             \
		test_confirm_dev_resources();                                  \
		test_cleanup_dev_resources();                                  \
	}                                                                  \
	void test_##suite##_##test_name(const byte *test_dir,              \
									const byte *resources_dir)
