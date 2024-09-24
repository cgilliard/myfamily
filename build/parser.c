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
#include <base/panic.h>
#include <base/resources.h>
#include <build/parser.h>
#include <lexer/lexer.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_HEADER_CAPACITY (1024 * 25)

u64 gen_file_counter = 0;

typedef struct HeaderTypeInfo {
	char type[MAX_NAME_LEN];
	bool is_arr;
	u64 arr_size;
} HeaderTypeInfo;

typedef struct HeaderNameInfo {
	char name[MAX_NAME_LEN];
} HeaderNameInfo;

typedef struct HeaderTypes {
	Vec types;
	Vec names;
} HeaderTypes;

typedef struct ConfigType {
	char type_name[MAX_NAME_LEN];
} ConfigType;

typedef struct ConfigName {
	char name[MAX_NAME_LEN];
} ConfigName;

typedef struct FnParam {
	bool is_mut;
	char type[MAX_NAME_LEN];
	char name[MAX_NAME_LEN];
} FnParam;

typedef struct IncompleteFn {
	char name[MAX_NAME_LEN];
	bool is_mut;
	Vec params;
	char return_type[MAX_NAME_LEN];
	bool has_impl;
} IncompleteFn;

typedef struct SuperTraitInfo {
	char name[MAX_NAME_LEN];
} SuperTraitInfo;

typedef struct IncompleteType {
	u64 count;
	IncompleteFn *fns;
	char name[MAX_NAME_LEN];
	char path_name[PATH_MAX];
	SuperTraitInfo *si;
	u64 si_count;
} IncompleteType;

typedef struct GlobalIncompleteList {
	u64 count;
	IncompleteType *types;
} GlobalIncompleteList;

typedef struct ImplPair {
	char type[MAX_NAME_LEN];
	char trait[MAX_NAME_LEN];
} ImplPair;

typedef struct GlobalImplList {
	u64 count;
	ImplPair *impls;
} GlobalImplList;

GlobalImplList global_impl_list = {0, NULL};
GlobalIncompleteList global_incomplete_list = {0, NULL};

// TODO: these functions work, but use linear search. Move to something like an RB Tree.
void add_super_trait_list(const char *trait_name, const Vec *super_traits) {
	for (u64 i = 0; i < vec_size(super_traits); i++) {
		SuperTraitInfo *si = vec_element_at(super_traits, i);
	}
}

void add_trait_impl(const char *trait_name, const char *type_name) {
	if (global_impl_list.count == 0) {
		global_impl_list.impls = mymalloc(sizeof(ImplPair));
		if (global_impl_list.impls == NULL)
			panic("Could not allocate sufficient memory");

	} else {
		global_impl_list.impls =
			myrealloc(global_impl_list.impls, sizeof(ImplPair) * (global_impl_list.count + 1));
		if (global_impl_list.impls == NULL)
			panic("Could not allocate sufficient memory");
	}
	strcpy(global_impl_list.impls[global_impl_list.count].type, type_name);
	strcpy(global_impl_list.impls[global_impl_list.count].trait, trait_name);
	global_impl_list.count++;
}

bool check_super_traits(const char *type_name, const char *trait_name, char *buf, u64 limit) {
	for (u64 i = 0; i < global_incomplete_list.count; i++) {
		if (!strcmp(global_incomplete_list.types[i].path_name, trait_name)) {
			for (u64 j = 0; j < global_incomplete_list.types[i].si_count; j++) {
				char *required_trait = global_incomplete_list.types[i].si[j].name;
				bool impl = false;
				for (u64 k = 0; k < global_impl_list.count; k++) {
					if (!strcmp(global_impl_list.impls[k].trait, required_trait) &&
						!strcmp(global_impl_list.impls[k].type, type_name)) {
						impl = true;
						break;
					}
				}
				if (!impl) {
					strncpy(buf, required_trait, limit);
					return false;
				}
			}
		}
	}
	return true;
}

void add_to_global_incomplete_list(const char *type_name, const Vec *incomplete_fns,
								   const char *args_file, Vec *super_traits,
								   const char *trait_full_name) {
	if (super_traits != NULL)
		add_super_trait_list(trait_full_name, super_traits);
	if (global_incomplete_list.count == 0) {
		global_incomplete_list.types = mymalloc(sizeof(IncompleteType));
	} else {
		global_incomplete_list.types =
			myrealloc(global_incomplete_list.types,
					  sizeof(IncompleteType) * (global_incomplete_list.count + 1));
	}

	IncompleteType *next = &global_incomplete_list.types[global_incomplete_list.count];
	strcpy(next->name, type_name);
	if (trait_full_name)
		strcpy(next->path_name, trait_full_name);
	else
		strcpy(next->path_name, "");
	next->count = vec_size(incomplete_fns);
	next->fns = mymalloc(sizeof(IncompleteFn) * next->count);
	if (super_traits) {
		next->si = mymalloc(sizeof(SuperTraitInfo) * vec_size(super_traits));
		next->si_count = vec_size(super_traits);
		for (u64 i = 0; i < vec_size(super_traits); i++) {
			SuperTraitInfo *sti = vec_element_at(super_traits, i);
			strcpy(next->si[i].name, sti->name);
		}
	} else {
		next->si = NULL;
	}

	for (u64 i = 0; i < next->count; i++) {
		IncompleteFn *fn = vec_element_at(incomplete_fns, i);
		next->fns[i].has_impl = fn->has_impl;
		strcpy(next->fns[i].name, fn->name);
		strcpy(next->fns[i].return_type, fn->return_type);
		vec_init(&next->fns[i].params, 3, sizeof(FnParam));
		FILE *fp = myfopen(args_file, "a");

		fprintf(fp, "-DFn_override_%s_return=\"%s\"\n", fn->name, fn->return_type);
		if (next->fns[i].is_mut) {
			fprintf(fp, "-DFn_override_%s_params=\"_%s(Obj *self", fn->name, fn->name);
		} else {
			fprintf(fp, "-DFn_override_%s_params=\"_%s(const Obj *self", fn->name, fn->name);
		}
		if (vec_size(&fn->params))
			fprintf(fp, ",");
		for (u64 j = 0; j < vec_size(&fn->params); j++) {
			FnParam *param = vec_element_at(&fn->params, j);
			vec_push(&next->fns[i].params, param);
			char *comma;
			if (j != vec_size(&fn->params) - 1)
				comma = ",";
			else
				comma = "";
			if (!strcmp(param->type, "__config__")) {
				fprintf(fp, " void *__selfconfig__%s", comma);
			} else {
				if (param->is_mut) {
					fprintf(fp, " %s %s%s", param->type, param->name, comma);
				} else {
					fprintf(fp, " const %s %s%s", param->type, param->name, comma);
				}
			}
		}
		fprintf(fp, ")\"\n");

		fprintf(fp, "-DFn_expand_%s_return=\"%s\"\n", fn->name, fn->return_type);
		if (next->fns[i].is_mut) {
			fprintf(fp, "-DFn_expand_%s_params=\"_%s(Obj *self", fn->name, fn->name);
		} else {
			fprintf(fp, "-DFn_expand_%s_params=\"_%s(const Obj *self", fn->name, fn->name);
		}
		if (vec_size(&fn->params))
			fprintf(fp, ",");
		for (u64 j = 0; j < vec_size(&fn->params); j++) {
			FnParam *param = vec_element_at(&fn->params, j);
			char *comma;
			if (j != vec_size(&fn->params) - 1)
				comma = ",";
			else
				comma = "";
			if (!strcmp(param->type, "__config__")) {
				fprintf(fp, " void *__selfconfig__%s", comma);
			} else {
				if (param->is_mut) {
					fprintf(fp, " %s %s%s", param->type, param->name, comma);
				} else {
					fprintf(fp, " const %s %s%s", param->type, param->name, comma);
				}
			}
		}
		fprintf(fp, ")\"\n");
		myfclose(fp);
	}

	global_incomplete_list.count++;
}

void incomplete_fns_cleanup(Vec *incomplete_fns) {
	while (vec_size(incomplete_fns)) {
		IncompleteFn *next = vec_pop(incomplete_fns);
		vec_cleanup(&next->params);
	}
}

typedef enum ParserStateEnum {
	ParserStateBeginStatement,
	ParserStateExpectModuleName,
	ParserStateExpectBrace,
	ParserStateExpectType,
	ParserStateExpectName,
	ParserStateInTypeExpectSemi,
	ParserStateInImportListExpectModuleName,
	ParserStateInImportListExpectSeparator,
	ParserStateExpectParenBuildDefn,
	ParserStateExpectConfigParen,
	ParserStateExpectConfigType,
	ParserStateExpectConfigComma,
	ParserStateExpectConfigName,
	ParserStateIncompleteName,
	ParserIncompleteExpectBrace,
	ParserStateExpectAt,
	ParserStateExpectIncompleteFnName,
	ParserStateExpectIncompleteFnParenStart,
	ParserStateExpectIncompleteFnMutOrType,
	ParserStateExpectIncompleteFnType,
	ParserStateExpectIncompleteFnParamName,
	ParserStateExpectIncompleteFnParamCommaOrEnd,
	ParserStateIncompleteExpectSemi,
	ParserStateIncompleteExpectSemiOrTypeArrow,
	ParserStateIncompleteExpectReturnType,
	ParserStateExpectIncompleteNameForImpl,
	ParserStateImplExpectSemi,
	ParserStateExpectIncompleteMutFnName,
	ParserStateExpectPipe,
	ParserStateExpectArraySize,
	ParserStateExpectEndArray,
	ParserStateExpectConfigKeyword,
	ParserStateExpectIncompleteMutFnNameWithDefaultImpl,
	ParserStateExpectSuperTrait,
	ParserStateExpectPlusOrBrace,
	ParserStateOther,
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
	Vec config_types;
	Vec config_names;
	Vec incomplete_fns;
	bool has_errors;
	bool cur_is_array;
	Vec super_traits;
} ParserState;

void append_to_header(ParserState *state, const char *text, ...) {
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
		char text_with_nl[strlen(text) + 2];
		strcpy(text_with_nl, text);
		strcat(text_with_nl, "\n");
		vsnprintf(state->gen_header, size, text_with_nl, args);
		va_end(args);

	} else {
		u64 size = state->header_capacity;
		if (length + strlen(state->gen_header) + 1 >= state->header_capacity) {
			size = state->header_capacity + INITIAL_HEADER_CAPACITY;
			if (length + strlen(state->gen_header) + 1 >=
				state->header_capacity + INITIAL_HEADER_CAPACITY) {
				size = length + strlen(state->gen_header) + 1;
			}
			state->gen_header = myrealloc(state->gen_header, sizeof(char) * (size));
			if (state->gen_header == NULL)
				exit_error("Could not allocate sufficient memory to continue!");
		}

		va_start(args, text);
		char text_with_nl[strlen(text) + 2];
		strcpy(text_with_nl, text);
		strcat(text_with_nl, "\n");
		vsnprintf(state->gen_header + strlen(state->gen_header), size - strlen(state->gen_header),
				  text_with_nl, args);
		va_end(args);
	}

	state->header_capacity += INITIAL_HEADER_CAPACITY;
}

void free_gen_header(ParserState *state) {
	if (state->gen_header)
		myfree(state->gen_header);
}

void proc_ParserStateBeginStatement(ParserState *state, Token *tk) {
	if (tk->type == TokenTypeIdent) {
		if (!strcmp(tk->token, "mod")) {
			state->state = ParserStateExpectModuleName;
		} else {
			if (strlen(tk->token) > MAX_NAME_LEN) {
				state->has_errors = true;
				token_display_error(tk, "token name is longer than MAX_NAME_LEN (%i)",
									MAX_NAME_LEN);
			} else {
				strcpy(state->type_name, tk->token);
			}
			state->state = ParserStateExpectBrace;
		}
	} else if (tk->type == TokenTypePunct) {
		if (!strcmp(tk->token, "}"))
			state->state = ParserStateBeginStatement;
		else if (!strcmp(tk->token, "$")) {
			state->import_module_info.sub_module_count = 0;
			state->state = ParserStateInImportListExpectModuleName;
		} else if (!strcmp(tk->token, "@")) {
			state->state = ParserStateIncompleteName;
		} else
			state->state = ParserStateOther;
	} else
		state->state = ParserStateOther;
}

void proc_ParserStateInImportListExpectModuleName(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->has_errors = true;
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

void proc_ParserStateInImportListExpectSeparator(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "::")) {
		state->state = ParserStateInImportListExpectModuleName;
	} else if (!strcmp(tk->token, ";")) {
		u64 import_list_size = state->import_module_info.sub_module_count;
		if (import_list_size == 0) {
			state->has_errors = true;
			token_display_error(tk, "Expected at least one module name here");
		} else {
			char type_name[PATH_MAX + 1];
			module_to_type_string(&state->import_module_info, type_name, PATH_MAX);
			char *last = state->import_module_info.module_list[import_list_size - 1].name;
			append_to_header(state, "typedef struct %s %s;", type_name, type_name);
			append_to_header(state, "#define %s %s", last, type_name);
			append_to_header(state, "Type_Import_Expand_%s_", type_name);
			state->state = ParserStateBeginStatement;
		}
	} else {
		state->has_errors = true;
		token_display_error(tk, "Expected one of [';', '::']. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectConfigKeyword(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "Config")) {
		state->state = ParserStateExpectParenBuildDefn;
	} else {
		state->has_errors = true;
		token_display_error(tk, "Expected ['Config']. Found [%s]", tk->token);
		state->state = ParserStateBeginStatement;
	}
}

void proc_ParserStateExpectParenBuildDefn(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "("))
		state->state = ParserStateExpectConfigType;
	else {
		state->has_errors = true;
		token_display_error(tk, "Expected ['(']. Found [%s]", tk->token);
		state->state = ParserStateBeginStatement;
	}
}

void proc_ParserStateExpectConfigType(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->has_errors = true;
		token_display_error(tk, "Expected Type name. Found [%s]", tk->token);
		state->state = ParserStateBeginStatement;
	} else {
		if (strcmp(tk->token, "u8") && strcmp(tk->token, "u16") && strcmp(tk->token, "u32") &&
			strcmp(tk->token, "u64") && strcmp(tk->token, "u128") && strcmp(tk->token, "bool") &&
			strcmp(tk->token, "String")) {
			state->has_errors = true;
			token_display_error(
				tk, "Expected one of [u8, u16, u32, u64, u128, bool, String]. Found [%s]",
				tk->token);
		} else {
			ConfigType ct;
			if (!strcmp(tk->token, "String"))
				strcpy(ct.type_name, "char *");
			else
				strcpy(ct.type_name, tk->token);
			vec_push(&state->config_types, &ct);
		}

		state->state = ParserStateExpectConfigName;
	}
}

void proc_ParserStateExpectConfigComma(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, ",")) {
		state->state = ParserStateExpectConfigType;
	} else if (!strcmp(tk->token, ")")) {
		state->state = ParserStateInTypeExpectSemi;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected ','. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectConfigName(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->has_errors = true;
		token_display_error(tk, "Expected config name. Found [%s]", tk->token);
		state->state = ParserStateBeginStatement;
	} else {
		ConfigName cn;
		strcpy(cn.name, tk->token);
		vec_push(&state->config_names, &cn);

		state->state = ParserStateExpectConfigComma;
	}
}

void proc_ParserStateExpectType(ParserState *state, Token *tk, const char *args_file) {
	if (!strcmp(tk->token, "$")) {
		// Config definition
		vec_clear(&state->config_names);
		vec_clear(&state->config_types);
		state->state = ParserStateExpectConfigKeyword;
	} else if (!strcmp(tk->token, "[")) {
		// array
		state->cur_is_array = true;
		state->state = ParserStateExpectType;
	} else if (tk->type != TokenTypeIdent) {
		if (!strcmp(tk->token, "}")) {
			u64 count = vec_size(&(state->ht.types));
			char type_name[PATH_MAX];
			TypeInfo *ti = vec_element_at(state->types, vec_size(state->types) - 1);
			type_info_to_string(ti, type_name, PATH_MAX);
			append_to_header(state, "typedef struct %s %s;", type_name, type_name);
			append_to_header(state, "typedef struct %s {", type_name);
			for (u64 i = 0; i < count; i++) {
				HeaderNameInfo *ni;
				HeaderTypeInfo *ti;
				ti = vec_element_at(&(state->ht.types), i);
				ni = vec_element_at(&(state->ht.names), i);
				if (ti->is_arr) {
					if (ti->arr_size == UINT64_MAX) {
						// dynamic array
						append_to_header(state, "%s *%s; FatPtr %s_fat_ptr__;", ti->type, ni->name,
										 ni->name);
					} else {
						// static array
						append_to_header(state, "%s %s[%" PRIu64 "];", ti->type, ni->name,
										 ti->arr_size);
					}
				} else {
					if (!strcmp(ti->type, "u8") || !strcmp(ti->type, "u16") ||
						!strcmp(ti->type, "u32") || !strcmp(ti->type, "u64") ||
						!strcmp(ti->type, "u128") || !strcmp(ti->type, "i8") ||
						!strcmp(ti->type, "i16") || !strcmp(ti->type, "i32") ||
						!strcmp(ti->type, "i64") || !strcmp(ti->type, "i128") ||
						!strcmp(ti->type, "f32") || !strcmp(ti->type, "f64") ||
						!strcmp(ti->type, "bool"))
						append_to_header(state, "%s %s;", ti->type, ni->name);
					else
						append_to_header(state, "Obj %s;", ni->name);
				}
			}
			if (count == 0) {
				append_to_header(state, "char dummy;");
			}
			append_to_header(state, "} %s;\n", type_name);

			append_to_header(state, "extern Vtable %s_Vtable__;", type_name);
			append_to_header(state, "u64 %s_size();", type_name);
			append_to_header(state, "void %s_drop_internal(Obj *ptr);", type_name);
			append_to_header(state, "void %s_build_internal(Obj *ptr);", type_name);

			// add to args
			FILE *fp = myfopen(args_file, "a");
			fprintf(fp, "-DType_Expand_%s_=\"", type_name);
			fprintf(fp, "Vtable %s_Vtable__ = {\\\"%s\\\", 0, NULL, 0, NULL, false};\
				u64 %s_size() {return sizeof(%s); }\
				void %s_drop_internal(Obj *ptr) { ",
					type_name, type_name, type_name, type_name, type_name);
			for (u64 i = 0; i < count; i++) {
				HeaderNameInfo *ni;
				HeaderTypeInfo *ti;
				ti = vec_element_at(&(state->ht.types), i);
				ni = vec_element_at(&(state->ht.names), i);
				if (ti->is_arr && ti->arr_size == UINT64_MAX) {
					fprintf(fp, "$Free(%s);", ni->name);
				} else if (!ti->is_arr) {
					if (!(!strcmp(ti->type, "u8") || !strcmp(ti->type, "u16") ||
						  !strcmp(ti->type, "u32") || !strcmp(ti->type, "u64") ||
						  !strcmp(ti->type, "u128") || !strcmp(ti->type, "i8") ||
						  !strcmp(ti->type, "i16") || !strcmp(ti->type, "i32") ||
						  !strcmp(ti->type, "i64") || !strcmp(ti->type, "i128") ||
						  !strcmp(ti->type, "f32") || !strcmp(ti->type, "f64") ||
						  !strcmp(ti->type, "bool"))) {
						// not a prim
						fprintf(fp, "Obj_cleanup(&$Mut(%s));", ni->name);
					}
				}
			}
			fprintf(fp, "}\
				void %s_build_internal(Obj *ptr) {\
				u64 size = %s_size();                     \
				memset(ptr->ptr.data, 0, size);           \
				",
					type_name, type_name);

			for (u64 i = 0; i < count; i++) {
				HeaderNameInfo *ni;
				HeaderTypeInfo *ti;
				ti = vec_element_at(&(state->ht.types), i);
				ni = vec_element_at(&(state->ht.names), i);
				if (ti->is_arr) {
				} else {
					if (!(!strcmp(ti->type, "u8") || !strcmp(ti->type, "u16") ||
						  !strcmp(ti->type, "u32") || !strcmp(ti->type, "u64") ||
						  !strcmp(ti->type, "u128") || !strcmp(ti->type, "i8") ||
						  !strcmp(ti->type, "i16") || !strcmp(ti->type, "i32") ||
						  !strcmp(ti->type, "i64") || !strcmp(ti->type, "i128") ||
						  !strcmp(ti->type, "f32") || !strcmp(ti->type, "f64") ||
						  !strcmp(ti->type, "bool"))) {
						fprintf(fp, "$Mut(%s) = OBJECT_INIT;", ni->name);
					}
				}
			}

			fprintf(fp, "}");
			fprintf(fp, "\"\n");
			append_to_header(
				state, "static void __attribute__((constructor)) __add_impls_%s_vtable() {\
				\nVtableEntry size =\
				{\
				\"size\", %s_size}; \
				\nvtable_add_entry(&%s_Vtable__, size); \
				\nVtableEntry build_internal = {\"build_internal\", %s_build_internal};\
				\nvtable_add_entry(& % s_Vtable__, build_internal);\
				\nVtableEntry drop_internal = {\"drop_internal\", %s_drop_internal}; \
				\nvtable_add_entry(&%s_Vtable__, drop_internal);  \n}",
				type_name, type_name, type_name, type_name, type_name, type_name, type_name);

			u64 count_configs = vec_size(&state->config_types);
			u64 count_config_names = vec_size(&state->config_names);
			append_to_header(state, "typedef struct %sConfig {", type_name);
			fprintf(fp, "-DType_Import_Expand_%s_=\"", type_name);
			fprintf(fp, "u64 %s_size();", type_name);
			fprintf(fp, "extern Vtable %s_Vtable__;", type_name);
			fprintf(fp, "typedef struct %sConfig {", type_name);

			if (count_configs == count_config_names && count_configs > 0) {
				for (u64 i = 0; i < count_configs; i++) {
					ConfigType *t = vec_element_at(&state->config_types, i);
					ConfigName *n = vec_element_at(&state->config_names, i);
					append_to_header(state, "%s %s; bool %s_is_set__;", t->type_name, n->name,
									 n->name);
					fprintf(fp, "%s %s; bool %s_is_set__;", t->type_name, n->name, n->name);
				}
			} else {
				append_to_header(state, "char dummy;");
				fprintf(fp, "char dummy;");
			}
			append_to_header(state, "} %sConfig;", type_name);
			fprintf(fp, "} %sConfig;", type_name);
			fprintf(fp, "\"\n");

			myfclose(fp);

			append_to_header(state, "#define %s %s\n", state->type_name, type_name);
			vec_clear(&(state->ht.types));
			vec_clear(&(state->ht.names));
			state->state = ParserStateBeginStatement;
		} else {
			state->has_errors = true;
			token_display_error(tk, "Expected type name. Found [%s]", tk->token);
			state->state = ParserStateOther;
		}
	} else {
		if (strlen(tk->token) > MAX_NAME_LEN) {
			state->has_errors = true;
			token_display_error(tk, "Name [%s] is too long.", tk->token);
			state->state = ParserStateOther;
		} else {
			HeaderTypeInfo hti;
			hti.is_arr = state->cur_is_array;
			strcpy(hti.type, tk->token);
			vec_push(&(state->ht.types), &hti);
			if (state->cur_is_array) {
				state->state = ParserStateExpectPipe;
			} else {
				state->state = ParserStateExpectName;
			}
		}
	}
}

void proc_ParserStateExpectPipe(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "|")) {
		state->state = ParserStateExpectArraySize;
	} else if (!strcmp(tk->token, "]")) {
		// dynamic array
		HeaderTypeInfo *hti;
		hti = vec_element_at(&(state->ht.types), vec_size(&(state->ht.types)) - 1);
		// use UINT64_MAX to indicate dynamic array
		hti->arr_size = UINT64_MAX;
		state->state = ParserStateExpectName;
	} else {
		state->has_errors = true;
		token_display_error(tk, "Expected ['|']. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectArraySize(ParserState *state, Token *tk) {
	if (tk->type == TokenTypeLiteral) {
		char first = tk->token[0];
		if (!(first >= '0' && first <= '9')) {
			state->has_errors = true;
			token_display_error(tk, "Expected number literal. Found [%s]", tk->token);
			state->state = ParserStateBeginStatement;
		} else {
			char *endptr;
			HeaderTypeInfo *hti;
			hti = vec_element_at(&(state->ht.types), vec_size(&(state->ht.types)) - 1);
			hti->arr_size = strtoull(tk->token, &endptr, 10);

			state->state = ParserStateExpectEndArray;
		}
	} else {
		state->has_errors = true;
		token_display_error(tk, "Expected number literal. Found [%s]", tk->token);
		state->state = ParserStateBeginStatement;
	}
}

void proc_ParserStateExpectEndArray(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "]")) {
		state->state = ParserStateExpectName;
	} else {
		state->has_errors = true;
		token_display_error(tk, "Expected [']']. Found [%s]", tk->token);
		state->state = ParserStateBeginStatement;
	}
}

void proc_ParserStateExpectName(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->has_errors = true;
		token_display_error(tk, "Expected name. Found [%s]", tk->token);
		state->state = ParserStateOther;
	} else {
		if (strlen(tk->token) > MAX_NAME_LEN) {
			state->has_errors = true;
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

void proc_ParserStateInTypeExpectSemi(ParserState *state, Token *tk) {
	if (strcmp(tk->token, ";")) {
		state->has_errors = true;
		token_display_error(tk, "Expected ';'. Found [%s]", tk->token);
	}
	state->state = ParserStateExpectType;
	state->cur_is_array = false;
}

void proc_ParserStateExpectModuleName(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->has_errors = true;
		token_display_error(tk, "Expected module name, Found [%s]", tk->token);
	} else {
		if (strlen(tk->token) >= PATH_MAX) {
			state->has_errors = true;
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

void proc_ParserStateIncompleteName(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected incomplete type name. Found [%s]", tk->token);
	} else {
		state->state = ParserIncompleteExpectBrace;
		strcpy(state->type_name, tk->token);
	}
}

void proc_ParserStateExpectPlusOrBrace(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "+"))
		state->state = ParserStateExpectSuperTrait;
	else if (!strcmp(tk->token, "{"))
		state->state = ParserStateExpectAt;
	else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected '+' or '{'. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectSuperTrait(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected super trait name. Found [%s]", tk->token);
	} else {
		state->state = ParserStateExpectPlusOrBrace;
		SuperTraitInfo si;
		TypeInfo ti;
		ti.mi = *state->cur;
		strcpy(ti.type_name, tk->token);

		type_info_to_string(&ti, si.name, MAX_NAME_LEN);
		vec_push(&state->super_traits, &si);
	}
}

void proc_ParserIncompleteExpectBrace(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "{")) {
		state->state = ParserStateExpectAt;
	} else if (!strcmp(tk->token, ":")) {
		vec_clear(&state->super_traits);
		state->state = ParserStateExpectSuperTrait;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected '{'. Found [%s]", tk->token);
	}
}

void expand_params(ParserState *state, IncompleteFn *fn, FILE *fp) {
	u64 param_count = vec_size(&fn->params);
	if (param_count) {
		append_to_header(state, ",");
		if (fp) {
			fprintf(fp, ",");
		}
	}
	for (u64 i = 0; i < param_count; i++) {
		FnParam *fparam = vec_element_at(&fn->params, i);
		char *comma;
		if (i != param_count - 1)
			comma = ", ";
		else
			comma = "";
		// handle special case for __config__ (in build)
		if (!strcmp("__config__", fparam->type)) {
			append_to_header(state, "void *%s%s", fparam->name, comma);
			if (fp) {
				fprintf(fp, "void *%s%s", fparam->name, comma);
			}
		} else {
			if (fparam->is_mut) {
				append_to_header(state, "%s %s%s", fparam->type, fparam->name, comma);
				if (fp) {
					fprintf(fp, "%s %s%s", fparam->type, fparam->name, comma);
				}
			} else {
				append_to_header(state, "const %s %s%s", fparam->type, fparam->name, comma);
				if (fp) {
					fprintf(fp, "const %s %s%s", fparam->type, fparam->name, comma);
				}
			}
		}
	}
}

void expand_vars(ParserState *state, IncompleteFn *fn, FILE *fp) {
	u64 param_count = vec_size(&fn->params);
	if (param_count) {
		append_to_header(state, ",");
		if (fp) {
			fprintf(fp, ",");
		}
	}
	for (u64 i = 0; i < param_count; i++) {
		FnParam *fparam = vec_element_at(&fn->params, i);
		char *comma;
		if (i != param_count - 1)
			comma = ", ";
		else
			comma = "";
		append_to_header(state, "%s%s", fparam->name, comma);
		if (fp)
			fprintf(fp, "%s%s", fparam->name, comma);
	}
}

void proc_ParserStateExpectAt(ParserState *state, Token *tk, const char *args_file) {
	if (!strcmp(tk->token, "@")) {
		state->state = ParserStateExpectIncompleteFnName;
	} else if (!strcmp(tk->token, "}")) {
		// end incomplete type
		state->state = ParserStateBeginStatement;
		u64 fn_count = vec_size(&state->incomplete_fns);

		FILE *fp = myfopen(args_file, "a");
		TypeInfo ti;
		ti.mi = *state->cur;
		strcpy(ti.type_name, state->type_name);

		char incomplete_type[PATH_MAX + 1];
		type_info_to_string(&ti, incomplete_type, PATH_MAX);
		fprintf(fp, "-DType_Import_Expand_%s_=\"", incomplete_type);
		for (u64 i = 0; i < fn_count; i++) {
			IncompleteFn *fn = vec_element_at(&state->incomplete_fns, i);

			if (fn->is_mut) {
				append_to_header(state, "static %s %s(Obj *self", fn->return_type, fn->name);
				fprintf(fp, "static %s %s(Obj *self ", fn->return_type, fn->name);
			} else {
				append_to_header(state, "static %s %s(const Obj *self", fn->return_type, fn->name);
				fprintf(fp, "static %s %s(const Obj *self ", fn->return_type, fn->name);
			}
			expand_params(state, fn, fp);
			append_to_header(state, ") {");
			fprintf(fp, ") {");

			if (fn->is_mut) {
				append_to_header(state, "%s (*impl)(Obj *self", fn->return_type);
				fprintf(fp, "%s (*impl)(Obj *self", fn->return_type);
			} else {
				append_to_header(state, "%s (*impl)(const Obj *self", fn->return_type);
				fprintf(fp, "%s (*impl)(const Obj *self", fn->return_type);
			}
			expand_params(state, fn, fp);
			append_to_header(state, ") = find_fn(self, \"%s\");", fn->name);
			fprintf(fp, ") = find_fn(self, \\\"%s\\\");", fn->name);
			append_to_header(
				state, "if(impl == NULL) panic(\"Implementation for [%s] not found!\");", fn->name);

			fprintf(fp, "if(impl == NULL) panic(\\\"Implementation for [%s] not found!\\\");",
					fn->name);
			append_to_header(
				state, "SelfCleanup sc = {__thread_local_self_Const, __thread_local_self_Var};");
			fprintf(fp, "SelfCleanup sc = {__thread_local_self_Const, __thread_local_self_Var};");
			append_to_header(state, "__thread_local_self_Const = self;");
			fprintf(fp, "__thread_local_self_Const = self;");
			if (fn->is_mut) {
				append_to_header(state, "__thread_local_self_Var = self;");
				fprintf(fp, "__thread_local_self_Var = self;");
			} else {
				append_to_header(state, "__thread_local_self_Var = NULL;");
				fprintf(fp, "__thread_local_self_Var = NULL;");
			}

			append_to_header(state, "return impl(self");
			fprintf(fp, "return impl(self");
			expand_vars(state, fn, fp);
			append_to_header(state, ");");
			fprintf(fp, ");");
			append_to_header(state, "}");
			fprintf(fp, "}");
		}

		fprintf(fp, "\"\n");
		myfclose(fp);

		add_to_global_incomplete_list(state->type_name, &state->incomplete_fns, args_file,
									  &state->super_traits, incomplete_type);
		vec_clear(&state->super_traits);

		incomplete_fns_cleanup(&state->incomplete_fns);

	} else if (tk->type == TokenTypeIdent) {
		// required method with an implementation.
		if (!strcmp(tk->token, "mut")) {
			state->state = ParserStateExpectIncompleteMutFnNameWithDefaultImpl;
		} else {
			IncompleteFn fn;
			fn.has_impl = true;
			fn.is_mut = false;
			strcpy(fn.name, tk->token);
			vec_init(&fn.params, 3, sizeof(FnParam));
			strcpy(fn.return_type, "void");
			vec_push(&state->incomplete_fns, &fn);
			state->state = ParserStateExpectIncompleteFnParenStart;
		}
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected '@'. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectIncompleteMutFnNameWithDefaultImpl(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected required function name. Found [%s]", tk->token);
	} else {
		IncompleteFn fn;
		fn.has_impl = true;
		fn.is_mut = true;
		strcpy(fn.name, tk->token);
		vec_init(&fn.params, 3, sizeof(FnParam));
		strcpy(fn.return_type, "void");
		vec_push(&state->incomplete_fns, &fn);
		state->state = ParserStateExpectIncompleteFnParenStart;
	}
}

void proc_ParserStateExpectIncompleteMutFnName(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected required function name. Found [%s]", tk->token);
	} else {
		IncompleteFn fn;
		fn.has_impl = false;
		fn.is_mut = true;
		strcpy(fn.name, tk->token);
		vec_init(&fn.params, 3, sizeof(FnParam));
		strcpy(fn.return_type, "void");
		vec_push(&state->incomplete_fns, &fn);
		state->state = ParserStateExpectIncompleteFnParenStart;
	}
}

void proc_ParserStateExpectIncompleteFnName(ParserState *state, Token *tk) {
	if (tk->type != TokenTypeIdent) {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected required function name. Found [%s]", tk->token);
	} else {
		if (!strcmp(tk->token, "mut")) {
			state->state = ParserStateExpectIncompleteMutFnName;
		} else {
			IncompleteFn fn;
			fn.has_impl = false;
			fn.is_mut = false;
			strcpy(fn.name, tk->token);
			vec_init(&fn.params, 3, sizeof(FnParam));
			strcpy(fn.return_type, "void");
			vec_push(&state->incomplete_fns, &fn);
			state->state = ParserStateExpectIncompleteFnParenStart;
		}
	}
}

void proc_ParserStateExpectIncompleteFnParenStart(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "(")) {
		state->state = ParserStateExpectIncompleteFnMutOrType;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected ['(']. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectIncompleteFnType(ParserState *state, Token *tk) {
	if (tk->type == TokenTypeIdent) {
		u64 last = vec_size(&state->incomplete_fns);
		if (last == 0) {
			state->has_errors = true;
			token_display_error(tk, "internal error: Expected type here!");
			state->state = ParserStateBeginStatement;
		} else {
			last--;
			IncompleteFn *fn = vec_element_at(&state->incomplete_fns, last);
			u64 last_param = vec_size(&fn->params);
			if (last_param == 0) {
				state->has_errors = true;
				token_display_error(tk, "internal error: Expected a param here!");
				state->state = ParserStateBeginStatement;
			} else {
				last_param--;
				FnParam *p = vec_element_at(&fn->params, last_param);
				strcpy(p->type, tk->token);
				state->state = ParserStateExpectIncompleteFnParamName;
			}
		}
	} else {
		// unexpected - error
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected type name. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectIncompleteFnParamName(ParserState *state, Token *tk) {
	if (tk->type == TokenTypeIdent) {
		u64 last = vec_size(&state->incomplete_fns);
		if (last == 0) {
			state->has_errors = true;
			token_display_error(tk, "internal error: Expected a name here!");
			state->state = ParserStateBeginStatement;
		} else {
			last--;
			IncompleteFn *fn = vec_element_at(&state->incomplete_fns, last);
			u64 last_param = vec_size(&fn->params);
			if (last_param == 0) {
				state->has_errors = true;
				token_display_error(tk, "internal error: Expected a param here!");
				state->state = ParserStateBeginStatement;
			} else {
				last_param--;
				FnParam *p = vec_element_at(&fn->params, last_param);
				strcpy(p->name, tk->token);
				state->state = ParserStateExpectIncompleteFnParamCommaOrEnd;
			}
		}
	} else {
		// unexpected - error
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected param name. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectIncompleteFnParamCommaOrEnd(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, ",")) {
		// look for the next param
		state->state = ParserStateExpectIncompleteFnMutOrType;
	} else if (!strcmp(tk->token, ")")) {
		state->state = ParserStateIncompleteExpectSemiOrTypeArrow;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected [',' or ')']. Found [%s]", tk->token);
	}
}

void proc_ParserStateIncompleteExpectSemi(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, ";")) {
		state->state = ParserStateExpectAt;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected ';'. Found [%s]", tk->token);
	}
}

void proc_ParserStateIncompleteExpectSemiOrTypeArrow(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "->")) {
		state->state = ParserStateIncompleteExpectReturnType;
	} else if (!strcmp(tk->token, ";")) {
		state->state = ParserStateExpectAt;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected [';' or '->']. Found [%s]", tk->token);
	}
}

void proc_ParserStateIncompleteExpectReturnType(ParserState *state, Token *tk) {
	if (tk->type == TokenTypeIdent) {
		u64 last = vec_size(&state->incomplete_fns);
		if (last == 0) {
			state->has_errors = true;
			token_display_error(tk, "internal error: Expected return type here!");
			state->state = ParserStateBeginStatement;
		} else {
			last--;
			IncompleteFn *fn = vec_element_at(&state->incomplete_fns, last);
			strcpy(fn->return_type, tk->token);
			state->state = ParserStateIncompleteExpectSemi;
		}
	} else {
		// unexpected - error
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected return type. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectIncompleteFnMutOrType(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, "mut")) {
		u64 last = vec_size(&state->incomplete_fns);
		if (last == 0) {
			state->has_errors = true;
			token_display_error(tk, "internal error: Expected an incomplete function here!");
			state->state = ParserStateBeginStatement;
		} else {
			last--;
			IncompleteFn *fn = vec_element_at(&state->incomplete_fns, last);
			FnParam param;
			param.is_mut = true;
			vec_push(&fn->params, &param);
			state->state = ParserStateExpectIncompleteFnType;
		}
	} else if (tk->type == TokenTypeIdent) {
		// It's a type name pass on to Type fn
		u64 last = vec_size(&state->incomplete_fns);
		if (last == 0) {
			state->has_errors = true;
			token_display_error(tk, "internal error: Expected an incomplete function here!");
			state->state = ParserStateBeginStatement;
		} else {
			last--;
			IncompleteFn *fn = vec_element_at(&state->incomplete_fns, last);
			FnParam param;
			param.is_mut = false;
			vec_push(&fn->params, &param);
			proc_ParserStateExpectIncompleteFnType(state, tk);
		}
	} else if (!strcmp(tk->token, ")")) {
		// this is a parameterless function
		state->state = ParserStateIncompleteExpectSemiOrTypeArrow;
	} else {
		// unexpected - error
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected 'mut' or type name. Found [%s]", tk->token);
	}
}

void proc_ParserStateExpectIncompleteNameForImpl(ParserState *state, Token *tk,
												 const char *args_file) {
	if (tk->type == TokenTypeIdent) {
		TypeInfo ti;
		ti.mi = *state->cur;
		strcpy(ti.type_name, state->type_name);

		TypeInfo ti_inc;
		ti_inc.mi = *state->cur;
		strcpy(ti_inc.type_name, tk->token);
		char incomplete_full_type[PATH_MAX + 1];
		type_info_to_string(&ti_inc, incomplete_full_type, PATH_MAX);

		char complete_type[PATH_MAX + 1];
		type_info_to_string(&ti, complete_type, PATH_MAX);
		char *incomplete_type = tk->token;
		bool found = false;
		bool has_impl = false;
		char required_trait[PATH_MAX + 1];
		bool ret =
			check_super_traits(complete_type, incomplete_full_type, required_trait, PATH_MAX);
		if (!ret) {
			state->has_errors = true;
			// TODO: switch to path version of missing trait mod1::mod2::MyTrait
			token_display_error(
				tk, "All super traits not implemented for type [%s]. Missing trait [%s]",
				state->type_name, required_trait);
		}
		add_trait_impl(incomplete_full_type, complete_type);
		for (u64 i = 0; i < global_incomplete_list.count; i++) {
			if (!strcmp(incomplete_type, global_incomplete_list.types[i].name)) {
				IncompleteType *inc = &global_incomplete_list.types[i];
				for (u64 j = 0; j < inc->count; j++) {
					if (inc->fns[j].has_impl)
						has_impl = true;
					if (inc->fns[j].has_impl) {
						if (inc->fns[j].is_mut) {
							append_to_header(state, "%s %s_%s(Obj *self", inc->fns[j].return_type,
											 incomplete_full_type, inc->fns[j].name);
						} else {
							append_to_header(state, "%s %s_%s(const Obj *self",
											 inc->fns[j].return_type, incomplete_full_type,
											 inc->fns[j].name);
						}
					} else {
						if (inc->fns[j].is_mut) {
							append_to_header(state, "%s %s_%s(Obj *self", inc->fns[j].return_type,
											 complete_type, inc->fns[j].name);
						} else {
							append_to_header(state, "%s %s_%s(const Obj *self",
											 inc->fns[j].return_type, complete_type,
											 inc->fns[j].name);
						}
					}

					expand_params(state, &inc->fns[j], NULL);
					append_to_header(state, ");");
				}
				append_to_header(
					state, "static void __attribute__((constructor)) vtable_add_inc_impl_%s_%s() {",
					complete_type, incomplete_type);
				for (u64 j = 0; j < inc->count; j++) {
					if (inc->fns[j].has_impl) {
						append_to_header(
							state,
							"VtableEntry "
							"next_%" PRIu64
							" = {\"%s\", %s_%s}; vtable_add_entry(&%s_Vtable__, next_%" PRIu64 ");",
							j, inc->fns[j].name, incomplete_full_type, inc->fns[j].name,
							complete_type, j);
					} else {
						append_to_header(
							state,
							"VtableEntry next_%" PRIu64
							" = {\"%s\", %s_%s}; vtable_add_entry(&%s_Vtable__, next_%" PRIu64 ");",
							j, inc->fns[j].name, complete_type, inc->fns[j].name, complete_type, j);
					}
				}
				append_to_header(state, "}");
				found = true;
			}
		}

		if (has_impl) {
			TypeInfo ti;
			ti.mi = *(state->cur);
			strcpy(ti.type_name, incomplete_type);
			ti.gen_file_counter = state->gen_file_counter;
			vec_push(state->types, &ti);

			FILE *fp = myfopen(args_file, "a");
			fprintf(fp, "-DType_Expand_%s_=\"\"\n", incomplete_full_type);
			myfclose(fp);
		}

		if (!found) {
			state->has_errors = true;
			token_display_error(tk, "Attempt to complete an unknown incomplete type [%s]",
								incomplete_type);
		}
		state->state = ParserStateImplExpectSemi;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected type name. Found [%s]", tk->token);
	}
}

void proc_ParserStateImplExpectSemi(ParserState *state, Token *tk) {
	if (!strcmp(tk->token, ";")) {
		state->state = ParserStateBeginStatement;
	} else {
		state->state = ParserStateBeginStatement;
		state->has_errors = true;
		token_display_error(tk, "Expected ';'. Found [%s]", tk->token);
	}
}

void file_for(const char *base_dir, const ModuleInfo *self_info, char buf[PATH_MAX]) {
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

void init_parser(const char *args_file) {
	Vec build_fns;
	vec_init(&build_fns, 1, sizeof(IncompleteFn));
	IncompleteFn build;
	build.has_impl = false;
	strcpy(build.name, "build");
	strcpy(build.return_type, "void");
	vec_init(&build.params, 1, sizeof(FnParam));
	FnParam config = {.is_mut = true};
	strcpy(config.type, "__config__");
	strcpy(config.name, "__selfconfig__");
	vec_push(&build.params, &config);
	vec_push(&build_fns, &build);

	add_to_global_incomplete_list("Build", &build_fns, args_file, NULL, NULL);

	Vec drop_fns;
	vec_init(&drop_fns, 1, sizeof(IncompleteFn));
	IncompleteFn drop;
	drop.has_impl = false;
	strcpy(drop.name, "drop");
	strcpy(drop.return_type, "void");
	vec_init(&drop.params, 1, sizeof(FnParam));
	vec_push(&drop_fns, &drop);

	add_to_global_incomplete_list("Drop", &drop_fns, args_file, NULL, NULL);
}

void parse_header(const char *config_dir, const char *base_dir, Vec *modules, Vec *types,
				  const ModuleInfo *self_info) {

	Path args_path;
	path_for(&args_path, base_dir);
	path_push(&args_path, "target");
	path_push(&args_path, "args");
	path_push(&args_path, "args.txt");
	const char *args_file = path_to_string(&args_path);
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
		.has_errors = false,
	};
	gen_file_counter++;

	vec_init(&state.config_types, 3, sizeof(ConfigType));
	vec_init(&state.config_names, 3, sizeof(ConfigName));
	vec_init(&state.ht.names, 3, sizeof(HeaderNameInfo));
	vec_init(&state.ht.types, 3, sizeof(HeaderTypeInfo));
	vec_init(&state.import_list, 3, sizeof(SubModuleInfo));
	vec_init(&state.incomplete_fns, 3, sizeof(IncompleteFn));
	vec_init(&state.super_traits, 3, sizeof(SuperTraitInfo));

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
				proc_ParserStateExpectType(&state, &tk, args_file);
			} else if (state.state == ParserStateExpectName) {
				proc_ParserStateExpectName(&state, &tk);
			} else if (state.state == ParserStateInTypeExpectSemi) {
				proc_ParserStateInTypeExpectSemi(&state, &tk);
			} else if (state.state == ParserStateInImportListExpectModuleName) {
				proc_ParserStateInImportListExpectModuleName(&state, &tk);
			} else if (state.state == ParserStateInImportListExpectSeparator) {
				proc_ParserStateInImportListExpectSeparator(&state, &tk);
			} else if (state.state == ParserStateExpectParenBuildDefn) {
				proc_ParserStateExpectParenBuildDefn(&state, &tk);
			} else if (state.state == ParserStateExpectConfigName) {
				proc_ParserStateExpectConfigName(&state, &tk);
			} else if (state.state == ParserStateExpectConfigComma) {
				proc_ParserStateExpectConfigComma(&state, &tk);
			} else if (state.state == ParserStateExpectConfigType) {
				proc_ParserStateExpectConfigType(&state, &tk);
			} else if (state.state == ParserStateIncompleteName) {
				proc_ParserStateIncompleteName(&state, &tk);
			} else if (state.state == ParserIncompleteExpectBrace) {
				proc_ParserIncompleteExpectBrace(&state, &tk);
			} else if (state.state == ParserStateExpectAt) {
				proc_ParserStateExpectAt(&state, &tk, args_file);
			} else if (state.state == ParserStateExpectIncompleteFnMutOrType) {
				proc_ParserStateExpectIncompleteFnMutOrType(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteFnName) {
				proc_ParserStateExpectIncompleteFnName(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteFnParenStart) {
				proc_ParserStateExpectIncompleteFnParenStart(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteFnType) {
				proc_ParserStateExpectIncompleteFnType(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteFnParamName) {
				proc_ParserStateExpectIncompleteFnParamName(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteFnParamCommaOrEnd) {
				proc_ParserStateExpectIncompleteFnParamCommaOrEnd(&state, &tk);
			} else if (state.state == ParserStateIncompleteExpectSemi) {
				proc_ParserStateIncompleteExpectSemi(&state, &tk);
			} else if (state.state == ParserStateIncompleteExpectSemiOrTypeArrow) {
				proc_ParserStateIncompleteExpectSemiOrTypeArrow(&state, &tk);
			} else if (state.state == ParserStateIncompleteExpectReturnType) {
				proc_ParserStateIncompleteExpectReturnType(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteNameForImpl) {
				proc_ParserStateExpectIncompleteNameForImpl(&state, &tk, args_file);
			} else if (state.state == ParserStateImplExpectSemi) {
				proc_ParserStateImplExpectSemi(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteMutFnName) {
				proc_ParserStateExpectIncompleteMutFnName(&state, &tk);
			} else if (state.state == ParserStateExpectPipe) {
				proc_ParserStateExpectPipe(&state, &tk);
			} else if (state.state == ParserStateExpectArraySize) {
				proc_ParserStateExpectArraySize(&state, &tk);
			} else if (state.state == ParserStateExpectEndArray) {
				proc_ParserStateExpectEndArray(&state, &tk);
			} else if (state.state == ParserStateExpectConfigKeyword) {
				proc_ParserStateExpectConfigKeyword(&state, &tk);
			} else if (state.state == ParserStateExpectIncompleteMutFnNameWithDefaultImpl) {
				proc_ParserStateExpectIncompleteMutFnNameWithDefaultImpl(&state, &tk);
			} else if (state.state == ParserStateExpectPlusOrBrace) {
				proc_ParserStateExpectPlusOrBrace(&state, &tk);
			} else if (state.state == ParserStateExpectSuperTrait) {
				proc_ParserStateExpectSuperTrait(&state, &tk);
			} else if (tk.type == TokenTypePunct) {
				if (!strcmp(tk.token, "}") || !strcmp(tk.token, ";")) {
					state.state = ParserStateBeginStatement;
				} else if (!strcmp(tk.token, "{") && state.state == ParserStateExpectBrace) {
					TypeInfo ti;
					ti.mi = *state.cur;
					strcpy(ti.type_name, state.type_name);
					ti.gen_file_counter = state.gen_file_counter;
					vec_push(types, &ti);
					state.state = ParserStateExpectType;
					state.cur_is_array = false;
				} else if (!strcmp(tk.token, "::") && state.state == ParserStateExpectBrace) {
					// This is an implementation statement
					state.state = ParserStateExpectIncompleteNameForImpl;
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
		fprintf(stderr, "Partial write of header: %s occurred!\n",
				path_to_string(&gen_header_name));

	myfclose(fp);
	free_gen_header(&state);
	vec_cleanup(&state.ht.names);
	vec_cleanup(&state.ht.types);
	vec_cleanup(&state.import_list);
	vec_cleanup(&state.config_types);
	vec_cleanup(&state.config_names);
	vec_cleanup(&state.incomplete_fns);
	vec_cleanup(&state.super_traits);

	lexer_cleanup(&l);

	if (state.has_errors) {
		exit_error("Compilation of file: %s failed!", file);
	}
}
