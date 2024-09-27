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

#include <args/args.h>
#include <base/test.h>

MySuite(args);

MyTest(args, test_args_params) {
	ArgsParam p1;
	cr_assert(!args_param_build(&p1, "name", "name help here", "n", false, false, "myname"));
	cr_assert(!strcmp(p1.name, "name"));
	cr_assert(!strcmp(p1.help, "name help here"));
	cr_assert(!strcmp(p1.short_name, "n"));
	cr_assert_eq(p1.takes_value, false);
	cr_assert_eq(p1.multiple, false);
	cr_assert(!strcmp(p1.default_value, "myname"));

	ArgsParamImpl p2;
	char name[ARGS_MAX_ARGUMENT_NAME_LENGTH + 2];
	for (int i = 0; i < ARGS_MAX_ARGUMENT_NAME_LENGTH + 1; i++)
		name[i] = '1';
	name[ARGS_MAX_ARGUMENT_NAME_LENGTH + 1] = 0;
	cr_assert(args_param_build(NULL, NULL, NULL, NULL, false, false, NULL));
	cr_assert(args_param_build(&p2, name, "name help here", "n", false, false, "myname"));

	__is_debug_malloc = true;
	cr_assert(args_param_build(&p2, "name", "name help here", "n", false, false, "myname"));
	__is_debug_malloc = false;

	ArgsParam p3;
	cr_assert(!args_param_build(&p3, "name", "name help here", "n", false, false, NULL));
	cr_assert(!strcmp(p3.name, "name"));
	cr_assert(!strcmp(p3.help, "name help here"));
	cr_assert(!strcmp(p3.short_name, "n"));
	cr_assert_eq(p3.takes_value, false);
	cr_assert_eq(p3.multiple, false);
	cr_assert_eq(p3.default_value, NULL);
}
