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
#include <base/string.h>
#include <base/unit.h>
#include <stdio.h>

Result build_args() {
	Args args = ARGS("prog", "v1.0", "MyFamily Developers");

	ArgsParam p1 = PARAM("threads", "number of threads", "t", true, false);
	Result res1 = Args_add_param(&args, &p1);
	Expect(res1);

	ArgsParam p2 = PARAM("port", "tcp/ip port to bind to", "p", true, true);
	Result res2 = Args_add_param(&args, &p2);
	Expect(res2);
	Result r1 = SubCommand_build("test1", 2, 5);
	Expect(r1);
	SubCommand sub1 = *(SubCommand *)unwrap(&r1);
	ArgsParam p3 = PARAM("other", "other option", "o", false, false);
	Result r2 = SubCommand_add_param(&sub1, &p3);
	Expect(r2);
	ArgsParam p4 = PARAM("another", "another option", "x", true, false);
	Result rr = SubCommand_add_param(&sub1, &p4);
	Expect(rr);

	ArgsParam p5 = PARAM("special", "special option", "y", true, true);
	Result rrr = SubCommand_add_param(&sub1, &p5);
	Expect(rrr);

	Result r3 = Args_add_sub(&args, &sub1);
	Expect(r3);

	return Ok(args);
}

Result real_main(int argc, char **argv) {
	Result res = build_args();
	Args args = *(Args *)Try(res);
	Result res2 = ARGS_INIT(&args, argc, argv);
	Try(res2);

	printf("arguments:\n");
	u64 i = 0;
	while (true) {
		Option o = Args_argument(&args, i);
		if (!o.is_some())
			break;
		StringPtr *p = unwrap(&o);
		char *p1 = unwrap(p);
		printf("      arg[%i]=%s\n", i, p1);
		i += 1;
	}

	printf("port:\n");
	while (true) {
		Result r = Args_value(&args, "port");
		Option o = *(Option *)unwrap(&r);
		if (!o.is_some())
			break;
		StringPtr *s = (String *)unwrap(&o);
		char *v = unwrap(s);
		printf("port=%s\n", v);
	}

	printf("special:\n");
	while (true) {
		Result r = Args_value(&args, "special");
		Option o = *(Option *)unwrap(&r);
		if (!o.is_some())
			break;
		StringPtr *s = (String *)unwrap(&o);
		char *v = unwrap(s);
		printf("special=%s\n", v);
	}

	return Ok(UNIT);
}
