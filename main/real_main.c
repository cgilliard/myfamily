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

int real_main(int argc, char **argv) {
	Args args = ARGS("fam", "0.0.1-beta.1", "The MyFamily Developers");
	PARAM(&args, "threads", "t", "Number of threads to execute", true,
	      false);
	PARAM(&args, "port", "p", "TCP/IP Port to bind to (multiple allowed)",
	      true, true);
	PARAM(&args, "debug", "d", "Print debugging information", false, false);
	PARAM(&args, "address", "a",
	      "TCP/IP address to bind to. The default value is 127.0.0.1.",
	      true, false);
	PARAM(&args, "counter", "c", "The counter", true, false);
	ARGS_INIT(&args, argc, argv);

	Backtrace bt = GENERATE_BACKTRACE;
	print(&bt);
	return 0;
}
