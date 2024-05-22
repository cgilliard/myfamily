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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "parser.h" 

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

	return 0;
}

// Rust spec is less restrictive. May have to add something to get std libraries to work?
int is_white_space(char ch) {
	if (ch == '\n' || ch == '\t' || ch == '\r' || ch == '\v' || ch == '\f' || ch == ' ')
		return 1;
	else 
		return 0;
}

// Rust spec is less restrictive. May have to add something to get std libraries to work?
int is_ident_start(char ch) {
	if ((ch <= 'Z' && ch >= 'A') || (ch <= 'z' && ch >= 'a') || ch == '_') {
		return 1;
	} else {
		return 0;
	}
}

// Rust spec is less restrictive. May have to add something to get std libraries to work?
int is_ident_secondary(char ch) {
	if (is_ident_start(ch) || (ch <= '9' && ch >= '0')) {
        	return 1;
        } else {
                return 0;
        }
}

int next_token(TokenStream *strm, TokenTree *next) {
	int len = strlen(strm->bytes);
	int ret;

	if(strm->pos >= len) {
                ret = 0;
        } else {
                ret = 1;
        }

	char next_token[len+1];

	int itt = 0;

	// first loop until we're not in white space
	while(strm->pos < len) {
		if (!is_white_space(strm->bytes[strm->pos])) {
			break;
		}
		strm->pos += 1;
	}

	// if we're not an ident start we must be another type
	if (strm->pos < len && !is_ident_start(strm->bytes[strm->pos])) {
		// it's either a literal, group or punct
		int is_literal = 0;
		if(strm->bytes[strm->pos] == '\"') {
			printf("dquote\n");
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
			// TODO: 0usize, 0.3, etc
                        while(strm->pos < len && strm->bytes[strm->pos] >= '0' && strm->bytes[strm->pos] <= '9') {
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

		if(is_literal) {
			next->token_type = LiteralType;
			next->literal = malloc(sizeof(Literal));
			next->literal->literal = malloc(sizeof(char) * itt);
			strcpy(next->literal->literal, next_token);
		} else {
			printf("ch=%i\n", strm->bytes[strm->pos]);
			next->token_type = PunctType;
			next->punct = malloc(sizeof(Punct));
			next->punct->ch = strm->bytes[strm->pos];
			strm->pos += 1;
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
	return 0;
}

int free_token_tree(TokenTree *tree) {
	return 0;
}

