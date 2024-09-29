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
#include <base/resources.h>
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

MyTest(args, test_help_and_version) {
	Args args3;
	args_build(&args3, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	const char *argv3[2] = {"myprog", "--help"};
	cr_assert_eq(args3.argc, 0);
	args_init(&args3, 2, argv3);
	cr_assert_eq(args3.argc, 2);

	Args args4;
	args_build(&args4, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	const char *argv4[2] = {"myprog", "-v"};
	cr_assert_eq(args4.argc, 0);
	args_init(&args4, 2, argv4);
	cr_assert_eq(args4.argc, 2);
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

MyTest(args, test_args_param_copy) {
	char big[ARGS_MAX_ARGUMENT_NAME_LENGTH + 10];
	for (int i = 0; i < ARGS_MAX_ARGUMENT_NAME_LENGTH + 10; i++) {
		big[i] = 'a';
	}
	big[ARGS_MAX_ARGUMENT_NAME_LENGTH + 9] = 0;

	ArgsParam p1;
	ArgsParamImpl p2;
	cr_assert(!args_param_build(&p1, "", "name help here", "n", false, false, "myname"));

	__is_debug_malloc = true;
	cr_assert(!args_param_copy(&p2, &p1));
	__is_debug_malloc = false;

	__is_debug_malloc_counter_ = 1;
	cr_assert(!args_param_copy(&p2, &p1));
	__is_debug_malloc_counter_ = UINT64_MAX;
}

MyTest(args, test_sub_command_other) {
	SubCommand sc1;
	SubCommandImpl sc2;
	if (sub_command_build(&sc1, "sc1", "sc1 help", 1, 2, "<arg doc>"))
		exit_error("Could not build subcommand.");
	__is_debug_malloc = true;
	cr_assert(!sub_command_copy(&sc2, &sc1));
	__is_debug_malloc = false;

	__is_debug_malloc_counter_ = 1;
	cr_assert(!sub_command_copy(&sc2, &sc1));
	__is_debug_malloc_counter_ = UINT64_MAX;

	ArgsParam p1;
	if (args_param_build(&p1, "name", "name help here", "n", false, false, "myname"))
		exit_error("Could not build param");
	sub_command_add_param(&sc1, &p1);

	__is_debug_malloc_counter_ = 2;
	cr_assert(!sub_command_copy(&sc2, &sc1));
	__is_debug_malloc_counter_ = UINT64_MAX;

	__is_debug_malloc_counter_ = 3;
	cr_assert(!sub_command_copy(&sc2, &sc1));
	__is_debug_malloc_counter_ = UINT64_MAX;

	SubCommandImpl sc0;
	sc0.help = NULL;
	strcpy(sc0.name, "test");
	sc0.min_args = 0;
	sc0.max_args = 0;
	sc0.arg_doc = NULL;
	sc0.params = NULL;
	sc0.is_specified = NULL;
	sc0.param_count = 0;
	cr_assert(!sub_command_copy(&sc2, &sc0));

	SubCommand sc3;
	if (sub_command_build(&sc3, "sc3", "sc3 help", 3, 7, NULL))
		exit_error("Could not build subcommand.");

	SubCommand sc4;
	cr_assert(sub_command_copy(&sc4, &sc3));

	cr_assert(sub_command_build(NULL, NULL, NULL, 0, 0, NULL));

	char big[ARGS_MAX_SUBCOMMAND_LENGTH + 100];
	for (int i = 0; i < ARGS_MAX_SUBCOMMAND_LENGTH + 10; i++) {
		big[i] = '0';
	}
	big[ARGS_MAX_SUBCOMMAND_LENGTH + 10] = 0;

	cr_assert(sub_command_build(&sc2, big, "help1", 3, 7, NULL));

	__is_debug_malloc = true;
	cr_assert(sub_command_build(&sc2, "abc", "def", 4, 5, NULL));
	__is_debug_malloc = false;

	__is_debug_malloc_counter_ = 1;
	cr_assert(sub_command_build(&sc2, "abc", "def", 4, 5, "test"));
	__is_debug_malloc_counter_ = UINT64_MAX;

	__is_debug_realloc = true;
	cr_assert(sub_command_add_param(&sc1, &p1));
	__is_debug_realloc = false;

	__is_debug_realloc_counter_ = 1;
	cr_assert(sub_command_add_param(&sc1, &p1));
	__is_debug_realloc_counter_ = UINT64_MAX;

	__is_debug_malloc_counter_ = 0;
	cr_assert(sub_command_add_param(&sc3, &p1));
	__is_debug_malloc_counter_ = UINT64_MAX;

	__is_debug_malloc_counter_ = 1;
	cr_assert(sub_command_add_param(&sc3, &p1));
	__is_debug_malloc_counter_ = UINT64_MAX;
}

MyTest(args, test_args_build_exit) {
	cr_assert(args_build(NULL, NULL, NULL, NULL, 0, 1, NULL));
	char big[ARGS_MAX_DETAIL_LENGTH + 100];
	for (int i = 0; i < ARGS_MAX_DETAIL_LENGTH + 10; i++) {
		big[i] = '0';
	}
	ArgsImpl args1;
	// name too long
	cr_assert(args_build(&args1, big, "ver1.0", "me", 2, 3, "Darwin arm64"));

	// min greater than max
	cr_assert(args_build(&args1, "ok", "ver1.0", "me", 3, 2, "Darwin arm64"));
	Args args2;
	cr_assert(!args_build(&args2, "name", "ver1.0", "me", 2, 3, NULL));

	__is_debug_misc_no_exit = true;
	args_exit_error(&args2, "test");
	__is_debug_misc_no_exit = false;

	SubCommand sc;
	cr_assert(!sub_command_build(&sc, "abc", "def", 4, 5, "test"));
	__is_debug_realloc = true;
	cr_assert(args_add_sub_command(&args2, &sc));
	__is_debug_realloc = false;

	ArgsImpl testadd_sub;
	testadd_sub.subs_count = 0;
	__is_debug_malloc = true;
	cr_assert(args_add_sub_command(&testadd_sub, &sc));
	__is_debug_malloc = false;

	Args args3;
	cr_assert(!args_build(&args3, "name", "ver1.0", "me", 2, 3, NULL));
	SubCommandImpl scerr;
	scerr.help = NULL;
	cr_assert(args_add_sub_command(&args3, &scerr));
}

MyTest(args, test_print_version) {
	Args args;
	cr_assert(!args_build(&args, "name", "ver1.0", "me", 2, 3, NULL));
	__is_debug_misc_no_exit = true;
	args_print_version(&args);
	__is_debug_misc_no_exit = false;
}

MyTest(args, test_args_init_err) {
	Args args1;
	cr_assert(!args_build(&args1, "name", "ver1.0", "me", 2, 3, NULL));
	const char *argv1[2] = {"prog", NULL};
	__is_debug_misc_no_exit = true;
	args_init(&args1, 2, argv1);
	__is_debug_misc_no_exit = false;
	// failed due to NULL so argc not set
	cr_assert_eq(args1.argc, 0);

	const char *argv2[3] = {"prog", "test", "@abc"};
	__is_debug_misc_no_exit = true;
	args_init(&args1, 3, argv2);
	__is_debug_misc_no_exit = false;
	// failed due to '@' so argc not set
	cr_assert_eq(args1.argc, 0);

	__is_debug_malloc = true;
	// with len 2 it should succeed, but malloc fails
	__is_debug_misc_no_exit = true;
	args_init(&args1, 2, argv2);
	__is_debug_misc_no_exit = false;
	__is_debug_malloc = false;
	cr_assert_eq(args1.argc, 0);

	__is_debug_malloc_counter_ = 1;
	__is_debug_misc_no_exit = true;
	args_init(&args1, 2, argv2);
	__is_debug_malloc_counter_ = UINT64_MAX;
	__is_debug_misc_no_exit = false;
	cr_assert_eq(args1.argc, 0);
}

MyTest(args, test_help_subs) {
	__is_debug_misc_no_exit = true;
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
	const char *argv1[5] = {"prog", "--name2", "ok", "sc1", "--help"};
	cr_assert_eq(args1.argc, 0);
	args_init(&args1, 5, argv1);
	cr_assert_eq(args1.argc, 5);

	Args args2;
	args_build(&args2, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args2, &p2);
	args_add_sub_command(&args2, &sc1);
	const char *argv2[5] = {"prog", "-x", "ok2", "sc1", "--help"};
	cr_assert_eq(args2.argc, 0);
	args_init(&args2, 5, argv2);
	cr_assert_eq(args2.argc, 5);

	Args args3;
	args_build(&args3, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	sub_command_add_param(&sc1, &p2);
	args_add_sub_command(&args3, &sc1);
	const char *argv3[5] = {"prog", "sc1", "-x", "ok", "--help"};
	cr_assert_eq(args3.argc, 0);
	args_init(&args3, 5, argv3);
	cr_assert_eq(args3.argc, 5);

	__is_debug_misc_no_exit = false;
}

MyTest(args, test_value_of) {
	__is_debug_misc_no_exit = true;

	ArgsParam p2;
	cr_assert(!args_param_build(&p2, "name2", "name2 help here", "x", true, true, "mydef"));

	SubCommand sc1;
	cr_assert(!sub_command_build(&sc1, "sc1", "sc1 help", 1, 2, "<arg doc>"));
	Args args1;
	args_build(&args1, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args1, &p2);
	args_add_sub_command(&args1, &sc1);

	const char *argv1[5] = {"prog", "--name2", "ok", "sc1", "--help"};
	cr_assert_eq(args1.argc, 0);
	args_init(&args1, 5, argv1);
	cr_assert_eq(args1.argc, 5);
	cr_assert_eq(args_value_of(NULL, NULL, NULL, 0, 0), -2);

	cr_assert_eq(args_value_of(&args1, "name2", NULL, 1, 0), -2);

	char buf[2];
	buf[0] = 0;
	buf[1] = 0;
	cr_assert_eq(args_value_of(&args1, "name2", buf, 2, 0), 2);
	cr_assert(!strcmp(buf, "o"));

	Args args2;
	args_build(&args2, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args2, &p2);
	sub_command_add_param(&sc1, &p2);
	args_add_sub_command(&args2, &sc1);

	const char *argv2[4] = {"prog", "sc1", "0", "--name2"};
	cr_assert_eq(args2.argc, 0);
	args_init(&args2, 4, argv2);
	cr_assert_eq(args2.argc, 4);

	cr_assert_eq(args_value_of(&args2, "name2", buf, 1, 0), 0);
	cr_assert_eq(args_value_of(&args2, "name2", NULL, 10, 0), -2);

	Args args3;
	args_build(&args3, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args3, &p2);
	args_add_sub_command(&args3, &sc1);

	const char *argv3[8] = {"prog", "--name2", "ok1", "--name2", "ok2", "sc1", "0", "1"};
	cr_assert_eq(args3.argc, 0);
	args_init(&args3, 8, argv3);
	cr_assert_eq(args3.argc, 8);

	char buf2[20];
	cr_assert_eq(args_value_of(&args3, "name2", buf2, 20, 0), 3);
	cr_assert(!strcmp(buf2, "ok1"));
	cr_assert_eq(args_value_of(&args3, "name2", buf2, 20, 1), 3);
	cr_assert(!strcmp(buf2, "ok2"));

	Args args4;
	args_build(&args4, "prog", "ver1.0", "me", 2, 3, "Darwin arm64");
	args_add_param(&args4, &p2);
	args_add_sub_command(&args4, &sc1);

	const char *argv4[3] = {"prog", "sc1", "0"};
	cr_assert_eq(args4.argc, 0);
	args_init(&args4, 3, argv4);
	cr_assert_eq(args4.argc, 3);

	cr_assert_eq(args_value_of(&args4, "name2", buf2, 20, 0), 5);
	cr_assert(!strcmp(buf2, "mydef"));

	// if max_len > 0, buf cannot be NULL
	cr_assert_eq(args_value_of(&args4, "name2", NULL, 1, 0), -2);

	__is_debug_misc_no_exit = false;
}

MyTest(args, test_arguments) {
	__is_debug_misc_no_exit = true;
	ArgsParam p1;
	cr_assert(!args_param_build(&p1, "threads", "number of threads", "t", true, false, "6"));
	ArgsParam p2;
	cr_assert(!args_param_build(&p2, "port", "tcp/ip port", "p", true, false, "9090"));
	ArgsParam p3;
	cr_assert(!args_param_build(&p3, "host", "tcp/ip host", "h", true, true, "127.0.0.1"));

	SubCommand sc1;
	cr_assert(!sub_command_build(&sc1, "start", "start the server", 2, 3, "<test> <test2>"));

	Args args1;
	cr_assert(!args_build(&args1, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));

	args_add_sub_command(&args1, &sc1);
	args_add_param(&args1, &p1);
	args_add_param(&args1, &p2);
	args_add_param(&args1, &p3);

	const char *argv[8] = {"prog", "--host", "0.0.0.0", "-p", "9999", "start", "0", "1"};
	cr_assert_eq(args1.argc, 0);
	args_init(&args1, 8, argv);
	cr_assert_eq(args1.argc, 8);

	char buf[1024];
	cr_assert_eq(args_get_argument(&args1, 0, buf, 1024), 5);
	cr_assert(!strcmp(buf, "start"));

	cr_assert_eq(args_get_argument(&args1, 1, buf, 1024), 1);
	cr_assert(!strcmp(buf, "0"));

	cr_assert_eq(args_get_argument(&args1, 2, buf, 1024), 1);
	cr_assert(!strcmp(buf, "1"));

	cr_assert_eq(args_get_argument(&args1, 3, buf, 1024), -1);

	cr_assert_eq(args_get_argument(&args1, 2, NULL, 1024), -2);

	__is_debug_misc_no_exit = false;
}

MyTest(args, test_other_situations) {
	__is_debug_misc_no_exit = true;
	ArgsParam p1;
	cr_assert(!args_param_build(&p1, "threads", "number of threads", "t", true, false, "6"));
	ArgsParam p2;
	cr_assert(!args_param_build(&p2, "port", "tcp/ip port", "p", true, false, "9090"));
	ArgsParam p3;
	cr_assert(!args_param_build(&p3, "host", "tcp/ip host", "h", true, true, "127.0.0.1"));
	ArgsParam p4;
	cr_assert(!args_param_build(&p4, "debug", "print debug info", "d", false, false, NULL));

	SubCommand sc1;
	cr_assert(!sub_command_build(&sc1, "start", "start the server", 2, 3, "<test> <test2>"));

	Args args1;
	cr_assert(!args_build(&args1, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));

	args_add_sub_command(&args1, &sc1);
	args_add_param(&args1, &p1);
	args_add_param(&args1, &p2);
	args_add_param(&args1, &p3);
	args_add_param(&args1, &p4);

	const char *argv[9] = {"prog", "-d", "--h", "0.0.0.0", "-p", "9999", "start", "0", "1"};
	cr_assert_eq(args1.argc, 0);
	args_init(&args1, 9, argv);
	cr_assert_eq(args1.argc, 9);

	char buf[1024];
	cr_assert_eq(args_value_of(&args1, "debug", buf, 1024, 0), 0);

	Args args2;
	cr_assert(!args_build(&args2, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args2, &p1);
	const char *argv2[8] = {"prog", "-t", "1", "-t", "2", "start", "9", "10"};
	args_init(&args2, 8, argv2);
	cr_assert_eq(args2.argc, 8);

	Args args3;
	cr_assert(!args_build(&args3, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args3, &p1);
	const char *argv3[8] = {"prog", "--threads", "1", "--threads", "2", "start", "9", "10"};
	args_init(&args3, 8, argv3);
	cr_assert_eq(args3.argc, 8);

	Args args4;
	cr_assert(!args_build(&args4, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args4, &p1);
	const char *argv4[2] = {"prog", "@./resources/test.txt"};
	__is_debug_malloc_counter_ = 1;
	args_init(&args4, 2, argv4);
	__is_debug_malloc_counter_ = UINT64_MAX;
	cr_assert_eq(args4.argc, 0);

	Args args5;
	cr_assert(!args_build(&args5, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args5, &p1);
	const char *argv5[2] = {"prog", "@./resources/test.txt"};
	__is_debug_malloc_counter_ = 2;
	args_init(&args5, 2, argv5);
	__is_debug_malloc_counter_ = UINT64_MAX;
	cr_assert_eq(args5.argc, 0);

	Args args6;
	cr_assert(!args_build(&args6, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args6, &p1);
	const char *argv6[2] = {"prog", "@./resources/test.txt"};
	__is_debug_malloc_counter_ = 3;
	args_init(&args6, 2, argv6);
	__is_debug_malloc_counter_ = UINT64_MAX;
	cr_assert_neq(args6.argc, 0);

	Args args7;
	cr_assert(!args_build(&args7, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args7, &p1);
	const char *argv7[2] = {"prog", "@./resources/test.txt"};
	__is_debug_realloc_counter_ = 0;
	args_init(&args7, 2, argv7);
	__is_debug_realloc_counter_ = UINT64_MAX;
	cr_assert_neq(args7.argc, 0);

	Args args8;
	cr_assert(!args_build(&args8, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args8, &p1);
	const char *argv8[2] = {"prog", "@./resources/blah.txt"};
	__is_debug_realloc_counter_ = 0;
	args_init(&args8, 2, argv8);
	__is_debug_realloc_counter_ = UINT64_MAX;
	cr_assert_eq(args8.argc, 0);
}

MyTest(args, test_usage2) {
	__is_debug_misc_no_exit = true;
	ArgsParam p1;
	cr_assert(!args_param_build(&p1, "threads________________________________________________",
								"number of threads", "t", true, false, "6"));
	ArgsParam p2;
	cr_assert(!args_param_build(&p2, "port________________________________________________",
								"tcp/ip port", "p", true, false, "9090"));
	ArgsParam p3;
	cr_assert(!args_param_build(&p3, "host________________________________________________",
								"tcp/ip host", "h", true, true, "127.0.0.1"));
	ArgsParam p4;
	cr_assert(!args_param_build(&p4, "debug", "print debug info", "d", false, false, NULL));
	ArgsParam p5;
	cr_assert(!args_param_build(&p5, "hint", "hint info", "q", false, false, NULL));

	SubCommand sc1;
	cr_assert(!sub_command_build(&sc1, "start", "start the server", 2, 3, "<test> <test2>"));

	Args args1;
	cr_assert(!args_build(&args1, "fastserver", "1.0", "me", 0, 0, "Darwin arm64"));
	args_add_param(&args1, &p5);

	args_add_sub_command(&args1, &sc1);
	args_add_param(&args1, &p1);
	args_add_param(&args1, &p2);
	args_add_param(&args1, &p3);
	args_add_param(&args1, &p4);

	args_usage(&args1, "blah");
	args_usage(&args1, "start");
	args_usage(&args1, NULL);

	__is_debug_misc_no_exit = false;
}
