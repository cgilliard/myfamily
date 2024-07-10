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

#include <compiler/parser.h>
#include <core/test.h>

MySuite(compiler);

MyTest(compiler, test_compiler_basic) {
	ParserPackage pkg;
	parser_pkg_init(&pkg);
	parser_parse_file(&pkg, "./resources/test.fam");

	cr_assert_eq(pkg.class_count, 1);
	cr_assert(!strcmp(pkg.classes[0].name, "MyClass"));
	cr_assert_eq(pkg.classes[0].imports.count, 2);
	cr_assert(!strcmp(pkg.classes[0].imports.list[0].name, "ghi"));
	cr_assert(!strcmp(pkg.classes[0].imports.list[1].name, "*"));

	parser_pkg_cleanup(&pkg);
}
