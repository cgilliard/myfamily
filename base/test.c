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
#include <criterion/criterion.h>

void test_spawn() {
	printf("test spawn\n");
}

void test_spawn2() {
	printf("test spawn2\n");
}

void test_main() {
	printf("test_main\n");

	spawn(test_spawn);
	spawn(test_spawn2);

	printf("ret from spawn\n");
}

Test(base, test_base) {
	init(test_main);
}
