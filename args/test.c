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
#include <errno.h>

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

MyTest(args, test_sub_command) {
	SubCommand sc;
	cr_assert(!sub_command_build(&sc, "sc1", "sc1 help", 1, 2, "<arg doc>"));
	ArgsParam p1;
	cr_assert(!args_param_build(&p1, "name", "name help here", "n", false, false, "myname"));
	sub_command_add_param(&sc, &p1);

	cr_assert_eq(sc.param_count, 1);
	cr_assert(!strcmp(sc.params[0].name, p1.name));
	cr_assert(!strcmp(sc.params[0].help, p1.help));
	cr_assert(!strcmp(sc.params[0].short_name, p1.short_name));
	cr_assert_eq(sc.params[0].takes_value, p1.takes_value);
	cr_assert_eq(sc.params[0].multiple, p1.multiple);
	cr_assert(!strcmp(sc.params[0].default_value, p1.default_value));

	ArgsParam p2;
	cr_assert(!args_param_build(&p2, "name2", "name2 help here", "p", true, true, NULL));
	sub_command_add_param(&sc, &p2);

	cr_assert_eq(sc.param_count, 2);
	cr_assert(!strcmp(sc.params[1].name, p2.name));
	cr_assert(!strcmp(sc.params[1].help, p2.help));
	cr_assert(!strcmp(sc.params[1].short_name, p2.short_name));
	cr_assert_eq(sc.params[1].takes_value, p2.takes_value);
	cr_assert_eq(sc.params[1].multiple, p2.multiple);
	cr_assert(!sc.params[1].default_value);
}

MyTest(args, test_arg_population) {
	SubCommand sc1;
	cr_assert(!sub_command_build(&sc1, "sc1", "sc1 help", 1, 2, "<arg doc>"));
	ArgsParam p1;
	cr_assert(!args_param_build(&p1, "name", "name help here", "n", false, false, "myname"));
	sub_command_add_param(&sc1, &p1);

	ArgsParam p2;
	cr_assert(!args_param_build(&p2, "name2", "name2 help here", "x", true, false, NULL));

	Args args1;
	args_build(&args1, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args1, &p2);
	args_add_sub_command(&args1, &sc1);

	cr_assert(!strcmp("prog", args1.prog));
	cr_assert(!strcmp("ver1.0", args1.version));
	cr_assert(!strcmp("me", args1.author));
	cr_assert_eq(args1.subs_count, 2);
	cr_assert_eq(args1.argc, 0);
	cr_assert_eq(args1.argv, NULL);
	cr_assert(!strcmp(args1.subs[0].params[0].name, "name2"));
	cr_assert(!strcmp(args1.subs[0].params[0].short_name, "x"));
	cr_assert_eq(args1.subs[0].params[0].takes_value, true);
	cr_assert_eq(args1.subs[0].params[0].multiple, false);
	cr_assert(!strcmp(args1.subs[0].params[0].help, "name2 help here"));
	cr_assert_eq(args1.subs[0].params[0].default_value, NULL);

	cr_assert(!strcmp(args1.subs[1].params[0].name, "name"));
	cr_assert(!strcmp(args1.subs[1].params[0].short_name, "n"));
	cr_assert_eq(args1.subs[1].params[0].takes_value, false);
	cr_assert_eq(args1.subs[1].params[0].multiple, false);
	cr_assert(!strcmp(args1.subs[1].params[0].help, "name help here"));
	cr_assert(!strcmp(args1.subs[1].params[0].default_value, "myname"));

	const char *argv1[1] = {"test1"};
	args_init(&args1, 1, argv1);
	printf("vers\n");
	args_print_version(&args1);
}

MyTest(args, test_args_value_of) {
	Args args1;
	SubCommand sc1;
	if (sub_command_build(&sc1, "sc1", "sc1 help", 1, 2, "<arg doc>"))
		exit_error("Could not build subcommand.");
	ArgsParam p1;
	if (args_param_build(&p1, "name", "name help here", "n", false, false, "myname"))
		exit_error("Could not build param");
	sub_command_add_param(&sc1, &p1);

	ArgsParam p2;
	args_param_build(&p2, "name2", "name2 help here", "x", true, false, NULL);

	args_build(&args1, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args1, &p2);
	args_add_sub_command(&args1, &sc1);

	const char *argv[5] = {"myprog", "sc1", "--name", "1", "22"};
	args_init(&args1, 5, argv);

	char buf[1024];
	int ret = args_value_of(&args1, "name", buf, 1024, 0);
	// This a 'takes_value == false or flag'. So, return is 0 because it's found.
	cr_assert_eq(ret, 0);
	ret = args_value_of(&args1, "name2", buf, 1024, 0);
	// This is a 'takes_value == false or flag' So return is -1 because it's not found.
	cr_assert_eq(ret, -1);
	ret = args_value_of(&args1, "name22", buf, 1024, 0);
	// This is not a valid flag. So it's an error -2.
	cr_assert_eq(ret, -2);

	// We have two arguments in addition to the subcommand (index = 0)
	ret = args_get_argument(&args1, 0, buf, 1024);
	// first argument is our subcommand name
	cr_assert_eq(ret, 3);
	cr_assert(!strcmp(buf, "sc1"));

	// next is the argument '1' as specified.
	ret = args_get_argument(&args1, 1, buf, 1024);
	cr_assert_eq(ret, 1);
	cr_assert(!strcmp(buf, "1"));

	// next is '22' as specified.
	ret = args_get_argument(&args1, 2, buf, 1024);
	cr_assert_eq(ret, 2);
	cr_assert(!strcmp(buf, "22"));

	// When we attempt to access an additional argument we get -1
	ret = args_get_argument(&args1, 3, buf, 1024);
	cr_assert_eq(ret, -1);

	// This is distinct from the error of -2 return when we pass in a NULL pointer
	// in place our args pointer. (einval also set to errno in this case)
	errno = 0;
	ret = args_get_argument(NULL, 0, buf, 1024);
	cr_assert_eq(ret, -2);
	cr_assert_eq(errno, EINVAL);

	const char *argv2[2] = {"myprog", "@./resources/test.txt"};
	Args args2;
	args_build(&args2, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args2, &p2);
	args_add_sub_command(&args2, &sc1);
	args_init(&args2, 2, argv2);

	ret = args_get_argument(&args2, 0, buf, 1024);
	cr_assert_eq(ret, 3);
	cr_assert(!strcmp(buf, "sc1"));

	// next is the argument '9' as specified in file.
	ret = args_get_argument(&args2, 1, buf, 1024);
	cr_assert_eq(ret, 1);
	cr_assert(!strcmp(buf, "9"));

	// next is '92' as specified in file.
	ret = args_get_argument(&args2, 2, buf, 1024);
	cr_assert_eq(ret, 2);
	cr_assert(!strcmp(buf, "92"));

	// When we attempt to access an additional argument we get -1
	ret = args_get_argument(&args2, 3, buf, 1024);
	cr_assert_eq(ret, -1);
}

MyTest(args, test_usage) {
	Args args1;
	SubCommand sc1;
	if (sub_command_build(&sc1, "sc1", "sc1 help", 1, 2, "<arg doc>"))
		exit_error("Could not build subcommand.");
	ArgsParam p1;
	if (args_param_build(&p1, "name", "name help here", "n", false, false, "myname"))
		exit_error("Could not build param");
	sub_command_add_param(&sc1, &p1);

	ArgsParam p2;
	args_param_build(&p2, "name2", "name2 help here", "x", true, false, NULL);

	args_build(&args1, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args1, &p2);
	args_add_sub_command(&args1, &sc1);
	args_usage(&args1, NULL);
}
