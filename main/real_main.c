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
#include <base/unit.h>
#include <stdio.h>

Result build_args() {
	Result res = Args_build("prog", "v1.0", "MyFamily Developers");
	Expect(res);
	ArgsPtr args = *(Args *)Try(res);
	Result res2 = Args_add_param(&args, "threads", "number of threads", "t",
				     true, false);
	Expect(res2);
	Result res3 = Args_add_param(&args, "port", "tcp/ip port to bind to",
				     "p", true, true);
	Expect(res3);
	Result r1 = SubCommand_build("test1", 2, 5);
	SubCommand sub1 = *(SubCommand *)unwrap(&r1);
	Result r2 = Args_add_sub(&args, &sub1);

	return Ok(args);
}

Result real_main(int argc, char **argv) {
	Result res = build_args();
	Args args = *(Args *)Try(res);
	Result res2 = Args_init(&args, argc, argv, 0);
	return Ok(UNIT);
}
