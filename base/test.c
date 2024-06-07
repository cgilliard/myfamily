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

#include <base/types.h>
#include <base/backtrace.h>
#include <criterion/criterion.h>
#include <stdio.h>

Test(base, ErrorKindTest) {
	ErrorKind ekind1 = { "ArrayIndexOutOfBounds" };
	ErrorKind ekind2 = { "IllegalState" };
	ErrorKind ekind3 = { "ArrayIndexOutOfBounds" };
	ErrorKind ekind4 = { "IllegalState" };

	cr_assert_eq(errorkind_equal(&ekind1, &ekind2), false);
	cr_assert_eq(errorkind_equal(&ekind1, &ekind3), true);
	cr_assert_eq(errorkind_equal(&ekind1, &ekind4), false);
	cr_assert_eq(errorkind_equal(&ekind2, &ekind4), true);
}

Test(base, ErrorTest) {
	Error err1 = {
		{ "ArrayIndexOutOfBounds" },
		"array index (10) was greater than array.len (8)"
	};

	Error err2 = {
                { "ArrayIndexOutOfBounds" },
                "array index (30) was greater than array.len (18)"
        };
	
	Error err3 = {
                { "IllegalState" },
                "invalid state"
        };

	cr_assert_eq(error_equal(&err1, &err2), true);
	cr_assert_eq(error_equal(&err1, &err3), false);

	char buf[MAX_ERROR_MESSAGE_LEN];
	error_to_string(buf, &err1);
	cr_assert_eq(
		strcmp(
			buf,
			"ArrayIndexOutOfBounds: array index (10) was greater than array.len (8)"
		),
		0
	);

}

Test(base, StringTest) {
	void *ptr = NULL;
	{
		String x;
	        STRING_INIT(x, "this is a test");
		String s;
		s.ptr = malloc(sizeof(char)* 10);
		ptr = &s;
		strcpy(s.ptr, "test");
		cr_assert_neq(((String*)ptr)->ptr, NULL);
	}

	cr_assert_eq(((String*)ptr)->ptr, NULL);

}

Test(base, backtrace) {
	backtrace();
}
