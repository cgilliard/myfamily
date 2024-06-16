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

#include <base/args.h>
#include <base/class.h>
#include <base/test.h>

FamSuite(base);

FamTest(base, test_args_params) {
	ArgsParam param = ArgsParam_build("test", NULL, NULL, false, true);
	assert(*(bool *)ArgsParam_get_multiple(&param));
	assert(!*(bool *)ArgsParam_get_takes_value(&param));
	ArgsParam param2 = ArgsParam_build("test2", "help", "s", true, false);
	assert(!*(bool *)ArgsParam_get_multiple(&param2));
	assert(*(bool *)ArgsParam_get_takes_value(&param2));

	assert(!strcmp(*ArgsParam_get_name(&param), "test"));
	assert_eq(*ArgsParam_get_help(&param), NULL);
	assert_eq(*ArgsParam_get_short_name(&param), NULL);

	assert(!strcmp(*ArgsParam_get_name(&param2), "test2"));
	assert(!strcmp(*ArgsParam_get_help(&param2), "help"));
	assert(!strcmp(*ArgsParam_get_short_name(&param2), "s"));

	// you can call this more than once
	ArgsParam_cleanup(&param);
	ArgsParam_cleanup(&param);

	// or not call it because when it goes out of scope the cleanup fn will
	// be called
	ArgsParam param3 = ArgsParam_build("test", NULL, NULL, false, true);
}

FamTest(base, test_args_params_copy) {
	ArgsParam param = ArgsParam_build("test2", "help", "s", true, false);
	ArgsParam param2;
	copy(&param2, &param);
	assert(!strcmp(*ArgsParam_get_name(&param2), "test2"));
	assert(!strcmp(*ArgsParam_get_help(&param2), "help"));
	assert(!strcmp(*ArgsParam_get_short_name(&param2), "s"));
	assert_eq(*ArgsParam_get_multiple(&param2), false);
	assert_eq(*ArgsParam_get_takes_value(&param2), true);
}

FamTest(base, test_args) {

	Args x1;
	x1._count = 10;
	assert_eq(*(u64 *)Args_get_count(&x1), 10);
	x1 = Args_build();
	assert_eq(*(u64 *)Args_get_count(&x1), 0);

	Args x2 = Args_build();
	Args_add_param(&x2, "threads", "the number of threads to start", "t",
		       true, false);

	Args_add_param(&x2, "timeout", "time in milliseconds before halting",
		       "o", true, false);

	Args_add_param(&x2, "debug",
		       "if set to true, debugging info is printed", "d", false,
		       false);
	Args_add_param(&x2, "port", "port to listen to, multiple allowed", "p",
		       true, true);

	u64 count = *(u64 *)Args_get_count(&x2);
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(&x2);
	for (u64 i = 0; i < count; i++) {
		char *name = *((char **)ArgsParam_get_name(&params[i]));
		char *help = *((char **)ArgsParam_get_help(&params[i]));
		char *short_name =
		    *((char **)ArgsParam_get_short_name(&params[i]));
		bool takes_value =
		    *((bool *)ArgsParam_get_takes_value(&params[i]));
		bool multiple = *((bool *)ArgsParam_get_multiple(&params[i]));
		if (i == 0) {
			assert_eq_str(name, "threads");
			assert_eq_str(help, "the number of threads to start");
			assert_eq_str(short_name, "t");
			assert_eq(takes_value, true);
			assert_eq(multiple, false);
		}
		if (i == 1) {
			assert_eq_str(name, "timeout");
			assert_eq_str(help,
				      "time in milliseconds before halting");
			assert_eq_str(short_name, "o");
			assert_eq(takes_value, true);
			assert_eq(multiple, false);
		}
		if (i == 2) {
			assert_eq_str(name, "debug");
			assert_eq_str(
			    help, "if set to true, debugging info is printed");
			assert_eq_str(short_name, "d");
			assert_eq(takes_value, false);
			assert_eq(multiple, false);
		}

		if (i == 3) {
			assert_eq_str(name, "port");
			assert_eq_str(help,
				      "port to listen to, multiple allowed");
			assert_eq_str(short_name, "p");
			assert_eq(takes_value, true);
			assert_eq(multiple, true);
		}
	}
}

FamTest(base, test_params_equal) {
	ArgsParam p1 = ArgsParam_build("test", NULL, NULL, false, true);
	ArgsParam p2 = ArgsParam_build("test", NULL, NULL, false, true);
	assert(equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build("test", NULL, NULL, false, true);
	p2 = ArgsParam_build("test", "test", NULL, false, true);
	assert(!equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build("test", "ok", NULL, false, true);
	p2 = ArgsParam_build("test", "test", NULL, false, true);
	assert(!equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build("test", "test", "abc", false, true);
	p2 = ArgsParam_build("test", "test", NULL, false, true);
	assert(!equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build("test", "test", "ok", true, true);
	p2 = ArgsParam_build("test", "test", "ok", true, true);
	assert(equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build("test", "test", "ok", false, true);
	p2 = ArgsParam_build("test", "test", "ok", true, true);
	assert(!equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build("test", "test", "ok", true, true);
	p2 = ArgsParam_build("test", "test", "ok", true, false);
	assert(!equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build(NULL, "test", "ok", true, false);
	p2 = ArgsParam_build("test", "test", "ok", true, false);
	assert(!equal(&p1, &p2));

	cleanup(&p1);
	cleanup(&p2);
	p1 = ArgsParam_build(NULL, "test", "ok", true, false);
	p2 = ArgsParam_build(NULL, "test", "ok", true, false);
	assert(equal(&p1, &p2));
}

FamTest(base, test_init_args) {
	bool ret;
	Args args1 = Args_build();
	Args_add_param(&args1, "string4", "", "s", false, false);
	int argc1 = 1;
	const char *argv1[] = {"--string4"};
	ret = Args_init(&args1, argc1, (char **)argv1);
	assert(ret);

	Args args2 = Args_build();
	int argc2 = 4;

	Args_add_param(&args2, "str1", "", "s", true, false);
	Args_add_param(&args2, "verbose", "", "v", false, false);
	Args_add_param(&args2, "debug", "", "d", false, false);

	const char *argv2[] = {"--str1", "ok", "-d", "--verbose"};
	ret = Args_init(&args2, argc2, (char **)argv2);
	assert(ret);

	int argc3 = 4;
	Args args3 = Args_build();

	// str not found
	Args_add_param(&args3, "str1", "", "s", true, false);
	Args_add_param(&args3, "verbose", "", "v", false, false);
	Args_add_param(&args3, "debug", "", "d", false, false);

	const char *argv3[] = {"--str", "ok", "-d", "--verbose"};
	ret = Args_init(&args3, argc3, (char **)argv3);
	assert(!ret);

	// short and long matches ok
	char *argv4[] = {"--str1", "ok", "-d", "--verbose"};
	Args args4 = Args_build();
	Args_add_param(&args4, "str1", "", "s", true, false);
	Args_add_param(&args4, "verbose", "", "v", false, false);
	Args_add_param(&args4, "debug", "", "d", false, false);
	ret = Args_init(&args4, 4, argv4);
	assert(ret);

	// non multi error
	char *argv5[] = {"--str1", "ok", "x", "-d", "--verbose"};
	Args args5 = Args_build();
	Args_add_param(&args5, "str1", "", "s", true, false);
	Args_add_param(&args5, "verbose", "", "v", false, false);
	Args_add_param(&args5, "debug", "", "d", false, false);
	ret = Args_init(&args5, 5, argv5);
	assert(!ret);

	// multi
	char *argv6[] = {"--str1", "ok", "x", "-d", "--verbose"};
	Args args6 = Args_build();
	Args_add_param(&args6, "str1", "", "s", true, true);
	Args_add_param(&args6, "verbose", "", "v", false, false);
	Args_add_param(&args6, "debug", "", "d", false, false);
	ret = Args_init(&args6, 5, argv6);
	assert(ret);

	// takes param not found
	char *argv7[] = {"--str1", "ok", "x", "-d", "--verbose", "--port"};
	Args args7 = Args_build();
	Args_add_param(&args7, "str1", "", "s", true, true);
	Args_add_param(&args7, "verbose", "", "v", false, false);
	Args_add_param(&args7, "debug", "", "d", false, false);
	Args_add_param(&args7, "port", "", "p", true, false);
	ret = Args_init(&args7, 6, argv7);
	assert(!ret);

	// takes param not found because another option
	char *argv8[] = {"--str1", "-p", "x", "-d", "--verbose"};
	Args args8 = Args_build();
	Args_add_param(&args8, "str1", "", "s", true, true);
	Args_add_param(&args8, "verbose", "", "v", false, false);
	Args_add_param(&args8, "debug", "", "d", false, false);
	Args_add_param(&args8, "port", "", "p", true, false);
	ret = Args_init(&args8, 5, argv8);
	assert(!ret);

	// duplicate (debug)
	char *argv9[] = {"--str1", "p", "x", "-d", "--debug"};
	Args args9 = Args_build();
	Args_add_param(&args9, "str1", "", "s", true, true);
	Args_add_param(&args9, "verbose", "", "v", false, false);
	Args_add_param(&args9, "debug", "", "d", false, false);
	Args_add_param(&args9, "port", "", "p", true, false);
	ret = Args_init(&args9, 5, argv9);
	assert(!ret);
}
