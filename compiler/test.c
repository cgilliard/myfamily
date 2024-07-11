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
	cr_assert_eq(pkg.classes[0].function_count, 7);
	cr_assert(!strcmp(pkg.classes[0].functions[0].name, "build"));
	cr_assert(!strcmp(pkg.classes[0].functions[0].return_type, "MyClass"));
	cr_assert(!strcmp(pkg.classes[0].functions[1].name, "display"));
	cr_assert(!strcmp(pkg.classes[0].functions[1].return_type, "void"));
	cr_assert(!strcmp(pkg.classes[0].functions[2].name, "set_x"));
	cr_assert(!strcmp(pkg.classes[0].functions[2].return_type, "u64"));
	cr_assert(!strcmp(pkg.classes[0].functions[3].name, "set_y"));
	cr_assert(!strcmp(pkg.classes[0].functions[3].return_type, "i32"));
	cr_assert(!strcmp(pkg.classes[0].functions[4].name, "set_both"));
	cr_assert(!strcmp(pkg.classes[0].functions[4].return_type, "void"));
	cr_assert(!strcmp(pkg.classes[0].functions[5].name, "main"));
	cr_assert(!strcmp(pkg.classes[0].functions[5].return_type, "i32"));
	cr_assert(!strcmp(pkg.classes[0].functions[6].name, "x"));
	cr_assert(!strcmp(pkg.classes[0].functions[6].return_type, "i32"));
	cr_assert_eq(pkg.classes[0].field_count, 3);
	cr_assert(!strcmp(pkg.classes[0].fields[0].name, "x"));
	cr_assert(!strcmp(pkg.classes[0].fields[0].type, "i32"));
	cr_assert(!strcmp(pkg.classes[0].fields[1].name, "y"));
	cr_assert(!strcmp(pkg.classes[0].fields[1].type, "u64"));
	cr_assert(!strcmp(pkg.classes[0].fields[2].name, "z"));
	cr_assert(!strcmp(pkg.classes[0].fields[2].type, "OtherClass"));

	parser_pkg_cleanup(&pkg);
}
