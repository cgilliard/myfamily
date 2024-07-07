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
#include <stdio.h>

void build_args(Args *args) {
	args_build(args, "fam", "0.0.1-alpha.1", "The MyFamily Developers", 0,
		   0, 0);

	SubCommand sc;
	ArgsParam ap1, ap2, ap3, ap4, ap5;
	args_param_build(&ap1, "threads", "Number of threads to execute", "t",
			 true, false, "1");

	args_param_build(&ap2, "port", "Tcp/Ip port to bind to", "p", true,
			 true, "8080");
	args_param_build(&ap3, "debug", "Print debug info", "d", false, false,
			 NULL);

	args_param_build(&ap4, "show_request", "Show request information", "s",
			 false, false, NULL);

	args_param_build(&ap5, "address", "Tcp/Ip address to bind to", "a",
			 true, false, "127.0.0.1");

	sub_command_build(&sc, "node", "Start the full node", 1, 2);
	sub_command_add_param(&sc, &ap1);
	sub_command_add_param(&sc, &ap4);

	args_add_sub_command(args, &sc);
	args_add_param(args, &ap2);
	args_add_param(args, &ap3);
	args_add_param(args, &ap5);
}

int real_main(int argc, char **argv) {
	Args args;
	build_args(&args);
	args_init(&args, argc, argv);

	char buf[100];
	int ret;
	ret = args_value_of(&args, "threads", buf, 100, 0);
	if (ret > 0) {
		printf("threads='%s'\n", buf);
	}

	u64 count = 0;
	while (true) {
		ret = args_value_of(&args, "port", buf, 100, count);
		if (ret > 0) {
			printf("port='%s'\n", buf);
			count += 1;
		} else {
			break;
		}
	}

	ret = args_value_of(&args, "debug", buf, 100, 0);
	if (ret > 0) {
		printf("debug set\n");
	}

	count = 0;
	while (true) {
		ret = args_get_argument(&args, count, buf, 100);

		if (ret > 0) {
			printf("arg[%i]=%s\n", count, buf);
			count += 1;
		} else
			break;
	}

	printf("main does not do anything\n");

	args_cleanup(&args);
	return 0;
}
