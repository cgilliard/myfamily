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
#include <base/backtrace.h>
#include <base/class.h>
#include <base/error.h>
#include <base/result.h>
#include <base/test.h>
#include <base/unit.h>

GETTER_PROTO(ArgsParam, name)
GETTER_PROTO(ArgsParam, help)
GETTER_PROTO(ArgsParam, short_name)
GETTER_PROTO(ArgsParam, takes_value)
GETTER_PROTO(ArgsParam, multiple)
GETTER_PROTO(ArgsParam, specified)
GETTER_PROTO(Args, count)
GETTER_PROTO(Args, params)

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
	x1 = Args_build("", "", "");
	assert_eq(*(u64 *)Args_get_count(&x1), 0);

	Args x2 = Args_build("", "", "");
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
	char buffer[1024];
	bool ret;
	Args args1 = Args_build("", "", "");
	Args_add_param(&args1, "string4", "", "s", false, false);
	int argc1 = 1;
	const char *argv1[] = {"test", "--string4"};
	ret = Args_init(&args1, argc1, (char **)argv1, DEBUG_INIT_NO_EXIT);
	assert(ret);

	Args args2 = Args_build("", "", "");
	int argc2 = 4;

	Args_add_param(&args2, "str1", "", "s", true, false);
	Args_add_param(&args2, "verbose", "", "v", false, false);
	Args_add_param(&args2, "debug", "", "d", false, false);

	const char *argv2[] = {"test", "--str1", "ok", "-d", "--verbose"};
	ret = Args_init(&args2, argc2, (char **)argv2, DEBUG_INIT_NO_EXIT);
	assert(ret);

	int argc3 = 4;
	Args args3 = Args_build("", "", "");

	// str not found
	Args_add_param(&args3, "str1", "", "s", true, false);
	Args_add_param(&args3, "verbose", "", "v", false, false);
	Args_add_param(&args3, "debug", "", "d", false, false);

	const char *argv3[] = {"test", "--str", "ok", "-d", "--verbose"};
	ret = Args_init(&args3, argc3, (char **)argv3, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	// short and long matches ok
	char *argv4[] = {"test", "--str1", "ok", "-d", "--verbose"};
	Args args4 = Args_build("", "", "");
	Args_add_param(&args4, "str1", "", "s", true, false);
	Args_add_param(&args4, "verbose", "", "v", false, false);
	Args_add_param(&args4, "debug", "", "d", false, false);
	ret = Args_init(&args4, 4, argv4, DEBUG_INIT_NO_EXIT);
	assert(ret);

	// non multi error
	char *argv5[] = {"test", "--str1", "ok", "x", "-d", "--verbose"};
	Args args5 = Args_build("", "", "");
	Args_add_param(&args5, "str1", "", "s", true, false);
	Args_add_param(&args5, "verbose", "", "v", false, false);
	Args_add_param(&args5, "debug", "", "d", false, false);
	ret = Args_init(&args5, 5, argv5, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	// multi
	char *argv6[] = {"test", "--str1", "ok", "x", "-d", "--verbose"};
	Args args6 = Args_build("", "", "");
	Args_add_param(&args6, "str1", "", "s", true, true);
	Args_add_param(&args6, "verbose", "", "v", false, false);
	Args_add_param(&args6, "debug", "", "d", false, false);
	ret = Args_init(&args6, 5, argv6, DEBUG_INIT_NO_EXIT);
	assert(ret);

	// takes param not found
	char *argv7[] = {"test", "--str1",    "ok",    "x",
			 "-d",	 "--verbose", "--port"};
	Args args7 = Args_build("", "", "");
	Args_add_param(&args7, "str1", "", "s", true, true);
	Args_add_param(&args7, "verbose", "", "v", false, false);
	Args_add_param(&args7, "debug", "", "d", false, false);
	Args_add_param(&args7, "port", "", "p", true, false);
	ret = Args_init(&args7, 7, argv7, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	// takes param not found because another option
	char *argv8[] = {"test", "--str1", "-p", "x", "-d", "--verbose"};
	Args args8 = Args_build("", "", "");
	Args_add_param(&args8, "str1", "", "s", true, true);
	Args_add_param(&args8, "verbose", "", "v", false, false);
	Args_add_param(&args8, "debug", "", "d", false, false);
	Args_add_param(&args8, "port", "", "p", true, false);
	ret = Args_init(&args8, 5, argv8, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	// duplicate (debug)
	char *argv9[] = {"test", "--str1", "p", "x", "-d", "--debug"};
	Args args9 = Args_build("", "", "");
	Args_add_param(&args9, "str1", "", "s", true, true);
	Args_add_param(&args9, "verbose", "", "v", false, false);
	Args_add_param(&args9, "debug", "", "d", false, false);
	Args_add_param(&args9, "port", "", "p", true, false);
	ret = Args_init(&args9, 6, argv9, DEBUG_INIT_NO_EXIT);
	assert(!ret);
}

FamTest(base, args_value) {
	char buffer[1024];
	bool ret;
	char *value;
	char *argv1[] = {"test", "--str1", "x", "-v", "--debug"};
	Args args1 = Args_build("", "", "");
	Args_add_param(&args1, "str1", "", "s", true, false);
	Args_add_param(&args1, "verbose", "", "v", false, false);
	Args_add_param(&args1, "debug", "", "d", false, false);
	Args_add_param(&args1, "port", "", "p", true, false);
	ret = Args_init(&args1, 5, argv1, DEBUG_INIT_NO_EXIT);
	assert(ret);

	strcpy(buffer, "");
	ret = Args_value(&args1, buffer, 1024, "str1", "y");
	assert(ret);
	assert_eq_str(buffer, "x");

	ret = Args_value(&args1, buffer, 1024, "debug", "");
	assert(ret);

	ret = Args_value(&args1, buffer, 1024, "verbose", "");
	assert(ret);

	strcpy(buffer, "");

	char *argv2[] = {"test", "--str1", "x", "y", "-v", "--debug"};
	Args args2 = Args_build("", "", "");
	Args_add_param(&args2, "str1", "", "s", true, true);
	Args_add_param(&args2, "verbose", "", "v", false, false);
	Args_add_param(&args2, "debug", "", "d", false, false);
	Args_add_param(&args2, "port", "", "p", true, false);
	Args_add_param(&args2, "other", "", "o", false, false);
	ret = Args_init(&args2, 6, argv2, DEBUG_INIT_NO_EXIT);
	assert(ret);

	ret = Args_value(&args2, buffer, 1024, "str1", "z");
	assert(ret);
	assert_eq_str(buffer, "x");

	ret = Args_value(&args2, buffer, 1024, "str1", "z");
	assert(ret);
	assert_eq_str(buffer, "y");

	ret = Args_value(&args2, buffer, 1024, "str1", "z");
	assert(!ret);
	assert_eq_str(buffer, "z");

	ret = Args_value(&args2, buffer, 1024, "port", "8080");
	assert(!ret);
	assert_eq_str(buffer, "8080");

	ret = Args_value(&args2, buffer, 1024, "debug", NULL);
	assert(ret);

	ret = Args_value(&args2, buffer, 1024, "other", NULL);
	assert(!ret);
}

FamTest(base, args_errors) {
	bool ret;
	char *value;
	char *argv1[] = {"test", "--str1", "x", "-v", "--debug"};
	Args args1 = Args_build("", "", "");
	Args_add_param(&args1, "str2", "", "s", true, false);
	Args_add_param(&args1, "verbose", "", "v", false, false);
	Args_add_param(&args1, "debug", "", "d", false, false);
	Args_add_param(&args1, "port", "", "p", true, false);
	ret = Args_init(&args1, 5, argv1, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	char *argv2[] = {"test", "--str2", "x", "-v", "--debug"};
	Args args2 = Args_build("", "", "");
	Args_add_param(&args2, "str2", "", "s", true, false);
	Args_add_param(&args2, "verbose", "", "v", false, false);
	Args_add_param(&args2, "debug", "", "d", true, false);
	Args_add_param(&args2, "port", "", "p", true, false);
	ret = Args_init(&args2, 5, argv2, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	char *argv3[] = {"test", "--str2", "x", "-v", "--debug", "-p"};
	Args args3 = Args_build("", "", "");
	Args_add_param(&args3, "str2", "", "s", true, false);
	Args_add_param(&args3, "verbose", "", "v", false, false);
	Args_add_param(&args3, "debug", "", "d", true, false);
	Args_add_param(&args3, "port", "", "p", true, false);
	ret = Args_init(&args3, 6, argv3, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	char *argv4[] = {"test", "--str1", "x", "-v", "--verbose"};
	Args args4 =
	    Args_build("testargs", "0.0.1-beta.1", "The MyFamily Developers");
	Args_add_param(&args4, "str1", "String to use for this program", "s",
		       true, true);
	Args_add_param(&args4, "verbose",
		       "Print additional debugging information", "v", false,
		       false);
	Args_add_param(&args4, "debug", "Print debugging details", "d", false,
		       false);
	Args_add_param(&args4, "port", "TCP/IP port to bind to", "p", true,
		       false);
	ret = Args_init(&args4, 5, argv4, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	Args_usage(&args4);
}

FamTest(base, args_double_cleanup) {
	char buffer[1024];
	bool ret;
	char *value;
	char *argv1[] = {"test", "--str1", "x", "-v", "--debug"};
	Args args1 = Args_build("", "", "");
	Args_add_param(&args1, "str2", "", "s", true, false);
	Args_add_param(&args1, "verbose", "", "v", false, false);
	Args_add_param(&args1, "debug", "", "d", false, false);
	Args_add_param(&args1, "port", "", "p", true, false);
	ret = Args_init(&args1, 5, argv1, DEBUG_INIT_NO_EXIT);
	assert(!ret);

	cleanup(&args1);
	cleanup(&args1);
}

FamTest(base, args_dups) {
	Args args = ARGS("fam", "0.0.1-beta.1", "The MyFamily Developers");
	assert(PARAM(&args, "threads", "t", "Number of threads to execute",
		     true, false));
	assert(PARAM(&args, "port", "p",
		     "TCP/IP Port to bind to (multiple allowed)", true, true));

	// Cannot test these because they exit now. Leaving them commented out
	// for now.

	// assert(!PARAM(&args, "port", "p", "TCP/IP Port to bind to (multiple
	// allowed)", true, true));
	// assert(!PARAM(&args, "aaaa", "p", "", true, true));
	// assert(!PARAM(&args, "aaaa", "h", "", true, true));
	// assert(!PARAM(&args, "aaaa", "V", "", true, true));
}

FamTest(base, backtrace) {
	Backtrace bt = EMPTY_BACKTRACE;
	Backtrace_generate(&bt, 100);

	u64 count = *Backtrace_get_count(&bt);
	char buffer[1024];
	for (u64 i = 0; i < count; i++) {
		bool ret = Backtrace_fn_name(&bt, buffer, 1024, i);
		BacktraceEntryPtr *rows = *Backtrace_get_rows(&bt);
		char *name = *BacktraceEntry_get_name(&rows[i]);
		char *bin_name = *BacktraceEntry_get_bin_name(&rows[i]);
		char *address = *BacktraceEntry_get_address(&rows[i]);
		char *file_path = *BacktraceEntry_get_file_path(&rows[i]);
		assert_eq_str(buffer, name);
		assert(ret);

		ret = Backtrace_bin_name(&bt, buffer, 1024, i);
		assert(ret);
		assert_eq_str(buffer, bin_name);

		ret = Backtrace_address(&bt, buffer, 1024, i);
		assert(ret);
		assert_eq_str(buffer, address);

		ret = Backtrace_file_path(&bt, buffer, 1024, i);
		assert(ret);
		assert_eq_str(buffer, file_path);
	}

	bool ret = Backtrace_fn_name(&bt, buffer, 1024, count);
	assert(!ret);

	ret = Backtrace_bin_name(&bt, buffer, 1024, count);
	assert(!ret);

	ret = Backtrace_address(&bt, buffer, 1024, count);
	assert(!ret);

	ret = Backtrace_file_path(&bt, buffer, 1024, count);
	assert(!ret);

	print(&bt);
}

ErrorKind ILLEGAL_STATE = EKIND("IllegalState");
ErrorKind ILLEGAL_ARGUMENT = EKIND("IllegalArgument");

FamTest(base, test_error) {
	Error err1 = ERROR(ILLEGAL_STATE, "test illegal state");
	Error err2 = ERROR(ILLEGAL_ARGUMENT, "arg must be greater than 100");
	Error err3 = ERROR(ILLEGAL_ARGUMENT, "arg must be greater than 10");

	// error 2 and 3 are equal because they have the same kind. Message or
	// Backtrace is not compared.
	assert(equal(&err2, &err3));
	// different kind
	assert(!equal(&err1, &err2));
	// different kind
	assert(!equal(&err1, &err3));

	// kind can also be compared
	assert(equal(KIND(err2), KIND(err3)));
	assert(!equal(KIND(err1), KIND(err2)));
	assert(!equal(KIND(err1), KIND(err3)));
}

CLASS(TestResult, FIELD(u64, x))
IMPL(TestResult, TRAIT_COPY);
#define TestResult DEFINE_CLASS(TestResult)
void TestResult_cleanup(TestResult *ptr) { printf("cleanup test result\n"); }
GETTER(TestResult, x)
#define GET_X(tr) *TestResult_get_x(tr)

size_t TestResult_size(TestResult *result) { return sizeof(Result); }
bool TestResult_copy(TestResult *dst, TestResult *src) {
	dst->_x = src->_x;
	return true;
}

FamTest(base, test_result) {
	Unit x1 = BUILD(Unit);
	Result r1 = Result_build_ok(&x1);
	assert(r1.is_ok());

	TestResult x2 = BUILD(TestResult, 10);
	Result r2 = Result_build_ok(&x2);
	assert(r2.is_ok());
	TestResultPtr *res2 = (TestResult *)unwrap(&r2);
	u64 value = GET_X(res2);
	assert_eq(value, 10);

	Error err3 = ERROR(ILLEGAL_STATE, "test result");
	Result r3 = Result_build_err(&err3);
	assert(!r3.is_ok());
	ErrorPtr *ret3 = unwrap_err(&r3);
	assert(equal(KIND(err3), &ILLEGAL_STATE));
	print(&err3);
}
