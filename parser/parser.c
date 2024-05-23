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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int parse(char *file_name, TokenStream *strm) {
	FILE *file = fopen(file_name, "r");
	if (file == NULL) {
		return -1;
	}
    	long long file_size = 0; // File size to zero before byte check
       	fseek(file, 0L, SEEK_END); // seek to end
       	file_size = ftell(file); // find size of file by checking end
       	rewind(file); // back to the beginning for reading
    	char buffer[16]; // We will read the file 16 bytes at a time
    	strm->bytes = malloc(file_size + 1);
	if (strm->bytes == NULL) {
		fputs("Could not allocate enough memory", stderr);
		fclose(file);
		return -1;
    	}

    	while (fgets(buffer, 16, file) != NULL) {
        	strncat(strm->bytes, buffer, 16);
    	}

	fclose(file);
	strm->pos = 0;
	strm->line_num = 1;
	strm->start_doc = -1;
	strm->end_doc = -1;
	strm->len = file_size;
	strm->file_path = malloc(sizeof(char)*(strlen(file_name) + 1));
	strcpy(strm->file_path, file_name);
	strm->parent = NULL;
	strm->pos_offset = 0;

	return 0;
}

int is_white_space(char ch) {
	if (ch == '\n' || ch == '\t' || ch == '\r' || ch == '\v' || ch == '\f' || ch == ' ')
		return 1;
	else 
		return 0;
}

int is_ident_start(char ch) {
	if ((ch <= 'Z' && ch >= 'A') || (ch <= 'z' && ch >= 'a') || ch == '_') {
		return 1;
	} else {
		return 0;
	}
}

int is_ident_secondary(char ch) {
	if (is_ident_start(ch) || (ch <= '9' && ch >= '0')) {
        	return 1;
        } else {
                return 0;
        }
}

int is_joint_possible(char ch) {
	if (ch == '.' || ch == '=' || ch == '/' || ch == '+' || ch == '&' || ch == '<' ||
		ch == '-' || ch == '%' || ch == '^' || ch == '*' || ch == '>' || ch == '|') {
		return 1;
	} else {
		return 0;
	}
}

int display_span(Span *span, ErrorLevel level, char *message) {
	TokenStream *file_stream = span->strm;
	while(file_stream->parent != NULL) {
		file_stream = file_stream->parent;
	}
	int start = span->offset;
	int end = span->offset;
	int tab_count = 0;
	while(1) {
		if(start == 0 || file_stream->bytes[start-1] == '\n') {
			break;
		}
		if(file_stream->bytes[start-1] == '\t')
                        tab_count += 1;
		start -= 1;
	}
	while(1) {
		if(end >= file_stream->len || file_stream->bytes[end] == '\n') {
			break;
		}
		end += 1;
	}

	int span_len = end - start;
	char *display_str = malloc(sizeof(char) * (span_len + 1));
	memcpy(display_str, file_stream->bytes + start, span_len);
	display_str[span_len] = 0;
	if(level == Error) { // ERROR
		printf(
			"\e[1m%s:%i:%i:\e[m " ANSI_COLOR_RED "error:" ANSI_COLOR_RESET " %s\n%s\n",
			file_stream->file_path,
			span->offset,
			span->line_num,
			message,
			display_str
		);
	} else { // WARNING
		printf(         
                        "\e[1m%s:%i:%i:\e[m " ANSI_COLOR_MAGENTA "warning:" ANSI_COLOR_RESET " %s\n%s\n",
                        file_stream->file_path,
                        span->offset,   
                        span->line_num,
                        message,        
                        display_str
                );
	}
	char *spacing_and_up_arrow = malloc(sizeof(char) * (span->offset - start) + 2);

	int cur = 0;
	for(int i=start; i<span->offset; i++) {
		if(file_stream->bytes[i] == '\t')
			spacing_and_up_arrow[cur] = '\t';
		else
			spacing_and_up_arrow[cur] = ' ';
		cur += 1;
	}
	spacing_and_up_arrow[cur] = '^';
	spacing_and_up_arrow[cur+1] = 0;

	//printf("tab_count=%i,other=%i,first='%c',span_offset=%i,cur=%i,start=%i\n", tab_count, (span->offset-start)-tab_count, file_stream->bytes[start], span->offset, cur, start);

	printf(ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, spacing_and_up_arrow);
	//printf("012345678901234567890\n");

	free(display_str);

	return 0;
}

int next_token(TokenStream *strm, TokenTree *next) {
	int len = strm->len;
	int ret;

	if(strm->start_doc > 0) {
		next->token_type = GroupType;
                next->group = malloc(sizeof(Group));
                next->group->delimiter = Bracket;
                next->group->strm = malloc(sizeof(TokenStream));
		next->group->strm->parent = strm;
		next->group->strm->pos_offset = strm->start_doc + strm->pos_offset;
		next->group->strm->file_path = NULL;
                next->group->strm->pos = 0;
		next->group->strm->line_num = strm->line_num;
                int group_len = (strm->end_doc - strm->start_doc) + 6;
                next->group->strm->bytes = malloc(sizeof(char) * group_len);
		next->group->strm->bytes[0] = 'd';
		next->group->strm->bytes[1] = 'o';
		next->group->strm->bytes[2] = 'c';
		next->group->strm->bytes[3] = '=';
		next->group->strm->bytes[4] = '\"';
                memcpy(next->group->strm->bytes + 5, strm->bytes + strm->start_doc, group_len - 6);
		next->group->strm->bytes[group_len-1] = '\"';
                next->group->strm->len = group_len;

		next->span.line_num = strm->line_num;
		//next->span.offset = strm->start_doc + strm->pos_offset;
		next->span.offset = strm->start_doc + strm->pos_offset;
		next->span.strm = strm;

		strm->start_doc = -1;
		strm->end_doc = -1;
		return 1;
	}

	if(strm->pos >= len) {
                ret = 0;
        } else {
                ret = 1;
        }

	char next_token[len+1];

	int itt = 0;

	while(1) {
		// first loop until we're not in white space
		while(strm->pos < len) {
			if (!is_white_space(strm->bytes[strm->pos])) {
				break;
			}
			if(strm->bytes[strm->pos] == '\n'){
				strm->line_num += 1;
			}
			strm->pos += 1;
		}

		// next check if we're at the begining of a comment
		if (strm->pos < len-1 && strm->bytes[strm->pos] == '/' &&
			(strm->bytes[strm->pos+1] == '/' ||
			 strm->bytes[strm->pos+1] == '*')) {
			// we're in a comment see which kind
			if(strm->bytes[strm->pos+1] == '/') {
				if(strm->pos+4<len) {
					// this is a doc comment
					if(strm->bytes[strm->pos+2] == '/' && strm->bytes[strm->pos+3] == ' ') {
						strm->start_doc = strm->pos+4;
						strm->end_doc = strm->pos;
					}
				}
				while(strm->pos < len && strm->bytes[strm->pos] != '\n') {
					if(strm->start_doc > 0) {
						strm->end_doc += 1;
					}
					if(strm->bytes[strm->pos] == '\n'){
                                		strm->line_num += 1;
                        		}
					strm->pos += 1;
				}

				if(strm->start_doc > 0) {
					next->token_type = PunctType;
                        		next->punct = malloc(sizeof(Punct));
                        		next->punct->ch = '#';
                        		next->punct->second_ch = 0;
                        		next->punct->third_ch = 0;
					next->span.strm = strm;
					next->span.line_num = strm->line_num;
        				next->span.offset = strm->pos + strm->pos_offset;
					return 1;
				}
			} else {
				while(strm->pos < len && (strm->bytes[strm->pos-1] != '*' || strm->bytes[strm->pos] != '/')) {
					if(strm->bytes[strm->pos] == '\n'){
                                		strm->line_num += 1;
                        		}
                                	strm->pos += 1;
                        	}
				if(strm->bytes[strm->pos] == '\n'){
                                	strm->line_num += 1;
                        	}
				strm->pos += 1;
			}

		} else {
			break;
		}
	}

	next->span.line_num = strm->line_num;
	next->span.offset = strm->pos + strm->pos_offset;
	next->span.strm = strm;

	// if we're not an ident start we must be another type
	if (strm->pos < len && !is_ident_start(strm->bytes[strm->pos])) {
		// it's either a literal, group or punct
		int is_literal = 0;
		int is_group = 0;
		int start_group_pos = 0;
		int end_group_pos = 0;
		Delimiter delimiter = Parenthesis;
		if(strm->bytes[strm->pos] == '(') {
			delimiter = Parenthesis;
			is_group = 1;
			strm->pos += 1;
			start_group_pos = strm->pos;
			int inner_char_count = 0;
                        while(strm->pos < len) {
				if(strm->bytes[strm->pos] == ')' && inner_char_count == 0) {
					break;
				}
                                if(strm->bytes[strm->pos] == '(') {
                                        inner_char_count += 1;
                                } else if(strm->bytes[strm->pos] == ')') {
                                        inner_char_count -= 1;
                                }
                                strm->pos += 1;
                        }
			end_group_pos = strm->pos;
			strm->pos += 1;
		} else if(strm->bytes[strm->pos] == '[') {
			delimiter = Bracket;
			is_group = 1;
			strm->pos += 1;
			start_group_pos = strm->pos;
			int inner_char_count = 0;
                        while(strm->pos < len) {
				if(strm->bytes[strm->pos] == ']' && inner_char_count == 0) {
                                        break;
				}
                                if(strm->bytes[strm->pos] == '[') {
                                        inner_char_count += 1;
                                } else if(strm->bytes[strm->pos] == ']') {
                                        inner_char_count -= 1;
                                }
                                strm->pos += 1;
                        }
			end_group_pos = strm->pos;
                        strm->pos += 1;
		} else if(strm->bytes[strm->pos] == '{') {
			delimiter = Brace;
			is_group = 1;
			strm->pos += 1;
			start_group_pos = strm->pos;
			int inner_char_count = 0;
			int in_lit = 0;
			while(strm->pos < len) {
				if(strm->bytes[strm->pos] == '}' && inner_char_count == 0 && !in_lit) {
                                        break;
				}
				if(strm->bytes[strm->pos] == '{' && !in_lit) {
					inner_char_count += 1;
				} else if(strm->bytes[strm->pos] == '}' && !in_lit) {
					inner_char_count -= 1;
				} else if((strm->bytes[strm->pos] == '\"' || strm->bytes[strm->pos] == '\'') && !in_lit) {
					in_lit = 1;
				} else if((strm->bytes[strm->pos] == '\"' || strm->bytes[strm->pos] == '\'') && in_lit) {
                                        in_lit = 0;
                                }
                                strm->pos += 1;
                        }
			end_group_pos = strm->pos;
                        strm->pos += 1;
		} else if(strm->bytes[strm->pos] == '\"') {
			is_literal = 1;
			next_token[itt] = strm->bytes[strm->pos];
			itt += 1;
			strm->pos += 1;
			while(strm->pos < len && strm->bytes[strm->pos] != '\"') {
				next_token[itt] = strm->bytes[strm->pos];
				strm->pos += 1;
				itt += 1;

			}
			strm->pos += 1;
			next_token[itt] = '\"';
			itt++;
			next_token[itt] = 0;
		} else if (strm->bytes[strm->pos] >= '0' && strm->bytes[strm->pos] <= '9') {
			is_literal = 1;
			next_token[itt] = strm->bytes[strm->pos];
			itt += 1;
                        strm->pos += 1;
                        while(strm->pos < len &&
				(strm->bytes[strm->pos] >= '0' && strm->bytes[strm->pos] <= '9') ||
				strm->bytes[strm->pos] == '.') {
                                next_token[itt] = strm->bytes[strm->pos];
                                strm->pos += 1;
                                itt += 1;

                        }

			next_token[itt] = 0;
		} else if (strm->bytes[strm->pos] == '\'') {
			next_token[itt] = strm->bytes[strm->pos];
			itt += 1;
                        strm->pos += 1;
                        while(strm->pos < len && strm->bytes[strm->pos] != '\'') {
                                next_token[itt] = strm->bytes[strm->pos];
                                strm->pos += 1;
                                itt += 1;

                        }
			strm->pos += 1;
                        next_token[itt] = '\'';
                        itt++;
			next_token[itt] = 0;
			is_literal = 1;
		}

		if(is_group) {
			next->token_type = GroupType;
			next->group = malloc(sizeof(Group));
			next->group->delimiter = delimiter;
			next->group->strm = malloc(sizeof(TokenStream));
			next->group->strm->parent = strm;
			next->group->strm->pos_offset = start_group_pos + strm->pos_offset;
			next->group->strm->file_path = NULL;
			next->group->strm->line_num = strm->line_num;
			next->group->strm->pos = 0;
			int group_len = end_group_pos - start_group_pos;
			next->group->strm->bytes = malloc(sizeof(char) * group_len);
			memcpy(next->group->strm->bytes, strm->bytes + start_group_pos, group_len);
			next->group->strm->len = group_len;

		} else if(is_literal) {
			next->token_type = LiteralType;
			next->literal = malloc(sizeof(Literal));
			next->literal->literal = malloc(sizeof(char) * itt);
			strcpy(next->literal->literal, next_token);
		} else {
			next->token_type = PunctType;
			next->punct = malloc(sizeof(Punct));
			next->punct->ch = strm->bytes[strm->pos];
			next->punct->second_ch = 0;
			next->punct->third_ch = 0;
			strm->pos += 1;

			if (strm->pos < len && is_joint_possible(next->punct->ch)) {
				char cur = strm->bytes[strm->pos];
				if(next->punct->ch == '!' && cur == '=') {
					next->punct->second_ch = cur;
					strm->pos += 1;
				} else if(next->punct->ch == '%' && cur == '=') {
					next->punct->second_ch = cur;
					strm->pos += 1;
				} else if(next->punct->ch == '&' && (cur == '&' || cur == '=')) {
					next->punct->second_ch = cur;
					strm->pos += 1;
				} else if(next->punct->ch == '*' && cur == '=') {
					next->punct->second_ch = cur;
					strm->pos += 1;
				} else if(next->punct->ch == '+' && cur == '=') {
                                        next->punct->second_ch = cur;
					strm->pos += 1;
                                } else if(next->punct->ch == '-' && (cur == '=' || cur == '>')) {
                                        next->punct->second_ch = cur;
					strm->pos += 1;
                                } else if(next->punct->ch == '/' && cur == '=') {
                                        next->punct->second_ch = cur;
					strm->pos += 1;
                                } else if(next->punct->ch == '^' && cur == '=') {
                                        next->punct->second_ch = cur;
					strm->pos += 1;
                                } else if(next->punct->ch == '|' && (cur == '=' || cur == '|')) {
					next->punct->second_ch = cur;
                                        strm->pos += 1;
				} else if(next->punct->ch == '=' && (cur == '>' || cur == '=')) {
					next->punct->second_ch = cur;
                                        strm->pos += 1;
				} else if(next->punct->ch == '<' && (cur == '<' || cur == '=')) {
					next->punct->second_ch = cur;
                                        strm->pos += 1;
					if(strm->pos < len && cur == '<') {
                                                cur = strm->bytes[strm->pos];
                                                if(cur == '=') {
                                                        next->punct->third_ch = cur;
                                                        strm->pos += 1;
                                                }
                                        }
				} else if(next->punct->ch == '>' && (cur == '>' || cur == '=')) {
                                        next->punct->second_ch = cur;
                                        strm->pos += 1;
					if(strm->pos < len && cur == '>') {
                                                cur = strm->bytes[strm->pos];
                                                if(cur == '=') {
                                                        next->punct->third_ch = cur;
                                                        strm->pos += 1;
                                                }
                                        }
				} else if(next->punct->ch == '.' && cur == '.') {
					next->punct->second_ch = cur;
                                        strm->pos += 1;
					if(strm->pos < len) {
						cur = strm->bytes[strm->pos];
						if(cur == '.' || cur == '=') {
							next->punct->third_ch = cur;
                                        		strm->pos += 1;
						}
					}
				}
			}
		}
	} else {
		// ident start
		while(strm->pos < len) {
			next_token[itt] = strm->bytes[strm->pos];

			if (!is_ident_secondary(strm->bytes[strm->pos])) {
                        	break;
                	}

			strm->pos += 1;
			itt += 1;
		}

		if(itt == 0) {
			ret = 0;
		}

		next_token[itt] = 0;
		next->token_type = IdentType;
		next->ident = malloc(sizeof(Ident));
		next->ident->value = malloc(sizeof(char) * itt);
		strcpy(next->ident->value, next_token);
	}

	return ret;
}

int free_token_stream(TokenStream *strm) {
	free(strm->bytes);
	if(strm->file_path != NULL)
		free(strm->file_path);
	return 0;
}

int free_token_tree(TokenTree *tree) {
	if(tree->token_type == IdentType) {
		free(tree->ident->value);
		free(tree->ident);
	} else if(tree->token_type == PunctType) {
		free(tree->punct);
	} else if(tree->token_type == LiteralType) {
		free(tree->literal->literal);
		free(tree->literal);
	} else if(tree->token_type == GroupType) {
		free(tree->group->strm);
		free(tree->group);
	}
	return 0;
}

