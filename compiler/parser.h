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

#ifndef _COMPILER_PARSER__
#define _COMPILER_PARSER__

#include <lexer/lexer.h>

typedef struct ParserType {

} ParserType;

typedef struct ParserExpression {

} ParserExpression;

typedef struct ParserStatement {

} ParserStatement;

typedef struct ParserFunction {
	char *name;
	char *return_type;
} ParserFunction;

typedef struct ParserField {
	char *name;
	char *type;
} ParserField;

typedef struct ParserImport {
	char *name;
} ParserImport;

typedef struct ParserImportList {
	ParserImport *list;
	int count;
} ParserImportList;

typedef struct ParserClass {
	char *name;
	ParserImportList imports;
	ParserFunction *functions;
	int function_count;
	ParserField *fields;
	int field_count;
} ParserClass;

typedef struct ParserPackage {
	ParserClass *classes;
	int class_count;
	struct ParserPackage *sub_packages;
	int sub_package_count;
} ParserPackage;

typedef struct SyntaxTree {
	ParserPackage root;
} SyntaxTree;

int parser_parse_file(ParserPackage *pkg, char *file);
int parser_pkg_init(ParserPackage *pkg);
void parser_pkg_cleanup(ParserPackage *pkg);

#endif // _COMPILER_PARSER__
