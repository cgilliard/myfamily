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

#define MAX_TESTS 100
#define MAX_TEST_NAME 1024

#include <base/lib.h>

extern int test_count;
extern int fail_count;
typedef void (*test_fn_ptr)(const unsigned char *);
extern test_fn_ptr test_arr[MAX_TESTS + 1];
extern unsigned char test_names[MAX_TESTS][MAX_TEST_NAME + 1];
int printf(const char *fmt, ...);
int execute_tests(unsigned char *suite_name);
void fail_assert();

#define assert(v) \
	if (!(v)) fail_assert();

#define assert_eq(v1, v2)                                                \
	if ((v1) != (v2)) {                                                  \
		if (fail_count == 0)                                             \
			printf(                                                      \
				"--------------------------------"                       \
				"--------------------------------"                       \
				"--------------------------------"                       \
				"--------------------\n");                               \
		_Generic((v1),                                                   \
			float: printf("(%f != %f) ", ((float)(v1)), ((float)(v2))),  \
			double: printf("(%f != %f) ", ((float)(v1)), ((float)(v2))), \
			unsigned char: printf("(%c != %c) ", (unsigned char)(v1),    \
								  (unsigned char)(v2)),                  \
			unsigned long long: printf("(%llu != %llu) ",                \
									   (unsigned long long)(v1),         \
									   (unsigned long long)(v2)),        \
			long long: printf("(%lli != %lli) ", (long long)(v1),        \
							  (long long)(v2)),                          \
			int: printf("(%i != %i) ", (int)(v1), (int)(v2)),            \
			default: ({}));                                              \
		fail_assert();                                                   \
	}

#define Suite(name)                         \
	int main() {                            \
		int success = execute_tests(#name); \
		if (!success) {                     \
			return -1;                      \
		}                                   \
		return 0;                           \
	}

#define Test(name)                                                  \
	void _tfwork_##name(const unsigned char *test_dir);             \
	static void __attribute__((constructor)) __test_init_##name() { \
		if (test_count > MAX_TESTS)                                 \
			panic("Too many tests (MAX=%i)", MAX_TESTS);            \
		int name_len = cstring_len(#name);                          \
		if (name_len > MAX_TEST_NAME)                               \
			panic("test name: '%s' too long!", #name);              \
		test_arr[test_count] = &_tfwork_##name;                     \
		copy_bytes(test_names[test_count], #name, name_len);        \
		test_names[test_count][name_len] = 0;                       \
		test_count++;                                               \
	}                                                               \
	void _tfwork_##name(const unsigned char *test_dir)
