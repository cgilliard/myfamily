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

typedef enum ParserState {
	ParserStateBeginStatement = 0,
	ParserStateExpectModuleName = 1,
	ParserStateExpectBracket = 2,
	ParserStateOther = 3,
} ParserState;

#define MAX_NAME_LEN 1024

void parse_header(const Path *path, Vec *headers, Vec *types)
{
	printf("Parsing header ['%s'].\n", path_to_string(path));
	Lexer l;
	char *path_str = path_to_string(path);
	if (lexer_init(&l, path_str))
		exit_error("Could not open header file %s", path_str);

	int state = ParserStateBeginStatement;
	char type_name[MAX_NAME_LEN + 1];

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
		} else if (state == ParserStateBeginStatement) {
			if (tk.type == TokenTypeIdent) {
				if (!strcmp(tk.token, "mod")) {
					state = ParserStateExpectModuleName;
				} else {
					if (strlen(tk.token) > MAX_NAME_LEN) {
						token_display_error(
							&tk, "token name is longer than MAX_NAME_LEN (%i)\n", MAX_NAME_LEN);
					} else {
						strcpy(type_name, tk.token);
					}
					state = ParserStateExpectBracket;
				}
			} else if (tk.type == TokenTypePunct && !strcmp(tk.token, "}"))
				state = ParserStateBeginStatement;
			else
				state = ParserStateOther;
		} else if (state == ParserStateExpectModuleName) {
			if (tk.type != TokenTypeIdent) {
				token_display_error(&tk, "Expected module name, found '%s'\n", tk.token);
			} else {
				if (strlen(tk.token) >= PATH_MAX) {
					token_display_error(&tk, "Path is longer than max path (%i)\n", PATH_MAX);
				} else {
					HeaderInfo hi;
					Path npath;
					path_for(&npath, path_str);
					path_pop(&npath);
					path_push(&npath, tk.token);
					strncpy(hi.path, path_to_string(&npath), PATH_MAX);
					vec_push(headers, &hi);
				}
				state = ParserStateOther;
			}
		} else if (tk.type == TokenTypePunct) {
			if (!strcmp(tk.token, "}") || !strcmp(tk.token, ";")) {
				state = ParserStateBeginStatement;
			} else if (!strcmp(tk.token, "{") && state == ParserStateExpectBracket) {
				printf("Type: %s::['%s'].\n", path_str, type_name);
				TypeInfo ti;
				Path npath;
				path_for(&npath, path_str);
				path_pop(&npath);
				path_push(&npath, type_name);
				strncpy(ti.path, path_to_string(&npath), PATH_MAX);
				strcat(ti.path, ".c");
				vec_push(types, &ti);
				state = ParserStateOther;
			} else
				state = ParserStateOther;
		}
		token_cleanup(&tk);
	}

	lexer_cleanup(&l);
}
