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
#include <base/test.h>

MySuite(base);

MyTest(base, test_string) {
	CString s = String("test");
	cr_assert(!nil(s));
	cr_assert(!strcmp(cstring(&s), "test"));
	CString s2 = INIT_STRING;
	cr_assert(nil(s2));
	CString s3 = String(s);
	cr_assert(!strcmp(cstring(&s3), "test"));

	CString s4 = String();
	cr_assert(!nil(s4));
	cr_assert(!strcmp(cstring(&s4), ""));

	cr_assert(string_equal(&s, &s3));
	cr_assert(!string_equal(&s, &s2));
	cr_assert(!string_equal(&s, &s4));

	cr_assert_eq(string_len(&s), 4);
	cr_assert_eq(string_len(&s2), 0);
	cr_assert_eq(string_len(&s3), 4);
	cr_assert_eq(string_len(&s4), 0);
	string_append_s(&s, &s3);
	cr_assert_eq(string_len(&s), 8);

	CString s7 = String("testtest");
	cr_assert(string_equal(&s, &s7));

	CString s5 = String("abc");
	string_append_s(&s, &s5);
	cr_assert_eq(string_len(&s), 11);
	CString s6 = String("testtestabc");
	cr_assert(string_equal(&s, &s6));

	cr_assert_eq(string_index_of(&s, &s5), 8);
	cr_assert_eq(string_index_of(&s6, &s3), 0);
	cr_assert_eq(string_index_of(&s3, &s5), -1);

	cr_assert_eq(string_last_index_of(&s, &s5), 8);
	cr_assert_eq(string_last_index_of(&s6, &s3), 4);
	cr_assert_eq(string_last_index_of(&s3, &s5), -1);
}

Test(base, test_limits) {
	cr_assert_eq(NUM_MAX, 9223372036854775807LL);
	cr_assert_eq(INT_MAX, 2147483647);
	cr_assert_eq(CH_MAX, 255);
	cr_assert_eq(INT_MIN, -2147483648);

#pragma clang diagnostic ignored "-Wimplicitly-unsigned-literal"
	cr_assert_eq(NUM_MIN, (-9223372036854775808LL));
}
