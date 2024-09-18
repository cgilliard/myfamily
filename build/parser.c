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

#include <base/misc.h>
#include <base/path.h>
#include <build/parser.h>
#include <lexer/lexer.h>
#include <limits.h>
#include <string.h>
#include <util/vec.h>

#define MAX_TYPE_NAME_LEN 128
#define MAX_VAR_NAME_LEN 128

typedef struct HeaderTypeInfo {
	char type[MAX_TYPE_NAME_LEN];
} HeaderTypeInfo;

typedef struct HeaderNameInfo {
	char name[MAX_VAR_NAME_LEN];
} HeaderNameInfo;

typedef struct HeaderType {
	Vec types;
	Vec names;
} HeaderType;

typedef struct HeaderFile {
	HeaderType ht;
} HeaderFile;

typedef enum ParserStateEnum {
	ParserStateBeginStatement = 0,
	ParserStateExpectModuleName = 1,
	ParserStateExpectBracket = 2,
	ParserStateExpectType = 3,
	ParserStateOther = 4,
} ParserStateEnum;

typedef struct ParserState {
	ParserStateEnum state;
	Vec *header_files;
	char *path_str;
	Vec *types;
	char type_name[MAX_TYPE_NAME_LEN + 1];
} ParserState;

#define MAX_NAME_LEN 1024

void proc_ParserStateBeginStatement(ParserState *state, Token *tk)
{
	if (tk->type == TokenTypeIdent) {
		if (!strcmp(tk->token, "mod")) {
			state->state = ParserStateExpectModuleName;
		} else {
			if (strlen(tk->token) > MAX_NAME_LEN) {
				token_display_error(
					tk, "token name is longer than MAX_NAME_LEN (%i)\n", MAX_NAME_LEN);
			} else {
				strcpy(state->type_name, tk->token);
			}
			state->state = ParserStateExpectBracket;
		}
	} else if (tk->type == TokenTypePunct && !strcmp(tk->token, "}"))
		state->state = ParserStateBeginStatement;
	else
		state->state = ParserStateOther;
}

void proc_ParserStateExpectModuleName(ParserState *state, Token *tk)
{
	if (tk->type != TokenTypeIdent) {
		token_display_error(tk, "Expected module name, found '%s'\n", tk->token);
	} else {
		if (strlen(tk->token) >= PATH_MAX) {
			token_display_error(tk, "Path is longer than max path (%i)\n", PATH_MAX);
		} else {
			HeaderInfo hi;
			Path npath;
			path_for(&npath, state->path_str);
			path_pop(&npath);
			path_push(&npath, tk->token);
			strcpy(hi.path, path_to_string(&npath));
			vec_push(state->header_files, &hi);
		}
		state->state = ParserStateOther;
	}
}

void parse_header(const Path *path, Vec *headers, Vec *types, Vec *module_info)
{
	char module_str[PATH_MAX + 1024];
	int vec_sz = vec_size(module_info);
	strcpy(module_str, "");
	for (int i = 0; i < vec_sz; i++) {
		strcat(module_str, vec_element_at(module_info, i));
		strcat(module_str, "::");
	}
	printf("Parsing header %s['%s'].\n", module_str, path_file_name(path));
	Lexer l;
	char *path_str = path_to_string(path);
	if (lexer_init(&l, path_str))
		exit_error("Could not open header file %s", path_str);

	Vec types_holder;
	vec_init(&types_holder, 10, sizeof(HeaderType));
	ParserState state = { ParserStateBeginStatement, headers, path_str, &types_holder };

	while (true) {
		Token tk;
		int res = lexer_next_token(&l, &tk);
		if (res == LexerStateComplete) {
			break;
		}
		// printf("state=%i,token_type=%i,token_value='%s'\n", state, tk.type, tk.token);
		if (tk.type == TokenTypeDoc) {
			// skip over doc comments for these purposes
			continue;
		} else if (state.state == ParserStateBeginStatement) {
			proc_ParserStateBeginStatement(&state, &tk);
		} else if (state.state == ParserStateExpectModuleName) {
			proc_ParserStateExpectModuleName(&state, &tk);
		} else if (tk.type == TokenTypePunct) {
			if (!strcmp(tk.token, "}") || !strcmp(tk.token, ";")) {
				state.state = ParserStateBeginStatement;
			} else if (!strcmp(tk.token, "{") && state.state == ParserStateExpectBracket) {
				TypeInfo ti;
				Path npath;
				path_for(&npath, path_str);
				path_pop(&npath);
				path_push(&npath, state.type_name);
				strncpy(ti.path, path_to_string(&npath), PATH_MAX);
				strcat(ti.path, ".c");
				vec_push(types, &ti);
				state.state = ParserStateOther;
			} else
				state.state = ParserStateOther;
		}
		token_cleanup(&tk);
	}

	lexer_cleanup(&l);
}
