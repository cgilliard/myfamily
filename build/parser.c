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
#include <base/resources.h>
#include <build/parser.h>
#include <lexer/lexer.h>
#include <limits.h>
#include <string.h>

u64 gen_file_counter = 0;

typedef struct HeaderTypeInfo {
	char type[MAX_NAME_LEN];
} HeaderTypeInfo;

typedef struct HeaderNameInfo {
	char name[MAX_NAME_LEN];
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
	ParserStateInImportListExpectModuleName = 6,
	ParserStateInImportListExpectSeparator = 7,
	ParserStateOther = 1000000,
} ParserStateEnum;

typedef struct ParserState {
	ParserStateEnum state;
	const ModuleInfo *cur;
	char *gen_header;
	u64 header_capacity;
	Vec *modules;
	Vec *types;
	char type_name[MAX_NAME_LEN + 1];
	HeaderTypes ht;
	ModuleInfo import_module_info;
	Vec import_list;
	u64 gen_file_counter;
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
			if (strlen(tk->token) > MAX_NAME_LEN) {
				token_display_error(
					tk, "token name is longer than MAX_NAME_LEN (%i)", MAX_NAME_LEN);
			} else {
				strcpy(state->type_name, tk->token);
			}
			state->state = ParserStateExpectBracket;
		}
	} else if (tk->type == TokenTypePunct) {
		if (!strcmp(tk->token, "}"))
			state->state = ParserStateBeginStatement;
		else if (!strcmp(tk->token, "$")) {
			state->import_module_info.sub_module_count = 0;
			state->state = ParserStateInImportListExpectModuleName;
		} else
			state->state = ParserStateOther;
	} else
		state->state = ParserStateOther;
}

void proc_ParserStateInImportListExpectModuleName(ParserState *state, Token *tk)
{
	if (tk->type != TokenTypeIdent) {
		token_display_error(tk, "Expected module name. Found [%s]", tk->token);
	} else {
		SubModuleInfo mi;
		strcpy(mi.name, tk->token);
		strcpy(
			state->import_module_info.module_list[state->import_module_info.sub_module_count].name,
			tk->token);
		state->import_module_info.sub_module_count++;
		state->state = ParserStateInImportListExpectSeparator;
	}
}

void proc_ParserStateInImportListExpectSeparator(ParserState *state, Token *tk)
{
	if (!strcmp(tk->token, "::")) {
		state->state = ParserStateInImportListExpectModuleName;
	} else if (!strcmp(tk->token, ";")) {
		u64 import_list_size = state->import_module_info.sub_module_count;
		if (import_list_size == 0) {
			token_display_error(tk, "Expected at least one module name here");
		} else {
			char buf[PATH_MAX + 1];
			module_to_type_string(&state->import_module_info, buf, PATH_MAX);
			char *last = state->import_module_info.module_list[import_list_size - 1].name;
			append_to_header(state, "#define %s %s\n", last, buf);
			state->state = ParserStateBeginStatement;
		}
	} else {
		token_display_error(tk, "Expected one of [';', '::']. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectType(ParserState *state, Token *tk)
{
	if (tk->type != TokenTypeIdent) {
		if (!strcmp(tk->token, "}")) {
			u64 count = vec_size(&(state->ht.types));
			char type_name[PATH_MAX];
			TypeInfo *ti = vec_element_at(state->types, vec_size(state->types) - 1);
			type_info_to_string(ti, type_name, PATH_MAX);
			append_to_header(state, "typedef struct %s {\n", type_name);
			for (u64 i = 0; i < count; i++) {
				HeaderNameInfo *ni;
				HeaderTypeInfo *ti;
				ti = vec_element_at(&(state->ht.types), i);
				ni = vec_element_at(&(state->ht.names), i);
				append_to_header(state, "%s %s;\n", ti->type, ni->name);
			}
			if (count == 0) {
				append_to_header(state, "char dummy;\n");
			}
			append_to_header(state, "} %s;\n\n", type_name);
			append_to_header(state, "#define %s %s\n", state->type_name, type_name);
			vec_clear(&(state->ht.types));
			vec_clear(&(state->ht.names));
			state->state = ParserStateBeginStatement;
		} else {
			token_display_error(tk, "Expected type name. Found [%s]", tk->token);
			state->state = ParserStateOther;
		}
	} else {
		if (strlen(tk->token) > MAX_NAME_LEN) {
			token_display_error(tk, "Name [%s] is too long.", tk->token);
			state->state = ParserStateOther;
		} else {
			HeaderTypeInfo hti;
			strcpy(hti.type, tk->token);
			vec_push(&(state->ht.types), &hti);
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
		if (strlen(tk->token) > MAX_NAME_LEN) {
			token_display_error(tk, "Name [%s] is too long.", tk->token);
			state->state = ParserStateOther;
		} else {
			HeaderNameInfo hti;
			strcpy(hti.name, tk->token);
			vec_push(&(state->ht.names), &hti);
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
			ModuleInfo mi;
			mi.sub_module_count = state->cur->sub_module_count + 1;
			for (u64 i = 0; i < state->cur->sub_module_count; i++) {
				strcpy(mi.module_list[i].name, state->cur->module_list[i].name);
			}
			strcpy(mi.module_list[state->cur->sub_module_count].name, tk->token);
			vec_push(state->modules, &mi);
		}
		state->state = ParserStateOther;
	}
}

void file_for(const char *base_dir, const ModuleInfo *self_info, char buf[PATH_MAX])
{
	Path ret;
	path_for(&ret, base_dir);
	for (u64 i = 0; self_info->sub_module_count != 0 && i < self_info->sub_module_count; i++) {
		path_push(&ret, self_info->module_list[i].name);
	}
	if (self_info->sub_module_count == 0) {
		path_push(&ret, "mod.h");
	} else {
		char module_file_f[PATH_MAX];
		strcpy(module_file_f, path_to_string(&ret));
		strcat(module_file_f, ".h");

		Path test_f;
		path_for(&test_f, module_file_f);
		if (path_exists(&test_f)) {
			path_cleanup(&ret);
			path_copy(&ret, &test_f);
		} else {
			path_pop(&ret);
			path_push(&ret, self_info->module_list[self_info->sub_module_count - 1].name);
			path_push(&ret, "mod.h");
			if (!path_exists(&ret)) {
				exit_error("Module: [%s] not found!", path_to_string(&ret));
			}
		}
	}
	if (path_canonicalize(&ret)) {
		exit_error("Path %s does not exist.", path_to_string(&ret));
	}
	strcpy(buf, path_to_string(&ret));
}

void parse_header(const char *config_dir, const char *base_dir, Vec *modules, Vec *types,
	const ModuleInfo *self_info)
{
	char file[PATH_MAX];
	file_for(base_dir, self_info, file);
	Lexer l;
	if (lexer_init(&l, file)) {
		exit_error("Could not open module file: '%s'", file);
	}

	ParserState state = {
		.state = ParserStateBeginStatement,
		.modules = modules,
		.gen_header = NULL,
		.header_capacity = 0,
		.cur = self_info,
		.types = types,
		.gen_file_counter = gen_file_counter,
	};
	gen_file_counter++;

	vec_init(&state.ht.names, 10, sizeof(HeaderNameInfo));
	vec_init(&state.ht.types, 10, sizeof(HeaderTypeInfo));
	vec_init(&state.import_list, 10, sizeof(SubModuleInfo));

	while (true) {
		Token tk;
		int res = lexer_next_token(&l, &tk);
		if (res == LexerStateComplete) {
			break;
		}
		// printf("state=%i,token_type=%i,token_value='%s'\n", state.state, tk.type, tk.token);
		if (tk.type != TokenTypeDoc) {
			// skip over doc comments for these purposes
			if (state.state == ParserStateBeginStatement) {
				proc_ParserStateBeginStatement(&state, &tk);
			} else if (state.state == ParserStateExpectModuleName) {
				proc_ParserStateExpectModuleName(&state, &tk);
			} else if (state.state == ParserStateExpectType) {
				proc_ParserStateExpectType(&state, &tk);
			} else if (state.state == ParserStateExpectName) {
				proc_ParserStateExpectName(&state, &tk);
			} else if (state.state == ParserStateInTypeExpectSemi) {
				proc_ParserStateInTypeExpectSemi(&state, &tk);
			} else if (state.state == ParserStateInImportListExpectModuleName) {
				proc_ParserStateInImportListExpectModuleName(&state, &tk);
			} else if (state.state == ParserStateInImportListExpectSeparator) {
				proc_ParserStateInImportListExpectSeparator(&state, &tk);
			} else if (tk.type == TokenTypePunct) {
				if (!strcmp(tk.token, "}") || !strcmp(tk.token, ";")) {
					state.state = ParserStateBeginStatement;
				} else if (!strcmp(tk.token, "{") && state.state == ParserStateExpectBracket) {
					TypeInfo ti;
					ti.mi = *state.cur;
					strcpy(ti.type_name, state.type_name);
					ti.gen_file_counter = state.gen_file_counter;
					vec_push(types, &ti);
					state.state = ParserStateExpectType;
				} else
					state.state = ParserStateOther;
			}
		}

		token_cleanup(&tk);
	}

	Path gen_header_name;
	char header_name[128];
	snprintf(header_name, 128, "f%" PRIu64 "", state.gen_file_counter);
	path_for(&gen_header_name, base_dir);
	path_push(&gen_header_name, "target");
	path_push(&gen_header_name, "include");
	path_push(&gen_header_name, header_name);
	path_mkdir(&gen_header_name, 0700, false);
	path_push(&gen_header_name, "build_specific.h");

	append_to_header(&state, "");

	FILE *fp = myfopen(path_to_string(&gen_header_name), "w");
	if (fprintf(fp, "%s", state.gen_header) < strlen(state.gen_header))
		fprintf(
			stderr, "Partial write of header: %s occurred!\n", path_to_string(&gen_header_name));

	myfclose(fp);
	free_gen_header(&state);
	vec_cleanup(&state.ht.names);
	vec_cleanup(&state.ht.types);
	vec_cleanup(&state.import_list);

	lexer_cleanup(&l);
}
