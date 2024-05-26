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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syn/syn.h>
#include <parser/parser.h>
#include <util/constants.h>

int process_syn_ident(StateMachine *sm, Ident *ident, int debug_flags) {
	if(sm->state == StateExpectClassName) {
		if(sm->class_count == 0) {
			sm->class_array = malloc(sizeof(Class));
			sm->class_array[sm->class_count].fn_count = 0;
			sm->class_count += 1;
		} else {
			sm->class_count += 1;
			sm->class_array = realloc(sm->class_array, sm->class_count * sizeof(Class));
			sm->class_array[sm->class_count-1].fn_count = 0;
		}	
		if(sm->class_array == NULL) {
                        fputs("Error: could not allocate memory in process_syn_ident\n", stderr);
                	return -1;
                }
		int ident_len = strlen(ident->value);
		printf("copy classname: %s\n", ident->value);
		sm->class_array[sm->class_count-1].name = malloc(sizeof(char) * (ident_len + 1)); 
		sm->class_array[sm->class_count-1].impl_name = NULL;
		strcpy(sm->class_array[sm->class_count-1].name, ident->value);
		sm->state = StateAfterClassName;
	} else if(sm->state == StateExpectImplName) {
		printf("ident->value='%s'\n", ident->value);
		sm->class_array[sm->class_count-1].impl_name = malloc(sizeof(char) * (strlen(ident->value)+ 1));
		strcpy(sm->class_array[sm->class_count-1].impl_name, ident->value);
		sm->state = StateExpectClassBrace;
	} else if(sm->state == StateExpectReturnTypeOrAttribute) {
		printf("got a return type: %s\n", ident->value);
		sm->state = StateExpectFnName;

/*
if(sm->class_count == 0) {
                        sm->class_array = malloc(sizeof(Class));
                        sm->class_array[sm->class_count].fn_count = 0;
                        sm->class_count += 1;
                } else {
                        sm->class_count += 1;
                        sm->class_array = realloc(sm->class_array, sm->class_count * sizeof(Class));
                        sm->class_array[sm->class_count-1].fn_count = 0;
                }  

 *
 */
		if(sm->class_array[sm->class_count-1].fn_count == 0) {
			sm->class_array[sm->class_count-1].fn_array = malloc(sizeof(Fn));
			sm->class_array[sm->class_count-1].fn_count += 1;
		} else {
			sm->class_array[sm->class_count-1].fn_count += 1;
			sm->class_array[sm->class_count-1].fn_array = realloc(
				sm->class_array[sm->class_count-1].fn_array,
				sm->class_array[sm->class_count-1].fn_count * sizeof(Fn)
			);
		}
		int cur = (sm->class_array[sm->class_count-1].fn_count) - 1;
		printf("cur fn = %i\n", cur);
		sm->class_array[sm->class_count-1].fn_array[cur].name = malloc(sizeof(char) * (strlen(ident->value) + 1));
		strcpy(sm->class_array[sm->class_count-1].fn_array[cur].name, ident->value);
	} else if(sm->state == StateExpectFnName) {
		printf("got a fn name: %s\n", ident->value);
		int cur = (sm->class_array[sm->class_count-1].fn_count) - 1;
		sm->class_array[sm->class_count-1].fn_array[cur].ret_type = malloc(sizeof(char) * (strlen(ident->value) + 1));
                strcpy(sm->class_array[sm->class_count-1].fn_array[cur].ret_type, ident->value);
		sm->class_array[sm->class_count-1].fn_array[cur].is_implemented = 1;
		sm->state = StateExpectParameterList;
	} else {
		
	}

	return 0;
}

int process_syn_punct(StateMachine *sm, Punct *punct, int debug_flags) {
	if(sm->state == StateAfterClassName) {
		// it's an implementation block
		if(punct->ch == ':' && punct->second_ch == ':' && punct->third_ch == 0) {
			printf("impl block\n");
			sm->state = StateExpectImplName;
		} else {
		}
	} else if(sm->state == StateExpectReturnTypeOrAttribute) {
		// doc
		if(punct->ch == '#') {
			sm->state = StateExpectAttributeBracket;
		} else {
		}

	} else if(sm->state == StateExpectClassName) {
		 if(punct->ch == '#') {
                        sm->state = StateExpectAttributeBracket;
                } else {
                }
	} else if(sm->state == StateExpectFnBlockOrSemi) {
		if(punct->ch == ';' && sm->scope == 1) {
			sm->state = StateExpectReturnTypeOrAttribute;
			int cur = (sm->class_array[sm->class_count-1].fn_count) - 1;
			sm->class_array[sm->class_count-1].fn_array[cur].is_implemented = 0;
		}
	}
	return 0;
}

int process_syn_literal(StateMachine *sm, Literal *literal, int debug_flags) {
	return 0;
}

int process_syn_begin_group(StateMachine *sm, Delimiter delimiter, int debug_flags) {
	if(delimiter == Brace) {
		sm->scope += 1;
	}
	if(sm->state == StateAfterClassName || sm->state == StateExpectClassBrace) {
		if(delimiter == Brace) {
			sm->state = StateExpectReturnTypeOrAttribute;
		} else {
		}
	} else if(sm->state == StateExpectAttributeBracket) {
		if(delimiter == Bracket) {
			sm->state = StateInAttribute;
		} else {
		}
	} else if(sm->state == StateExpectParameterList) {
		sm->in_param_list = TRUE;

	} else {
	}
	return 0;
}

int process_syn_end_group(StateMachine *sm, Delimiter delimiter, int debug_flags) {
	if(delimiter == Brace) {
		sm->state = StateExpectClassName;
		sm->scope -= 1;
		printf("end scope. Scope is now %i\n", sm->scope);
		if(sm->scope < 0) {
			// err
		} else if(sm->scope == 1) {
			sm->state = StateExpectReturnTypeOrAttribute;
		}
	} else if(delimiter == Bracket) {
		printf("got end bracket scope = %i, in_param_list = %i\n", sm->scope, sm->in_param_list);
		if(sm->scope == 0) {
			sm->state = StateExpectClassName;
		} else if(sm->scope == 1 && !sm->in_param_list) {
			printf("expect ret or attr\n");
			sm->state = StateExpectReturnTypeOrAttribute;
		}
	} else if(delimiter == Parenthesis) {
		if(sm->scope == 1) {
                	sm->in_param_list = FALSE;
                }
		if(sm->state == StateExpectParameterList) {
			sm->state = StateExpectFnBlockOrSemi;
		}
	}
	return 0;
}

int process_token_tree(StateMachine *sm, TokenTree *tree, int debug_flags) {
	if(tree->token_type == IdentType) {
		if((debug_flags & DEBUG_FLAG_PRINT_TOKENS) != 0)
			printf("ident[%s]\n", tree->ident->value);
		process_syn_ident(sm, tree->ident, debug_flags);
	} else if(tree->token_type == PunctType) {
		char punct[4];
		punct[0] = tree->punct->ch;
		if(tree->punct->second_ch != 0) {
			punct[1] = tree->punct->second_ch;
			if(tree->punct->third_ch != 0) {
				punct[2] = tree->punct->third_ch;
				punct[3] = 0;
			} else {
				punct[2] = 0;
			}
		} else {
			punct[1] = 0;
		}
		if((debug_flags & DEBUG_FLAG_PRINT_TOKENS) != 0)
			printf("punct[%s]\n", punct);
		if(process_syn_punct(sm, tree->punct, debug_flags)) {
			return -1;
		}
	} else if(tree->token_type == LiteralType) {
		if((debug_flags & DEBUG_FLAG_PRINT_TOKENS) != 0)
			printf("literal[%s]\n", tree->literal->literal);
		process_syn_literal(sm, tree->literal, debug_flags);
	} else { // GroupType
		TokenTree group_tree;
		int v;
		int err = 0;
		if((debug_flags & DEBUG_FLAG_PRINT_TOKENS) != 0)
			printf("group[%u]\n", tree->group->delimiter);
		process_syn_begin_group(sm, tree->group->delimiter, debug_flags);
		while(TRUE) {
			v = next_token(tree->group->strm, &group_tree);
			if(v == 0)
				break;
			else if(v == 2) {
				err = 1;
				fputs("parse error! Cannot continue!\n", stderr);
				break;
			}

			process_token_tree(sm, &group_tree, debug_flags);

			free_token_tree(&group_tree);
		}
		if(err) {
			return -1;
		}
		if((debug_flags & DEBUG_FLAG_PRINT_TOKENS) != 0)
			printf("end group[%u]\n", tree->group->delimiter);
		process_syn_end_group(sm, tree->group->delimiter, debug_flags);
	}
	return 0;
}

int build_state(char *file, StateMachine *sm, int debug_flags) {
	sm->state = StateExpectClassName;
	sm->class_array = NULL;
	sm->class_count = 0;
	sm->scope = 0;
	sm->in_param_list = FALSE;

	if(parse(file, &sm->strm, debug_flags) != 0) {
		fputs("Error parsing file\n", stderr);
		return -1;
	}

	TokenTree tree;
	int v;
	while(TRUE) {
		v = next_token(&sm->strm, &tree);
		if(v == 0)
			break;
		else if(v == 2) {
			fputs("parse error! Cannot continue!\n", stderr);
			break;
		}

		if(process_token_tree(sm, &tree, debug_flags) != 0) {
			break;
		}

		free_token_tree(&tree);
	}

	free_token_stream(&sm->strm);


	return 0;
}
