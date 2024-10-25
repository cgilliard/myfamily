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
