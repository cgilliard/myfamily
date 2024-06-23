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
#include <base/error.h>
#include <base/option.h>
#include <base/rc.h>
#include <base/string.h>
#include <base/test.h>
#include <base/unit.h>

FamSuite(base);

FamTest(base, test_tmalloc) {
	void *ptr = tlmalloc(1);
	tlfree(ptr);
}

int xyz(int x) { return x + 10; }
int abc(int x) { return x + 20; }

FamTest(base, test_vtable_sort) {
	Vtable table = {0, 0, NULL};
	VtableEntry ent0 = {"zzz", xyz};
	VtableEntry ent1 = {"myent1", xyz};
	VtableEntry ent2 = {"next", xyz};
	VtableEntry ent3 = {"prev", xyz};
	VtableEntry ent4 = {"aaa", abc};
	vtable_add_entry(&table, ent0);
	vtable_add_entry(&table, ent1);
	vtable_add_entry(&table, ent2);
	vtable_add_entry(&table, ent3);
	vtable_add_entry(&table, ent4);

	assert_eq_str(table.entries[0].name, "aaa");
	assert_eq_str(table.entries[4].name, "zzz");

	Object obj;
	obj.vdata.vtable = &table;
	assert(implements(&obj, "zzz"));
	assert(!implements(&obj, "yyy"));
	assert(implements(&obj, "aaa"));
	assert(implements(&obj, "next"));
	assert(implements(&obj, "myent1"));
	assert(implements(&obj, "prev"));

	int (*prev_fn)(int x) = find_fn(&obj, "prev");
	assert_eq(prev_fn(7), 17);
	int (*abc_fn)(int x) = find_fn(&obj, "aaa");
	assert_eq(abc_fn(8), 28);

	vtable_cleanup(&table);
}

FamTest(base, test_error) {
	ErrorKind ILLEGAL_STATE = EKIND("IllegalState");
	ErrorKind ILLEGAL_ARGUMENT = EKIND("IllegalArgument");
	Error err1 = ERROR(ILLEGAL_STATE, "illegal state message %i", 3);
	Error err2 = ERROR(ILLEGAL_ARGUMENT, "ill arg");
	Error err3 = ERROR(ILLEGAL_STATE, "another illegal state err");
	assert(equal(KIND(err1), KIND(err1)));
	assert(!equal(KIND(err1), KIND(err2)));
	assert(equal(KIND(err1), KIND(err3)));
	assert(!equal(&err1, &err2));
	assert(equal(&err1, &err3));
}

static int rc_test_cleanup_count = 0;
CLASS(RcTest, FIELD(u32, x))
#define RcTest DEFINE_CLASS(RcTest)

void RcTest_cleanup(RcTestPtr *ptr) { rc_test_cleanup_count += 1; }
RcTest *test_build_rctest(u32 x) {
	RcTestPtr *ret = tlmalloc(sizeof(RcTest));
	ret->vdata.vtable = &RcTestVtable;
	ret->_x = x;
}

FamTest(base, test_rc) {
	{
		RcTestPtr *x = test_build_rctest(10);
		Rc rc = RC(x);
		assert_eq(rc_test_cleanup_count, 0);

		{
			Rc rc2;
			clone(&rc2, &rc);
		}

		// cleanup did not occur here because while rc2's cleanup was
		// called, the counter was not equal to 0 so RcTest's cleanup
		// was not called.
		assert_eq(rc_test_cleanup_count, 0);
	}
	assert_eq(rc_test_cleanup_count, 1);
}

FamTest(base, test_string) {
	StringPtr *s1 = STRINGIMPLP("this is a test");
	Result r1 = Ok(*s1);
	StringPtr *s2 = unwrap(&r1);
	assert(equal(s1, s2));
	char *x = unwrap(s2);
	assert_eq_str(x, "this is a test");
}

FamTest(base, test_option) {
	StringPtr *s1 = STRINGIMPLP("test");
	Option x = Some(*s1);
	StringPtr *s2 = unwrap(&x);
	assert(equal(s1, s2));
}

FamTest(base, test_multi_lvl) {
	/*
	StringPtr *s1 = STRINGP("test2");
	Option x = Some(*s1);
	Result y = Ok(x);
	Option z = *(Option *)unwrap(&y);
	StringPtr *s2 = unwrap(&z);
	assert(equal(s1, s2));

	StringPtr *s3 = STRINGP("test3");
	Result x3 = Ok(*s3);
	Option y3 = Some(x3);
	Result x3_out = *(Result *)unwrap(&y3);
	StringPtr *s3_out = unwrap(&x3_out);
	assert(equal(s3_out, s3));

	StringPtr *s4 = STRINGP("test4");
	Rc rc = RC(s4);
	*/

	Result x5 = Ok(None);
	Option o5 = *(Option *)unwrap(&x5);
	assert(!o5.is_some());
}

GETTER_PROTO(ArgsParam, name)
GETTER_PROTO(ArgsParam, help)
GETTER_PROTO(ArgsParam, short_name)
GETTER_PROTO(ArgsParam, takes_value)
GETTER_PROTO(ArgsParam, multiple)

FamTest(base, test_args_param) {
	Result r1 = ArgsParam_build("p1name", "p1help", "p1short", true, true);
	ArgsParam args1 = *(ArgsParam *)unwrap(&r1);
	assert_eq_str(*ArgsParam_get_name(&args1), "p1name");
	assert_eq_str(*ArgsParam_get_help(&args1), "p1help");
	assert_eq_str(*ArgsParam_get_short_name(&args1), "p1short");
	assert_eq(*ArgsParam_get_takes_value(&args1), true);
	assert_eq(*ArgsParam_get_multiple(&args1), true);

	Result r2 =
	    ArgsParam_build("p2name", "p2help", "p2short", false, false);
	ArgsParam args2 = *(ArgsParam *)unwrap(&r2);
	assert_eq_str(*ArgsParam_get_name(&args2), "p2name");
	assert_eq_str(*ArgsParam_get_help(&args2), "p2help");
	assert_eq_str(*ArgsParam_get_short_name(&args2), "p2short");
	assert_eq(*ArgsParam_get_takes_value(&args2), false);
	assert_eq(*ArgsParam_get_multiple(&args2), false);

	Result r3 =
	    ArgsParam_build("p2name", "p2help", "p2short", false, false);
	ArgsParam args3 = *(ArgsParam *)unwrap(&r3);

	assert(!equal(&args1, &args2));
	assert(equal(&args1, &args1));
	assert(equal(&args2, &args3));
	assert(!equal(&args1, &args3));
}

GETTER_PROTO(SubCommand, name)
GETTER_PROTO(SubCommand, params)
GETTER_PROTO(SubCommand, params_state)
GETTER_PROTO(SubCommand, count)
GETTER_PROTO(SubCommand, min_add_args)
GETTER_PROTO(SubCommand, max_add_args)

FamTest(base, test_sub_command) {
	Result r1 = SubCommand_build("test1", 2, 5, "this is a help message");
	SubCommand s1 = *(SubCommand *)unwrap(&r1);
	assert_eq_str(*SubCommand_get_name(&s1), "test1");
	assert_eq(*SubCommand_get_min_add_args(&s1), 2);
	assert_eq(*SubCommand_get_max_add_args(&s1), 5);
	assert_eq(*SubCommand_get_count(&s1), 0);

	Result r11 =
	    ArgsParam_build("p1name", "p1help", "p1short", false, false);
	ArgsParam p1 = *(ArgsParam *)unwrap(&r11);

	Result r111 = SubCommand_add_param(&s1, &p1);
	assert(r111.is_ok());

	Result r12 =
	    ArgsParam_build("p2name", "p2help", "p2short", false, false);
	ArgsParam p2 = *(ArgsParam *)unwrap(&r12);

	Result r112 = SubCommand_add_param(&s1, &p2);
	assert(r112.is_ok());

	ArgsParamPtr *params_out = *SubCommand_get_params(&s1);
	ArgsParamStatePtr *params_state_out = *SubCommand_get_params_state(&s1);

	assert_eq(*SubCommand_get_count(&s1), 2);
	assert(equal(&params_out[0], &p1));
	assert(equal(&params_out[1], &p2));
}

FamTest(base, test_args) {
	Args a1 = ARGS("prog", "v1.0", "MyFamily Developers");
	ArgsParam p11 = PARAM("threads", "number of threads", "t", true, false);
	Result r11 = Args_add_param(&a1, &p11);
	Expect(r11);
	ArgsParam p12 =
	    PARAM("port", "tcp/ip port to bind to", "p", true, true);
	Result r12 = Args_add_param(&a1, &p12);
	Expect(r12);
	Result r13 = SubCommand_build("mysub", 0, 1, "help msg");
	assert(r13.is_ok());
	SubCommand sub11 = *(SubCommand *)unwrap(&r13);
	Result r14 = Args_add_sub(&a1, &sub11);
	assert(r14.is_ok());
	char *argv1[] = {"fam", "--threads", "1", "--port",
			 "2",	"--port",    "3", "mysub"};
	int argc1 = 8;
	Result r1 = ARGS_INIT(&a1, argc1, argv1);
	assert(r1.is_ok());

	Option o1 = Args_argument(&a1, 0);
	assert(o1.is_some());
	StringPtr *arg1 = unwrap(&o1);
	assert_eq_str(unwrap(arg1), "mysub");
	Option o2 = Args_argument(&a1, 1);
	assert(!o2.is_some());

	String scomp1 = String_build_expect("test84");
	String scomp2 = String_build_expect("test84");
	String scomp3 = String_build_expect("asldfjkl");
	assert(equal(&scomp1, &scomp2));
	assert(!equal(&scomp1, &scomp3));
}

FamTest(base, test_args_copy) {
	Args a1 = ARGS("prog", "v1.0", "MyFamily Developers");
}

FamTest(base, test_string2) {
	StringPtr *s1 = tlmalloc(sizeof(String));
	StringPtr *s2 = STRINGIMPLP("this is a test");
	StringPtr *s3 = tlmalloc(sizeof(String));
	Rc rc1 = RC(s1);
	Rc rc2 = RC(s2);
	Rc rc3 = RC(s3);

	clone(s1, s2);
	assert(equal(s1, s2));

	clone(s3, s1);
	assert(equal(s3, s2));

	Result r1 = append(s3, s1);
	Expect(r1);

	char *test = unwrap(s3);
	assert_eq_str(test, "this is a testthis is a test");

	assert(!equal(s1, s3));
	assert(!equal(s2, s3));
}

Result rc_string_fun() {
	StringRef ret = STRINGP("rc_string_fun");
	StringRef test;
	copy(&test, &ret);
	return Ok(ret);
}

Result rc_string_fun2() {
	Result r1 = rc_string_fun();
	StringRef s = *(StringRef *)unwrap(&r1);
	return Ok(s);
}

FamTest(base, test_rc_string) {
	StringRef rcs = STRINGP("test2");
	StringRef rcs2;
	copy(&rcs2, &rcs);
	assert_eq_str(unwrap(&rcs), "test2");
	assert_eq_str(unwrap(&rcs2), "test2");

	Result r1 = rc_string_fun2();
	assert(r1.is_ok());
	StringRef s1 = *(StringRef *)unwrap(&r1);
	assert_eq_str(unwrap(&s1), "rc_string_fun");

	Result r2 = STRING("abc");
	StringRef s2 = *(StringRef *)Expect(r2);
	assert_eq_str(unwrap(&s2), "abc");
}
