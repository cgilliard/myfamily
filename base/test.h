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

#define MAX_TESTS 1024
#define MAX_TEST_NAME 128

int printf(const char *fmt, ...);

extern int test_count;
extern int fail_count;
typedef void (*test_fn_ptr)(const char *);
extern test_fn_ptr test_arr[MAX_TESTS + 1];
extern char test_names[MAX_TESTS][MAX_TEST_NAME + 1];
int execute_tests(char *suite_name);
#define BREAK    \
	"----------" \
	"----------" \
	"----------" \
	"----------" \
	"----------" \
	"----------" \
	"----------" \
	"------------"

void fail_assert();

#define Suite(name)                         \
	int main() {                            \
		int success = execute_tests(#name); \
		if (success) {                      \
			return -1;                      \
		}                                   \
		return 0;                           \
	}

#define Test(name)                                                  \
	void __test_##name(const char *test_file);                      \
	static void __attribute__((constructor)) __test_init_##name() { \
		if (test_count > MAX_TESTS) {                               \
			printf("Too many tests!\n");                            \
			_exit(-1);                                              \
		}                                                           \
		int name_len = cstring_len(#name);                          \
		if (name_len > MAX_TEST_NAME) {                             \
			printf("test name too long!\n");                        \
			_exit(-1);                                              \
		}                                                           \
		test_arr[test_count] = &__test_##name;                      \
		copy_bytes(test_names[test_count], #name, name_len);        \
		test_names[test_count][name_len] = 0;                       \
		test_count++;                                               \
	}                                                               \
	void __test_##name(const char *test_file)

#define assert(v) \
	if (!(v)) fail_assert();

#define assert_eq(v1, v2)                                                \
	if ((v1) != (v2)) {                                                  \
		if (fail_count == 0) printf("%s\n", BREAK);                      \
		_Generic((v1),                                                   \
			float: printf("(%f != %f) ", ((float)(v1)), ((float)(v2))),  \
			double: printf("(%f != %f) ", ((float)(v1)), ((float)(v2))), \
			byte: printf("(%c != %c) ", (byte)(v1), (byte)(v2)),         \
			u64: printf("(%llu != %llu) ", (u64)(v1), (u64)(v2)),        \
			i64: printf("(%lli != %lli) ", (i64)(v1), (i64)(v2)),        \
			int: printf("(%i != %i) ", (int)(v1), (int)(v2)),            \
			default: ({}));                                              \
		fail_assert();                                                   \
	}
