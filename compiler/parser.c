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
#include <core/colors.h>
#include <core/resources.h>
#include <errno.h>
#include <lexer/lexer.h>
#include <string.h>

int parser_copy_import_list(ParserImportList *dst, ParserImportList *src) {
	if (src->count == 0)
		return 0;

	dst->list = mymalloc(sizeof(ParserImport) * src->count);
	if (dst->list == NULL)
		return -1;

	for (int i = 0; i < src->count; i++) {
		dst->list[i].name =
		    mymalloc(sizeof(char) * (strlen(src->list[i].name) + 1));
		strcpy(dst->list[i].name, src->list[i].name);
	}
	dst->count = src->count;
	return 0;
}

int parser_pkg_add_class(ParserPackage *pkg, ParserClass *class,
			 ParserImportList *list) {
	if (pkg->class_count == 0) {
		pkg->classes = mymalloc(sizeof(ParserClass));
		if (pkg->classes == NULL)
			return -1;
	} else {
		void *tmp = myrealloc(pkg->classes, sizeof(ParserClass) *
							(pkg->class_count + 1));
		if (tmp == NULL)
			return -1;
		pkg->classes = tmp;
	}

	memcpy(&pkg->classes[pkg->class_count], class, sizeof(ParserClass));
	if (parser_copy_import_list(&pkg->classes[pkg->class_count].imports,
				    list))
		return -1;

	pkg->class_count += 1;
	return 0;
}

int parser_init_class(ParserClass *class, char *class_name) {
	class->name = mymalloc(sizeof(char) * (strlen(class_name) + 1));
	if (class->name == NULL) {
		return -1;
	}
	strcpy(class->name, class_name);
	return 0;
}

void parser_class_cleanup(ParserClass *class) {
	if (class->name) {
		myfree(class->name);
		class->name = NULL;
	}

	for (int i = 0; i < class->imports.count; i++) {
		myfree(class->imports.list[i].name);
	}
	if (class->imports.count > 0)
		myfree(class->imports.list);
}

int parser_add_import(ParserImportList *list, char *import_name) {
	if (list->count == 0) {
		list->list = mymalloc(sizeof(ParserImport));
		if (list->list == NULL)
			return -1;
	} else {
		void *tmp = myrealloc(list->list,
				      sizeof(ParserImport) * (list->count + 1));
		list->list = tmp;
	}

	list->list[list->count].name =
	    mymalloc(sizeof(char) * (strlen(import_name) + 1));
	if (list->list[list->count].name == NULL) {
		return -1;
	}
	strcpy(list->list[list->count].name, import_name);
	list->count += 1;

	return 0;
}

int parser_process_import(ParserPackage *pkg, Lexer *lexer,
			  ParserImportList *list) {
	int ret = 0;
	Token token;
	char *import_name = NULL;
	while (true) {
		LexerState ret = lexer_next_token(lexer, &token);
		if (ret == LexerStateComplete)
			break;
		if (ret == LexerStateErr) {
			fprintf(stderr, "Lexer return error");
			perror("lexer");
			ret = -1;
			break;
		}
		if (token.type == TokenTypePunct) {
			if (!strcmp(token.token, ";")) {
				token_cleanup(&token);
				break;
			}
		}

		// temporarily just take last token as the import name
		// TODO: additional parsing required
		if (import_name)
			myfree(import_name);
		import_name =
		    mymalloc(sizeof(char) * (strlen(token.token) + 1));

		strcpy(import_name, token.token);

		token_cleanup(&token);
	}

	if (import_name == NULL) {
		// TODO: handle error
	} else {
		parser_add_import(list, import_name);
		myfree(import_name);
	}

	return ret;
}

int parser_process_class_base_plus_at(Lexer *lexer, ParserClass *class) {
	int ret = 0;
	int scope = 0;
	Token token;

	while (ret == 0) {
		LexerState ret = lexer_next_token(lexer, &token);
		if (ret == LexerStateComplete)
			break;
		if (ret == LexerStateErr) {
			fprintf(stderr, "Lexer return error");
			perror("lexer");
			ret = -1;
			break;
		}

		if (token.type == TokenTypePunct) {
			if (!strcmp(token.token, "{")) {
				scope += 1;
			} else if (!strcmp(token.token, "}")) {
				scope -= 1;
				if (scope == 0) {
					token_cleanup(&token);
					break;
				}
			}
		}
		token_cleanup(&token);
	}
	return ret;
}

int parser_process_class_base_plus_ident(Lexer *lexer, ParserClass *class) {
	int ret = 0;
	int scope = 0;
	Token token;

	while (ret == 0) {
		LexerState ret = lexer_next_token(lexer, &token);
		if (ret == LexerStateComplete)
			break;
		if (ret == LexerStateErr) {
			fprintf(stderr, "Lexer return error");
			perror("lexer");
			ret = -1;
			break;
		}

		if (token.type == TokenTypePunct) {
			if (!strcmp(token.token, "{")) {
				scope += 1;
			} else if (!strcmp(token.token, "}")) {
				scope -= 1;
				if (scope == 0) {
					token_cleanup(&token);
					break;
				}
			} else if (!strcmp(token.token, ";") && scope == 0) {
				token_cleanup(&token);
				break;
			}
		}
		token_cleanup(&token);
	}

	return ret;
}

int parser_process_class_inner(Lexer *lexer, ParserClass *class) {
	int ret = 0;
	Token token;
	while (ret == 0) {
		LexerState ret = lexer_next_token(lexer, &token);
		if (ret == LexerStateComplete)
			break;
		if (ret == LexerStateErr) {
			fprintf(stderr, "Lexer return error");
			perror("lexer");
			ret = -1;
			break;
		}

		if (token.type == TokenTypePunct) {
			if (!strcmp(token.token, "@")) {
				ret = parser_process_class_base_plus_at(lexer,
									class);
			} else if (!strcmp(token.token, "}")) {
				// end class
				token_cleanup(&token);
				break;
			} else {
				display_error(
				    &token,
				    "Unexpected token. Expected either '@', "
				    "function name, or "
				    "type. Found '%s'.",
				    token.token);
				ret = -1;
			}
		} else if (token.type == TokenTypeIdent) {
			ret =
			    parser_process_class_base_plus_ident(lexer, class);
		} else {
			display_error(&token,
				      "Unexpected token. Expected either '@', "
				      "function name, or "
				      "type. Found '%s'.",
				      token.token);
			ret = -1;
		}
		token_cleanup(&token);
	}
	return ret;
}

int parser_process_class(ParserPackage *pkg, Lexer *lexer, char *class_name,
			 ParserImportList *import_list) {
	int ret = 0;
	Token token;
	ParserClass nclass;
	if (parser_init_class(&nclass, class_name)) {
		return -1;
	}

	if (parser_pkg_add_class(pkg, &nclass, import_list)) {
		return -1;
	}

	while (ret == 0) {
		LexerState ret = lexer_next_token(lexer, &token);
		if (ret == LexerStateComplete)
			break;
		if (ret == LexerStateErr) {
			fprintf(stderr, "Lexer return error");
			perror("lexer");
			ret = -1;
			break;
		}
		if (token.type == TokenTypePunct) {
			if (!strcmp(token.token, "{")) {
				parser_process_class_inner(lexer, &nclass);
			} else
				display_error(&token,
					      "Unexpected token. Expected '{'. "
					      "Found '%s'.",
					      token.token);
		} else {
			display_error(&token,
				      "Unexpected token. Expected '{'. "
				      "Found '%s'.",
				      token.token);
			ret = -1;
		}

		token_cleanup(&token);
	}

	return ret;
}

int parser_import_list_init(ParserImportList *list) {
	list->count = 0;
	list->list = NULL;
	return 0;
}

void parser_import_list_cleanup(ParserImportList *list) {
	if (list->list) {
		for (int i = 0; i < list->count; i++) {
			myfree(list->list[i].name);
		}
		myfree(list->list);
		list->list = NULL;
		list->count = 0;
	}
}

int parser_parse_file(ParserPackage *pkg, char *file) {
	int ret = 0;
	Lexer lexer;
	Token token;
	lexer_init(&lexer, file);
	ParserImportList import_list;
	parser_import_list_init(&import_list);

	while (true) {
		LexerState ret = lexer_next_token(&lexer, &token);

		if (ret == LexerStateComplete)
			break;
		if (ret == LexerStateErr) {
			fprintf(stderr, "Lexer return error");
			perror("lexer");
			ret = -1;
			break;
		}

		if (token.type == TokenTypePunct && !strcmp(token.token, "@")) {
			parser_process_import(pkg, &lexer, &import_list);
		} else if (token.type == TokenTypeIdent) {
			parser_process_class(pkg, &lexer, token.token,
					     &import_list);
		} else {
			// handle error
			display_error(
			    &token,
			    "Unexpected token. Expected either '@' or "
			    "class name. Found '%s'.",
			    token.token);
			ret = -1;
			token_cleanup(&token);
			break;
		}

		token_cleanup(&token);
	}

	parser_import_list_cleanup(&import_list);
	lexer_cleanup(&lexer);
	return ret;
}

int parser_pkg_init(ParserPackage *pkg) {
	if (pkg == NULL) {
		errno = EINVAL;
		return -1;
	}

	pkg->class_count = 0;
	pkg->sub_package_count = 0;

	return 0;
}

void parser_pkg_cleanup(ParserPackage *pkg) {
	for (int i = 0; i < pkg->class_count; i++) {
		parser_class_cleanup(&pkg->classes[i]);
	}
	if (pkg->class_count > 0)
		myfree(pkg->classes);
}
