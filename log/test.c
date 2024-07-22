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

#include <core/result.h>
#include <core/string.h>
#include <core/test.h>
#include <core/unit.h>
#include <log/log.h>

#define LOG_LEVEL DEBUG

MySuite(log);

MyTest(log, test_unit_tostr) {
	Log log;

	Unit s = UNIT;
	debug(&log, "u={}", s);
	return Ok(UNIT);
}

MyTest(log, test_log_basic) {
	Log log;
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
	usize usize1 = 123;
	isize isize1 = 456;
	bool b1 = true;
	bool b2 = false;

	debug(&log,
	      "v1={},v2={},v3={},v4={},vv1={},vv2={},vv3={},vv4={},"
	      "",
	      v1, v2, v3, v4, vv1, vv2, vv3, vv4);

	debug(&log, "v={},v2={}", vvv1, vvv2);

	debug(&log, "usize={},isize={}", usize1, isize1);

	debug(&log, "b1={},b2={}", b1, b2);

	debug(&log, "s1={},s2={}", s1, s2);
	Unit u = UNIT;
	printf("unit.vdata=%i\n", u.vdata);
	debug(&log, "u={}", u);

	return Ok(UNIT);
}
