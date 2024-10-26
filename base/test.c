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

#include <base/lib.h>
#include <base/test.h>

MySuite(base);

MyTest(base, test_init) {
	println("res={},test={}", resources_dir, test_dir);
	println("test {}", 1);
}

MyTest(base, test_backtrace) {
	Backtrace bt;
	backtrace_generate(&bt);
	backtrace_print(&bt);
}

MyTest(base, test_persist) {
}

MyTest(base, test_string) {
	cr_assert(mystrlen(NULL));
}

MyTest(base, test_print) {
	f64 x = 1.1;
	bool y = true;
	f32 z = 2.34;
	u8 *v1 = "okz";
	println("{} {} {} {} {} {} ", true, false, 1.32, z, 3, "zzz");
	println("ok2 {} {}", "abc", v1);

	u64 a = 1;
	i64 b = -1;
	u32 c = 2;
	i32 d = -2;
	u16 e = 3;
	i16 f = -3;
	u8 g = 4;
	i8 h = -4;
	bool i = false;
	bool j = true;
	f64 k = 123.456;
	f32 l = 567.888;
	char *m = "m1";
	char *n = "n1";
	u8 *o = "o1";
	u8 *p = "p1";
	i8 *q = "q1";
	const i8 *r = "r1";

	println("{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}", a, b, c, d, e, f, g, h, i, j,
			k, l, m, n, o, p, q, r);

	char buf[1024 + 1];
	sprint(buf, 1024, "{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}", a, b, c, d, e, f, g,
		   h, i, j, k, l, m, n, o, p, q, r);
	cr_assert(
		!mystrcmp(buf, "1 -1 2 -2 3 -3 4 -4 false true 123.456000 567.888000 m1 n1 o1 p1 q1 r1"));
}
