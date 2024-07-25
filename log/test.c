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

#include <core/enum.h>
#include <core/format.h>
#include <core/result.h>
#include <core/string.h>
#include <core/test.h>
#include <core/tuple.h>
#include <core/unit.h>
#include <log/log.h>

#define LOG_LEVEL TRACE

static void __attribute__((constructor)) log_init_logger() {
	Result r = init_global_log();
}

MySuite(log);

MyTest(log, test_va_args) { return Ok(_()); }

ENUM(MyEnumLog, VARIANTS(TYPE1, TYPE2, TYPE3), TYPES("u32", "u64", "u64"))
IMPL(MyEnumLog, TRAIT_DISPLAY)
#define MyEnumLog DEFINE_ENUM(MyEnumLog)

Result MyEnumLog_fmt(MyEnumLog *ptr, Formatter *f) {
	MyEnumLogPtr vvx = *ptr;
	u32 value1_out = ENUM_VALUE(value1_out, u32, vvx);
	return FORMAT(f, "value={}", value1_out);
}

MyTest(log, test_unit_tostr) {
	Log log = LOG();

	u32 v = 12345;
	MyEnumLog mel = BUILD_ENUM(MyEnumLog, TYPE1, v);

	idebug(&log, "mel={}", mel);

	String s2 = STRING("test");
	idebug(&log, "uii={},s={},u2={}", _(), s2, _());

	u32 x = 1;
	u64 y = 2;
	i32 z = -2;
	Tuple t1 = TUPLE(x, y, z);

	return Ok(_());
}

MyTest(log, test_log_basic) {
	Result r = Log_build(0);
	Log log = TRY(r, log);
	String s1 = STRING("abc");
	String s2 = STRING("def");
	i8 vi8 = 44;
	i16 vi16 = 55;
	i32 vi32 = 222;
	i64 v = 333;
	u8 vu8 = 34;
	u16 vu16 = 35;
	u32 vu32 = 322;
	u64 vu64 = 111;
	u8 v1 = 1;
	u16 v2 = 2;
	u32 v3 = 3;
	u64 v4 = 4;
	i8 vv1 = 11;
	i16 vv2 = 12;
	i32 vv3 = 13;
	i64 vv4 = 14;
	f32 vvv1 = 15.1;
	f64 vvv2 = 16.1;
	u128 vvvv1 = 9999;
	i128 vvvv2 = -8888;
	u64 u641 = 123;
	bool b1 = true;
	bool b2 = false;
	idebug(&log, "v1={},v2={},v3={},v4={},vv1={},vv2={},vv3={},vv4={}", v1,
	       v2, v3, v4, vv1, vv2, vv3, vv4);
	idebug(&log, "v={},v2={}", vvv1, vvv2);
	idebug(&log, "u64={},u64={}", u641, u641);
	idebug(&log, "x={},x={}", v1, v2);

	idebug(&log, "b1={},b2={}", b1, b2);
	String s3 = STRING("ok");

	idebug(&log, "s1={},s2={},s3={}", s1, s2, s3);
	Unit u = BUILD(Unit);
	idebug(&log, "u={},u2={},u3={}", b1, b2, b1);
	idebug(&log, "vvvv1={},vvvv2={}", vvvv1, vvvv2);

	return Ok(_());
}

MyTest(log, test_log_log_level) {
	Log log = LOG(ShowLogLevel(true), ShowTimestamp(true));
	u32 x = 4;
	iinfo(&log, "test {}", x);
	idebug(&log, "test2");

	return Ok(_());
}

MyTest(log, testfmt) {
	int x = 0;
	Formatter f = FORMATTER(1000);
	Result rr = FORMAT(&f, "test1");
	Result rrr = Formatter_to_string(&f);
	String s = TRY(rrr, s);
	printf("s=%s\n", unwrap(&s));
	return Ok(_());
}

MyTest(log, testlogmacro) {

	Log log =
	    LOG(ShowLogLevel(true), ShowTimestamp(true), FormatterSize(1500),
		ShowLineNum(false), ShowMillis(true), ShowColors(true),
		LogFilePath("./abcdef.log"), FileHeader("myheader"),
		MaxAgeMillis(100000), LineNumMaxLen(20));
	u64 x = 1000;

	itrace(&log, "test {}", x);
	itrace(&log, "ok ok ok");

	trace("hi");
	trace("abc {}", x);

	idebug(&log, "test {}", x);
	idebug(&log, "ok ok ok");

	debug("hi");
	debug("abc {}", x);

	iinfo(&log, "test {}", x);
	iinfo(&log, "ok ok ok");

	info("hi");
	info("abc {}", x);

	iwarn(&log, "test {}", x);
	iwarn(&log, "ok ok ok");

	warn("hi");
	warn("abc {}", x);

	ierror(&log, "test {}", x);
	ierror(&log, "ok ok ok");

	error("hi");
	error("abc {}", x);

	ifatal(&log, "test {}", x);
	ifatal(&log, "ok ok ok");

	fatal("hi");
	fatal("abc {}", x);

	return Ok(_());
}
