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

int next_token(TokenStream *strm, TokenTree *next) {
	int len = strlen(strm->bytes) - strm->pos;
	printf("len=%i,", len);

	char next_token[len+1];

	int pos = strm->pos;
	int itt = 0;

	// first loop until we're not in white space
	while(pos < len) {
		if(!(strm->bytes[pos] == ' ' ||
			strm->bytes[pos] == '\n' ||
			strm->bytes[pos] == '\r' ||
			strm->bytes[pos] == '\t')) {
			break;
		}
		pos += 1;
	}

	while(pos < len) {
		next_token[itt] = strm->bytes[pos];

		if(strm->bytes[pos] == ' ' ||
                        strm->bytes[pos] == '\n' ||
                        strm->bytes[pos] == '\r' ||
                        strm->bytes[pos] == '\t') {
                        break;
                }

		pos += 1;
		itt += 1;
	}
	next_token[itt] = 0;

	int ret = 0;
	next->token_type = IdentType;
	next->ident = malloc(sizeof(Ident));
	next->ident->value = malloc(sizeof(char) * itt);
	strcpy(next->ident->value, next_token);
	printf("pos=%i\n", strm->pos);

	strm->pos += 1;
	if(strm->pos == 10) {
		ret = 0;
	} else {
		ret = 1;
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

