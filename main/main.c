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
#include "../parser/parser.h"

int process_token(TokenStream strm, TokenTree token) {
	if(token.token_type == IdentType) {
		printf("ident='%s' [file_off=%i]\n",
			token.ident->value, token.span.offset
		);
		display_span(&token.span, Warning, "invalid format");
	} else if(token.token_type == PunctType) {
		printf("punct='%c", token.punct->ch);
		if(token.punct->second_ch != 0) {
			printf("%c", token.punct->second_ch);
		}
		if(token.punct->third_ch != 0) {
                        printf("%c", token.punct->third_ch);
                }
		printf("'\n");

		 //display_span(&token.span, Error, "bad punct");
	} else if(token.token_type == LiteralType) {
		printf("literal='%s'\n", token.literal->literal);
		//display_span(&token.span, Error, "illegal state, no literal");
	} else if(token.token_type == GroupType) {
		if(token.group->delimiter == Parenthesis)
			printf("group delimiter = Parenthesis\n");
		else if(token.group->delimiter == Bracket)
			printf("group delimiter = Bracket\n");
		else if(token.group->delimiter == Brace)
			printf("group delimiter = Brace\n");

		TokenTree grp_next;

                while(next_token(token.group->strm, &grp_next)) {
			process_token(*(token.group->strm), grp_next);
                        free_token_tree(&grp_next);
                }

		free_token_stream(token.group->strm);

		if(token.group->delimiter == Parenthesis)
                        printf("end group delimiter = Parenthesis\n");
                else if(token.group->delimiter == Bracket)
                        printf("end group delimiter = Bracket\n");
                else if(token.group->delimiter == Brace)
                        printf("end group delimiter = Brace\n");

		//display_span(&token.span, Warning, "warning about this group");
	}
	return 0;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("No input file specified!\n");
		exit(1);
	} else {
		TokenStream strm;
		TokenTree next;
		if(parse(argv[1], &strm)) {
			printf("error parsing file!\n");
			exit(1);
		}

		while(next_token(&strm, &next)) {
			process_token(strm, next);
			free_token_tree(&next);
		}

		free_token_stream(&strm);
	}

	return 0;
}
