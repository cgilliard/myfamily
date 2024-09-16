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
	ParserStateStart = 0,
	ParserStateExpectModuleName = 1,
} ParserState;

void parse_header(const Path *path, Vec *headers)
{
	printf("================Parsing header at path = '%s'\n", path_to_string(path));
	Lexer l;
	char *path_str = path_to_string(path);
	if (lexer_init(&l, path_str))
		exit_error("Could not open header file %s", path_str);

	int state = ParserStateStart;

	while (true) {
		Token tk;
		int res = lexer_next_token(&l, &tk);
		if (res == LexerStateComplete) {
			break;
		}
		printf("token_type=%i,token_value='%s'\n", tk.type, tk.token);
		if (state == ParserStateStart) {
			if (tk.type == TokenTypeIdent && !strcmp(tk.token, "mod")) {
				state = ParserStateExpectModuleName;
			}
		} else if (state == ParserStateExpectModuleName) {
			if (tk.type != TokenTypeIdent) {
				token_display_error(&tk, "Expected module name, found '%s'\n", tk.token);
			} else {
				if (strlen(tk.token) >= PATH_MAX) {
					token_display_error(&tk, "Path is longer than max path (%i)\n", PATH_MAX);
				} else {
					printf("found a module: %s\n", tk.token);
					HeaderInfo hi;
					Path npath;
					path_for(&npath, path_str);
					path_pop(&npath);
					path_push(&npath, tk.token);
					strncpy(hi.path, path_to_string(&npath), PATH_MAX);
					vec_push(headers, &hi);
				}
				state = ParserStateStart;
			}
		}
		token_cleanup(&tk);
	}

	lexer_cleanup(&l);
}
