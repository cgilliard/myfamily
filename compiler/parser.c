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

void *parser_mymalloc_and_copy(char *str) {
	char *ret = mymalloc(sizeof(char) * (1 + strlen(str)));
	if (ret)
		strcpy(ret, str);
	return ret;
}

void *parser_resize_list(void *ptr, int *cur_size, usize size) {
	void *ret;
	if (*cur_size == 0) {
		ret = mymalloc(size);
	} else {
		ret = myrealloc(ptr, size * (*cur_size + 1));
	}
	if (ret)
		*cur_size += 1;

	return ret;
}

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

int parser_init_class(ParserClass *class, char *class_name) {
	class->name = mymalloc(sizeof(char) * (strlen(class_name) + 1));
	if (class->name == NULL) {
		return -1;
	}
	strcpy(class->name, class_name);

	class->function_count = 0;
	class->field_count = 0;
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
	for (int i = 0; i < class->function_count; i++) {
		myfree(class->functions[i].name);
		myfree(class->functions[i].return_type);
	}
	if (class->function_count) {
		class->function_count = 0;
		myfree(class->functions);
	}

	for (int i = 0; i < class->field_count; i++) {
		myfree(class->fields[i].name);
		myfree(class->fields[i].type);
	}
	if (class->field_count) {
		class->field_count = 0;
		myfree(class->fields);
	}
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

int parser_process_param_list(Lexer *lexer, ParserClass *class) {
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

		if (!strcmp(token.token, "{")) {
			scope += 1;
		} else if (!strcmp(token.token, "}")) {
			scope -= 1;
			if (scope == 0) {
				token_cleanup(&token);
				break;
			}
		}
		token_cleanup(&token);
	}
}

int parser_process_idents(char *ident1, char *ident2, ParserClass *class) {
	int ret = 0;
	if (ident1 != NULL) {
		void *retptr =
		    parser_resize_list(class->functions, &class->function_count,
				       sizeof(ParserFunction));
		if (retptr)
			class->functions = retptr;
		else
			ret = -1;

		ParserFunction *fns = class->functions;
		int index = class->function_count - 1;

		if (ret == 0) {
			fns[index].name = NULL;
			fns[index].return_type = NULL;
			if (ident2 != NULL) {
				fns[index].name =
				    parser_mymalloc_and_copy(ident2);
				if (fns[index].name == NULL)
					ret = -1;
				else {
					fns[index].return_type =
					    parser_mymalloc_and_copy(ident1);
					if (fns[index].return_type == NULL)
						ret = -1;
				}
			} else {
				fns[index].name =
				    parser_mymalloc_and_copy(ident1);
				if (fns[index].name == NULL)
					ret = -1;
				else {
					fns[index].return_type =
					    parser_mymalloc_and_copy("void");
					if (fns[index].return_type == NULL)
						ret = -1;
				}
			}

			if (ret != 0) {
				class->function_count -= 1;
				if (fns[index].name)
					myfree(fns[index].name);
				if (fns[index].return_type)
					myfree(fns[index].return_type);
			}
		}
	}

	if (ident1 != NULL) {
		myfree(ident1);
	}
	if (ident2 != NULL) {
		myfree(ident2);
	}

	return ret;
}

int parser_process_class_base_plus_at(Lexer *lexer, ParserClass *class) {
	// we know this is a function declaration
	int ret = 0;
	int scope = 0;
	bool is_mut = false;
	int counter = 0;
	char *ident1 = NULL;
	char *ident2 = NULL;
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
			if (!strcmp(token.token, "(")) {
				// begin the param list.
				if (ident1 == NULL) {
					display_error(
					    &token, "Invalid function "
						    "declaration. No "
						    "function name specified.");
				}
				ret = parser_process_param_list(lexer, class);
				token_cleanup(&token);
				break;
			} else {
				display_error(&token,
					      "Unexpected token."
					      "Expected '('. Found '%s'",
					      token.token);
			}
		} else if (token.type == TokenTypeIdent) {
			if (!strcmp(token.token, "mut")) {
				if (counter != 0) {
					display_error(&token,
						      "Unexpected token. 'mut' "
						      "must be declared "
						      "immediately "
						      "after the @ symbol.");
				}
				// mutable function
				is_mut = true;
			} else {
				// this is either the function name or
				// return type
				if (ident1 == NULL) {
					ident1 = parser_mymalloc_and_copy(
					    token.token);
					if (ident1 == NULL)
						ret = -1;
				} else if (ident2 == NULL) {
					ident2 = parser_mymalloc_and_copy(
					    token.token);
					if (ident2 == NULL)
						ret = -1;
				} else {
					display_error(
					    &token,
					    "Unexpected token."
					    "Expected '('. Found '%s'",
					    token.token);
				}
			}
		} else {
			display_error(&token,
				      "Unexpected token."
				      "Expected '('. Found '%s'",
				      token.token);
		}
		counter += 1;
		token_cleanup(&token);
	}

	if (ret == 0)
		return parser_process_idents(ident1, ident2, class);
	else
		return ret;
}

int parser_process_class_base_plus_ident(Lexer *lexer, ParserClass *class,
					 char *ident1) {
	int ret = 0;
	Token token;
	char *ident2 = NULL;
	char *ident3 = NULL;
	bool is_static = !strcmp(ident1, "static");
	bool is_fn = false;

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
			if (!strcmp(token.token, "(")) {
				if (is_static && ident2 == NULL) {
					display_error(
					    &token, "Invalid function "
						    "declaration. No "
						    "function name specified.");
					ret = -1;
				} else {
					is_fn = true;
					ret = parser_process_param_list(lexer,
									class);
				}
				token_cleanup(&token);
				break;
			} else if (!strcmp(token.token, ";")) {
				// type decl
				if (is_static) {
					display_error(&token,
						      "Invalid field "
						      "declaration. static "
						      "is only allowed for "
						      "functions.");
					ret = -1;
				} else if (ident2 == NULL) {
					display_error(&token,
						      "Expected field "
						      "name. Found ';'.");
					ret = -1;
				}
				token_cleanup(&token);
				break;
			} else {
				token_cleanup(&token);
				break;
			}
		} else if (token.type == TokenTypeIdent) {
			if (ident2 == NULL) {
				ident2 = mymalloc(sizeof(char) *
						  (strlen(token.token) + 1));
				strcpy(ident2, token.token);
			} else if (ident3 == NULL) {
				if (!is_static) {
					display_error(&token,
						      "Unexpected token. "
						      "Expected '(', "
						      "Found '%s'.",
						      token.token);
					ret = -1;
				} else {
					ident3 =
					    mymalloc(sizeof(char) *
						     (strlen(token.token) + 1));
					strcpy(ident3, token.token);
				}
			}
		}
		token_cleanup(&token);
	}

	if (is_fn) {
		if (class->function_count == 0) {
			class->functions = mymalloc(sizeof(ParserFunction));
			if (class->functions == NULL) {
				ret = -1;
			}
		} else {
			void *tmp = myrealloc(class->functions,
					      sizeof(ParserFunction) *
						  (class->function_count + 1));
			if (tmp == NULL)
				ret = -1;
			else
				class->functions = tmp;
		}

		if (ret == 0) {
			class->function_count += 1;
			if (ident2 == NULL) {
				class->functions[class->function_count - 1]
				    .name = mymalloc(sizeof(char) *
						     (1 + strlen(ident1)));
				if (class->functions[class->function_count - 1]
					.name == NULL) {
					ret = -1;
				} else
					strcpy(
					    class
						->functions
						    [class->function_count - 1]
						.name,
					    ident1);

				class->functions[class->function_count - 1]
				    .return_type = mymalloc(sizeof(char) * 5);
				if (class->functions[class->function_count - 1]
					.return_type == NULL)
					ret = -1;
				else
					strcpy(
					    class
						->functions
						    [class->function_count - 1]
						.return_type,
					    "void");
			} else if (ident3 == NULL) {
				class->functions[class->function_count - 1]
				    .name = mymalloc(sizeof(char) *
						     (1 + strlen(ident2)));
				if (class->functions[class->function_count - 1]
					.name == NULL) {
					ret = -1;
				} else
					strcpy(
					    class
						->functions
						    [class->function_count - 1]
						.name,
					    ident2);
				if (is_static) {
					class
					    ->functions[class->function_count -
							1]
					    .return_type =
					    mymalloc(sizeof(char) * 5);
					if (class
						->functions
						    [class->function_count - 1]
						.return_type == NULL)
						ret = -1;
					else
						strcpy(
						    class
							->functions
							    [class
								 ->function_count -
							     1]
							.return_type,
						    "void");
				} else {
					class
					    ->functions[class->function_count -
							1]
					    .return_type =
					    mymalloc(sizeof(char) *
						     (1 + strlen(ident1)));
					if (class
						->functions
						    [class->function_count - 1]
						.return_type == NULL) {
						ret = -1;
					} else
						strcpy(
						    class
							->functions
							    [class
								 ->function_count -
							     1]
							.return_type,
						    ident1);
				}
			} else {
				class->functions[class->function_count - 1]
				    .name = mymalloc(sizeof(char) *
						     (1 + strlen(ident3)));
				if (class->functions[class->function_count - 1]
					.name == NULL) {
					ret = -1;
				} else
					strcpy(
					    class
						->functions
						    [class->function_count - 1]
						.name,
					    ident3);

				class->functions[class->function_count - 1]
				    .return_type = mymalloc(
				    sizeof(char) * (1 + strlen(ident2)));
				if (class->functions[class->function_count - 1]
					.return_type == NULL) {
					ret = -1;
				} else
					strcpy(
					    class
						->functions
						    [class->function_count - 1]
						.return_type,
					    ident2);
			}
		}
	} else if (ret == 0) {
		// it's a type declaration
		if (class->field_count == 0) {
			class->fields = mymalloc(sizeof(ParserField));
			if (class->fields == NULL)
				ret = -1;
		} else {
			void *tmp = myrealloc(class->fields,
					      sizeof(ParserField) *
						  (1 + class->field_count));
			if (tmp == NULL)
				ret = -1;
			else {
				class->fields = tmp;
			}
		}

		if (ret == 0) {
			class->field_count += 1;
			class->fields[class->field_count - 1].name =
			    mymalloc(sizeof(char) * (1 + strlen(ident2)));
			if (class->fields[class->field_count - 1].name == NULL)
				ret = -1;
			else {
				strcpy(
				    class->fields[class->field_count - 1].name,
				    ident2);
				class->fields[class->field_count - 1].type =
				    mymalloc(sizeof(char) *
					     (1 + strlen(ident1)));
				if (class->fields[class->field_count - 1]
					.type != NULL) {
					strcpy(
					    class->fields[class->field_count -
							  1]
						.type,
					    ident1);
				} else
					ret = -1;
			}
		}
	}

	if (ident2 != NULL)
		myfree(ident2);
	if (ident3 != NULL)
		myfree(ident3);

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
				display_error(&token,
					      "Unexpected token. "
					      "Expected either '@', "
					      "function name, or "
					      "type. Found '%s'.",
					      token.token);
				ret = -1;
			}
		} else if (token.type == TokenTypeIdent) {
			ret = parser_process_class_base_plus_ident(lexer, class,
								   token.token);
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

	ParserClass *class;

	if (pkg->class_count == 0) {
		pkg->classes = mymalloc(sizeof(ParserClass));
		if (pkg->classes == NULL)
			return -1;
	} else {
		void *tmp = myrealloc(pkg->classes, sizeof(ParserClass) *
							(pkg->class_count + 1));
		if (tmp == NULL) {
			return -1;
		}
		pkg->classes = tmp;
	}
	class = &pkg->classes[pkg->class_count];
	pkg->class_count += 1;
	parser_copy_import_list(&class->imports, import_list);

	if (parser_init_class(class, class_name)) {
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
				parser_process_class_inner(lexer, class);
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
