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
			if(next.token_type == IdentType) {
				printf("token_type=Ident,token_name=%s\n", next.ident->value);
			} else if(next.token_type == PunctType) {
				printf("token_type=PunctType,token_ch='%c'\n", next.punct->ch);
			} else if(next.token_type == LiteralType) {
				printf("token_type=LiteralType,token_literal='%s'\n", next.literal->literal);
			}


			free_token_tree(&next);
		}

		free_token_stream(&strm);
	}

	return 0;
}
