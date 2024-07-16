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

typedef enum ParserStatementType {
	ParserStatementTypeNative = 0,
	ParserStatementTypeExpression = 1,
} ParserStatementType;

typedef struct ParserStatement {
	ParserStatementType type;
	void *data;
} ParserStatement;

typedef struct ParserExpression {

} ParserExpression;

typedef struct ParserArgumentList {
	ParserExpression *list;
	int count;
} ParserArgumentList;

typedef struct ParserStatementNativeData {
	char *fn_name;
	ParserArgumentList arg_list;
} ParserStatementNativeData;

typedef struct ParserStatementExpressionData {

} ParserStatementExpressionData;

typedef struct ParserStatementBlock {
	ParserStatement *stmts;
	int count;
} ParserStatementBlock;

typedef enum ParserVisibility {
	ParserVisibilityPub = 0,
	ParserVisibilityMod = 1,
	ParserVisibilityPriv = 2,
} ParserVisibility;

typedef enum ParserTypeType {
	ParserTypeStandard = 0,
	ParserTypeSizedArray = 1,
	ParserTypeDynamicArray = 2,
	ParserTypeTuple = 3,
	ParserTypeRef = 4,
	ParserTypeClosure = 5,
	ParserTypeUnit = 6,
} ParserTypeType;

typedef struct ParserType {
	char *name;
	ParserTypeType type;
	void *data;
} ParserType;

typedef struct ParserGeneric {
	char *name;
	ParserType **param_types;
	int count;
} ParserGeneric;

typedef struct ParserGenericList {
	ParserGeneric *list;
	int count;
} ParserGenericList;

typedef struct ParserTypeStandardData {
	ParserGenericList list;
} ParserTypeStandardData;

typedef struct ParserTypeSizedArrayData {
	ParserType type;
	int len;
} ParserTypeSizedArrayData;

typedef struct ParserTypeDynamicArrayData {
	ParserType type;
} ParserTypeDynamicArrayData;

typedef struct ParserTypeTupleData {
	ParserType *types;
	int count;
} ParserTypeTupleData;

typedef struct ParserTypeRefData {
	ParserType type;
} ParserTypeRefData;

typedef struct ParserTypeClosureData {
	ParserType *param_list;
	int count;
	ParserType return_type;
} ParserTypeClosureData;

typedef struct ParserParam {
	char *name;
	ParserType type;
} ParserParam;

typedef struct ParserParamList {
	ParserParam *list;
	int count;
} ParserParamList;

typedef struct ParserFunction {
	ParserVisibility visibility;
	bool is_static;
	char *name;
	ParserType return_type;
	ParserParamList param_list;
	ParserStatementBlock block;
} ParserFunction;

typedef struct ParserField {
	ParserVisibility visibility;
	bool is_static;
	char *name;
	ParserType type;
} ParserField;

typedef struct ParserImport {
	char *name;
	char *import_path;
} ParserImport;

typedef struct ParserImportList {
	ParserImport *list;
	int count;
} ParserImportList;

typedef struct ParserEnumVariant {
	char *name;
	ParserImportList imports;
	ParserFunction *functions;
	int function_count;
	ParserField *fields;
	int field_count;
} ParserEnumVariant;

typedef struct ParserEnum {
	ParserVisibility visibility;
	struct ParserEnum *complete_list;
	int complete_count;
	ParserEnumVariant *variants;
	int variant_count;
} ParserEnum;

typedef struct ParserModule {
	ParserVisibility visibility;
	ParserEnum *enums;
	int enum_count;
	struct ParserModule *sub_modules;
	int sub_module_count;
} ParserModule;

typedef struct ParserModuleList {
	char *name;
	ParserModule *list;
	int count;
} ParserModuleList;

typedef struct ParserPackage {
	char *name;
	char *version;
	char **authors;
	int author_count;
	char *description;
	char **keywords;
	int keyword_count;
	bool is_lib;
	ParserModule root;
	ParserModuleList dependencies;
	ParserModuleList test_dependencies;
} ParserPackage;

int parser_parse_project(ParserPackage *pkg, char *dir);
void parser_pkg_init(ParserPackage *pkg);
void parser_pkg_cleanup(ParserPackage *pkg);

#endif // _COMPILER_PARSER__
