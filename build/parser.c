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
#include <base/resources.h>
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

typedef struct HeaderTypes {
	Vec types;
	Vec names;
} HeaderTypes;

typedef enum ParserStateEnum {
	ParserStateBeginStatement = 0,
	ParserStateExpectModuleName = 1,
	ParserStateExpectBracket = 2,
	ParserStateExpectType = 3,
	ParserStateExpectName = 4,
	ParserStateInTypeExpectSemi = 5,
	ParserStateOther = 6,
} ParserStateEnum;

typedef struct ParserState {
	ParserStateEnum state;
	Vec *header_files;
	char *path_str;
	HeaderTypes *ht;
	char *gen_header;
	u64 header_capacity;
	char *module_path;
	char type_name[MAX_TYPE_NAME_LEN + 1];
} ParserState;

#define INITIAL_HEADER_CAPACITY (1024 * 25)

void append_to_header(ParserState *state, const char *text, ...)
{
	u64 length;
	va_list args;

	va_start(args, text);
	length = vsnprintf(NULL, 0, text, args);
	va_end(args);

	if (state->header_capacity == 0) {
		u64 size = sizeof(char) * INITIAL_HEADER_CAPACITY;
		if (length >= size)
			size = length + 1;
		state->gen_header = mymalloc(size);
		if (state->gen_header == NULL)
			exit_error("Could not allocate sufficient memory to continue!");
		va_start(args, text);
		vsnprintf(state->gen_header, size, text, args);
		va_end(args);

	} else {
		u64 size = state->header_capacity;
		if (length + strlen(state->gen_header) + 1 >= state->header_capacity) {
			size = state->header_capacity + INITIAL_HEADER_CAPACITY;
			if (length + strlen(state->gen_header) + 1
				>= state->header_capacity + INITIAL_HEADER_CAPACITY) {
				size = length + strlen(state->gen_header) + 1;
			}
			state->gen_header = myrealloc(state->gen_header, sizeof(char) * (size));
			if (state->gen_header == NULL)
				exit_error("Could not allocate sufficient memory to continue!");
		}

		va_start(args, text);
		vsnprintf(state->gen_header + strlen(state->gen_header), size - strlen(state->gen_header),
			text, args);
		va_end(args);
	}

	state->header_capacity += INITIAL_HEADER_CAPACITY;
}

void free_gen_header(ParserState *state)
{
	if (state->gen_header)
		myfree(state->gen_header);
}

void proc_ParserStateBeginStatement(ParserState *state, Token *tk)
{
	if (tk->type == TokenTypeIdent) {
		if (!strcmp(tk->token, "mod")) {
			state->state = ParserStateExpectModuleName;
		} else {
			if (strlen(tk->token) > MAX_TYPE_NAME_LEN) {
				token_display_error(
					tk, "token name is longer than MAX_TYPE_NAME_LEN (%i)", MAX_TYPE_NAME_LEN);
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

void proc_ParserStateExpectType(ParserState *state, Token *tk)
{
	if (tk->type != TokenTypeIdent) {
		if (!strcmp(tk->token, "}")) {
			u64 count = vec_size(&state->ht->types);
			char module_path[strlen(state->module_path) + strlen(state->type_name) + 10];
			strcpy(module_path, "_type_");
			strncat(module_path, state->module_path, strlen(state->module_path) - 2);
			strcat(module_path, "_");
			strcat(module_path, state->type_name);
			printf("types count = %" PRIu64 ",path_str=%s\n", count, module_path);
			append_to_header(state, "typedef struct %s {\n", module_path);
			for (u64 i = 0; i < count; i++) {
				HeaderNameInfo *ni;
				HeaderTypeInfo *ti;
				ti = vec_element_at(&state->ht->types, i);
				ni = vec_element_at(&state->ht->names, i);
				printf("[%" PRIu64 "]=%s,%s\n", i, ti->type, ni->name);
				append_to_header(state, "%s %s;\n", ti->type, ni->name);
			}
			append_to_header(state, "} %s;\n\n", module_path);
			append_to_header(state, "#define %s %s\n", state->type_name, module_path);
			vec_clear(&state->ht->types);
			vec_clear(&state->ht->names);
			state->state = ParserStateBeginStatement;
		} else {
			token_display_error(tk, "Expected type name. Found [%s]", tk->token);
			state->state = ParserStateOther;
		}
	} else {
		if (strlen(tk->token) > MAX_TYPE_NAME_LEN) {
			token_display_error(tk, "Name [%s] is too long.", tk->token);
			state->state = ParserStateOther;
		} else {
			HeaderTypeInfo hti;
			strcpy(hti.type, tk->token);
			vec_push(&state->ht->types, &hti);
			state->state = ParserStateExpectName;
		}
	}
}

void proc_ParserStateExpectName(ParserState *state, Token *tk)
{
	if (tk->type != TokenTypeIdent) {
		token_display_error(tk, "Expected name. Found [%s]", tk->token);
		state->state = ParserStateOther;
	} else {
		if (strlen(tk->token) > MAX_VAR_NAME_LEN) {
			token_display_error(tk, "Name [%s] is too long.", tk->token);
			state->state = ParserStateOther;
		} else {
			HeaderNameInfo hti;
			strcpy(hti.name, tk->token);
			vec_push(&state->ht->names, &hti);
			state->state = ParserStateInTypeExpectSemi;
		}
	}
}

void proc_ParserStateInTypeExpectSemi(ParserState *state, Token *tk)
{
	if (strcmp(tk->token, ";")) {
		token_display_error(tk, "Expected ';'. Found [%s]", tk->token);
	}
	state->state = ParserStateExpectType;
}

void proc_ParserStateExpectModuleName(ParserState *state, Token *tk)
{
	if (tk->type != TokenTypeIdent) {
		token_display_error(tk, "Expected module name, found '%s'", tk->token);
	} else {
		if (strlen(tk->token) >= PATH_MAX) {
			token_display_error(tk, "Path is longer than max path (%i)", PATH_MAX);
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

void parse_header(
	const char *base_dir, const Path *path, Vec *headers, Vec *types, Vec *module_info)
{
	char module_str[PATH_MAX + 1024];
	int vec_sz = vec_size(module_info);
	strcpy(module_str, "");
	char module_path[PATH_MAX + 1024];
	strcpy(module_path, "");
	for (int i = 0; i < vec_sz; i++) {
		strcat(module_str, vec_element_at(module_info, i));
		strcat(module_str, "::");
		strcat(module_path, vec_element_at(module_info, i));
		strcat(module_path, "_");
	}
	strcat(module_path, path_file_name(path));
	printf("Parsing header %s['%s'].\n", module_str, path_file_name(path));
	Lexer l;
	char *path_str = path_to_string(path);
	if (lexer_init(&l, path_str))
		exit_error("Could not open header file %s", path_str);

	HeaderTypes ht;

	vec_init(&ht.names, 10, sizeof(HeaderNameInfo));
	vec_init(&ht.types, 10, sizeof(HeaderTypeInfo));
	ParserState state = { ParserStateBeginStatement, headers, path_str, &ht, NULL, 0, module_path };
	/*
	if (strlen(module_path) > 2)
		strncpy(state.module_path, module_path, strlen(module_path) - 2);
		*/

	while (true) {
		Token tk;
		int res = lexer_next_token(&l, &tk);
		if (res == LexerStateComplete) {
			break;
		}
		printf("state=%i,token_type=%i,token_value='%s'\n", state.state, tk.type, tk.token);
		if (tk.type == TokenTypeDoc) {
			// skip over doc comments for these purposes
			continue;
		} else if (state.state == ParserStateBeginStatement) {
			proc_ParserStateBeginStatement(&state, &tk);
		} else if (state.state == ParserStateExpectModuleName) {
			proc_ParserStateExpectModuleName(&state, &tk);
		} else if (state.state == ParserStateExpectType) {
			proc_ParserStateExpectType(&state, &tk);
		} else if (state.state == ParserStateExpectName) {
			proc_ParserStateExpectName(&state, &tk);
		} else if (state.state == ParserStateInTypeExpectSemi) {
			proc_ParserStateInTypeExpectSemi(&state, &tk);
		} else if (tk.type == TokenTypePunct) {
			if (!strcmp(tk.token, "}") || !strcmp(tk.token, ";")) {
				state.state = ParserStateBeginStatement;
			} else if (!strcmp(tk.token, "{") && state.state == ParserStateExpectBracket) {
				TypeInfo ti;
				Path npath;
				path_for(&npath, path_str);
				path_pop(&npath);
				path_push(&npath, state.type_name);
				strcpy(ti.path, path_to_string(&npath));
				strcat(ti.path, ".c");
				char *file_name = path_file_name(path);
				if (!strcmp(file_name, "mod.h"))
					strcpy(ti.module_file_name, "");
				else
					strncpy(ti.module_file_name, file_name, strlen(file_name) - 2);
				vec_push(types, &ti);
				state.state = ParserStateExpectType;
			} else
				state.state = ParserStateOther;
		}
		token_cleanup(&tk);
	}

	if (state.gen_header) {
		printf("Generated header: '%s'\n", state.gen_header);
	}

	Path header_include;
	path_for(&header_include, base_dir);
	path_push(&header_include, "target");
	path_push(&header_include, "include");
	path_push(&header_include, module_path);
	path_canonicalize(&header_include);
	printf("write to %s\n", path_to_string(&header_include));
	FILE *fp = myfopen(path_to_string(&header_include), "w");
	if (state.gen_header) {
		if (fprintf(fp, "%s", state.gen_header) < strlen(state.gen_header))
			fprintf(
				stderr, "Partial write of header: %s occurred!\n", path_to_string(&header_include));
	}
	myfclose(fp);
	free_gen_header(&state);

	lexer_cleanup(&l);
}
