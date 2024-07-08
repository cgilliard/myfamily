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
#include <core/test.h>

MySuite(args);

MyTest(args, test_args_param) {
	ArgsParam ap;
	args_param_build(&ap, "threads", "number of threads to execute", "t",
			 true, false, "1");

	cr_assert(!strcmp("threads", ap.name));
	cr_assert(!strcmp("number of threads to execute", ap.help));
	cr_assert(!strcmp("t", ap.short_name));
	cr_assert(!strcmp("1", ap.default_value));

	args_param_cleanup(&ap);

	args_param_build(&ap, "threads", "number of threads to execute", "t",
			 true, false, NULL);

	cr_assert_eq(ap.default_value, NULL);
	args_param_cleanup(&ap);
}

MyTest(args, test_sub_command) {
	SubCommand sc;
	ArgsParam ap1, ap2, ap3;

	args_param_build(&ap1, "threads", "number of threads to execute", "t",
			 true, false, "1");
	args_param_build(&ap2, "port", "tcp/ip port to bind to", "p", true,
			 true, "8080");
	args_param_build(&ap3, "debug", "print debug info", "d", false, false,
			 NULL);

	cr_assert(!sub_command_build(&sc, "node", "start the full node", 1, 2,
				     "arg doc"));

	sub_command_add_param(&sc, &ap1);
	sub_command_add_param(&sc, &ap2);
	sub_command_add_param(&sc, &ap3);

	cr_assert(!strcmp(sc.name, "node"));
	cr_assert(!strcmp(sc.help, "start the full node"));
	cr_assert_eq(sc.min_args, 1);
	cr_assert_eq(sc.max_args, 2);

	cr_assert(!strcmp(sc.params[0].name, "threads"));
	cr_assert(!strcmp(sc.params[1].name, "port"));
	cr_assert(!strcmp(sc.params[2].name, "debug"));
	cr_assert_eq(sc.params_count, 3);

	sub_command_cleanup(&sc);
	args_param_cleanup(&ap1);
	args_param_cleanup(&ap2);
	args_param_cleanup(&ap3);

	cr_assert(sub_command_build(&sc, NULL, "adkflaljdf", 1, 2, "arg doc"));
	cr_assert(sub_command_build(&sc, "ok", "adkflaljdf", 3, 2, "arg doc"));
}

MyTest(args, test_args) {
	Args args;
	args_build(&args, "myprog", "1.0", "myfamily devs", 0, 1, 0);
	char *argv[] = {"test", "test2"};
	args_init(&args, 2, argv);

	SubCommand sc;
	ArgsParam ap1, ap2, ap3;
	args_param_build(&ap1, "threads", "number of threads to execute", "t",
			 true, false, "1");

	args_param_build(&ap2, "port", "tcp/ip port to bind to", "p", true,
			 true, "8080");
	args_param_build(&ap3, "debug", "print debug info", "d", false, false,
			 NULL);

	sub_command_build(&sc, "node", "start the full node", 1, 2, "arg doc");
	sub_command_add_param(&sc, &ap1);

	args_add_sub_command(&args, &sc);
	args_add_param(&args, &ap2);
	args_add_param(&args, &ap3);

	cr_assert_eq(args.subs_count, 2);
	cr_assert(!strcmp(args.subs[1]->name, "node"));

	cr_assert(!strcmp(args.subs[0]->params[0].name, "port"));
	cr_assert(!strcmp(args.subs[0]->params[1].name, "debug"));
	cr_assert(!strcmp(args.subs[1]->params[0].name, "threads"));

	args_param_cleanup(&ap1);
	args_param_cleanup(&ap2);
	args_param_cleanup(&ap3);
	sub_command_cleanup(&sc);
	args_cleanup(&args);
}
