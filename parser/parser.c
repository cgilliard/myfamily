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
#include <parser/parser.h>
#include <util/constants.h>
#include <util/misc.h>

int parse(char *file_name, TokenStream *strm, int debug_flags) {
	FILE *file = fopen(file_name, "r");
	if (file == NULL) {
		fputs("File not found", stderr);
		return -1;
	}
    	long long file_size = 0; // File size to zero before byte check
       	fseek(file, 0L, SEEK_END); // seek to end
       	file_size = ftell(file); // find size of file by checking end
       	rewind(file); // back to the beginning for reading
    	char buffer[16]; // We will read the file 16 bytes at a time
    	strm->bytes = malloc(file_size);

	if(strm->bytes == NULL || (debug_flags & DEBUG_FLAG_OOM) != 0) {
		fputs("Could not allocate enough memory", stderr);
		fclose(file);
		return -1;
    	}

	int ret = fread(strm->bytes, file_size, 1, file);
	strm->len = file_size;
	fclose(file);
	strm->pos = 0;
	strm->line_num = 1;
	strm->start_doc = -1;
	strm->end_doc = -1;
	strm->file_path = malloc(sizeof(char)*(strlen(file_name)+1));
	strcpy(strm->file_path, file_name);
	strm->parent = NULL;
	strm->pos_offset = 0;

	return 0;
}

int display_span(Span *span, ErrorLevel level, char *message) {
	TokenStream *file_stream = span->strm;
        int start = span->offset;
        int end = span->offset;

	while(file_stream->parent != NULL)
		file_stream = file_stream->parent;
	while(start != 0 &&  file_stream->bytes[start-1] != '\n')
		start -= 1;
	while(end < file_stream->len && file_stream->bytes[end] != '\n')
		end += 1;

	int span_len = end - start;
	char display_str[span_len+1];
	memcpy(display_str, file_stream->bytes + start, span_len);
	display_str[span_len] = 0;
	int indent = 1 + (span->offset - start);

	if(level == Error) { // ERROR
		printf(
			"\e[1m%s:%i:%i:\e[m " ANSI_COLOR_ERROR_BG "Error" ANSI_COLOR_RESET ": %s\n%s\n",
			file_stream->file_path,
			span->line_num,
			indent,
			message,
			display_str
		);
	} else { // WARNING
		printf(         
                        "\e[1m%s:%i:%i:\e[m " ANSI_COLOR_WARNING_BG "Warning" ANSI_COLOR_RESET ": %s\n%s\n",
                        file_stream->file_path,
                        span->line_num,
			indent,
                        message,        
                        display_str
                );
	}
	char spacing_and_up_arrow[(span->offset - start) + 2];
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
	printf(ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, spacing_and_up_arrow);

	return 0;
}

void process_doc(TokenStream *strm, TokenTree *next) {
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
	next->span.offset = strm->start_doc + strm->pos_offset;
	next->span.strm = strm;

	strm->start_doc = -1;
	strm->end_doc = -1;
}

int skip_comments_and_white_space(TokenStream *strm, TokenTree *next, int len) {
	while(TRUE) {
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
                                        return TRUE;
                                }
                        } else {
                                while(strm->pos < len && (strm->bytes[strm->pos-1] != '*' || strm->bytes[strm->pos] != '/')) {
                                        if(strm->bytes[strm->pos] == '\n'){
                                                strm->line_num += 1;
                                        }
                                        strm->pos += 1;
                                }
                                strm->pos += 1;
                        }

                } else {
                        break;
                }
	}

	return FALSE;
}

int process_group(TokenStream *strm, TokenTree *next, int ret, int len, Delimiter delimiter) {
	char open_ch;
	char close_ch;

	if(delimiter == Parenthesis) {
		open_ch = '(';
		close_ch = ')';
	} else if(delimiter == Bracket) {
		open_ch = '[';
                close_ch = ']';
	} else if(delimiter == Brace) {
		open_ch = '{';
		close_ch = '}';
	}

	strm->pos += 1;
	int inner_char_count = 0;
	int in_lit = 0;
	while(strm->pos < len) {
		if(strm->bytes[strm->pos] == close_ch && inner_char_count == 0 && !in_lit)
			break;
		if(strm->bytes[strm->pos] == open_ch && !in_lit)
			inner_char_count += 1;
		else if(strm->bytes[strm->pos] == close_ch && !in_lit)
			inner_char_count -= 1;
		else if((strm->bytes[strm->pos] == '\"' || strm->bytes[strm->pos] == '\'') && !in_lit)
			in_lit = 1;
		else if((strm->bytes[strm->pos] == '\"' || strm->bytes[strm->pos] == '\'') && in_lit)
			in_lit = 0;

		if(strm->bytes[strm->pos] == '\n')
			strm->line_num += 1;
		strm->pos += 1;
	}
	if(strm->pos == len) {
		char msg[100];
		sprintf(msg, "parse error: file ended with unclosed '%c'", open_ch);
		display_span(&next->span, Error, msg);
		ret = 2;
	}
	strm->pos += 1;

	return ret;
}

int process_punct(TokenStream *strm, TokenTree *next, int len, int ret) {
	char ch = strm->bytes[strm->pos];
	if(ch == ')' || ch == ']' || ch == '}') {
		char err[100];
		next->span.line_num = strm->line_num;
		sprintf(err, "parse error: unexpected token, '%c'", ch);
		display_span(&next->span, Error, err);
		ret = 2;
	}

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
	return ret;
}

int next_token(TokenStream *strm, TokenTree *next) {
	int len = strm->len;
	char next_token[len+1];
	int itt = 0;
	int ret;

	if(strm->start_doc > 0) {
		process_doc(strm, next);
		return 1;
	}

	if(strm->pos >= len) {
                ret = 0;
        } else {
                ret = 1;
        }

	if(skip_comments_and_white_space(strm, next, len))
		return ret;

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
			start_group_pos = strm->pos + 1;
			ret = process_group(strm, next, ret, len, delimiter);
			end_group_pos = strm->pos - 1;
		} else if(strm->bytes[strm->pos] == '[') {
			delimiter = Bracket;
                        is_group = 1;
                        start_group_pos = strm->pos + 1;
                        ret = process_group(strm, next, ret, len, delimiter);
                        end_group_pos = strm->pos - 1;
		} else if(strm->bytes[strm->pos] == '{') {
			delimiter = Brace;
                        is_group = 1;
                        start_group_pos = strm->pos + 1;
                        ret = process_group(strm, next, ret, len, delimiter);
                        end_group_pos = strm->pos - 1;
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
			// punct
			ret = process_punct(strm, next, len, ret);
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

void free_token_stream(TokenStream *strm) {
	free(strm->bytes);
	if(strm->file_path != NULL)
		free(strm->file_path);
}

void free_token_tree(TokenTree *tree) {
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
}

