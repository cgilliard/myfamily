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
#include <lexer/lexer.h>
#include <stdlib.h>

int lexer_init(Lexer *l, char *file) {
	// create buffer with LEXER_BUF_SIZE for reading
	char buf[LEXER_BUF_SIZE];

	// file/lexer cannot be null
	if (file == NULL || l == NULL) {
		errno = EINVAL;
		return LexerStateErr;
	}

	l->fp = NULL;
	l->tokenizer = NULL;

	// open the file for reading
	l->fp = fopen(file, "r");

	// if file cannot be read return error
	if (l->fp == NULL) {
		return LexerStateErr;
	}

	// read first line of the file, tokenizer can handle newlines or lack
	// thereof so no problems if line is longer than LEXER_BUF_SIZE
	if (fgets(buf, LEXER_BUF_SIZE, l->fp) == NULL) {
		lexer_cleanup(l);
		return LexerStateErr;
	}

	// allocate memory for the tokenizer
	l->tokenizer = malloc(sizeof(Tokenizer));

	if (l->tokenizer == NULL) {
		// memory allocation err
		lexer_cleanup(l);
		return LexerStateErr;
	}

	// try to init the tokenizer
	if (tokenizer_init(l->tokenizer, buf) != TokenizerStateOk) {
		lexer_cleanup(l);
		return LexerStateErr;
	}

	return LexerStateOk;
}

int lexer_next_token(Lexer *l, Token *token) {
	// read the next token
	int state = tokenizer_next_token(l->tokenizer, token);

	// if we're complete keep trying to get more lines until we have a token
	// or we're at the end of file
	while (state == TokenizerStateComplete ||
	       state == TokenizerStateCompleteInComment) {
		// cleanup the previous tokenizer
		tokenizer_cleanup(l->tokenizer);

		// create buffer with LEXER_BUF_SIZE for reading
		char buf[LEXER_BUF_SIZE];

		// try to read the next line
		if (feof(l->fp) || fgets(buf, LEXER_BUF_SIZE, l->fp) == NULL) {
			if (state == TokenizerStateCompleteInComment)
				return LexerStateErr;
			else
				return LexerStateComplete;
		}

		// try to init again
		if (tokenizer_init(l->tokenizer, buf) != TokenizerStateOk) {
			return LexerStateErr;
		}

		state = tokenizer_next_token(l->tokenizer, token);
	}

	if (state == TokenizerStateOk)
		return LexerStateOk;
	else
		return LexerStateErr;
}

void lexer_cleanup(Lexer *l) {
	// check if fp is NULL and close it then set to NULL so this can be
	// called multiple times
	if (l->fp != NULL) {
		fclose(l->fp);
		l->fp = NULL;
	}

	// same for the tokenizer
	if (l->tokenizer != NULL) {
		tokenizer_cleanup(l->tokenizer);
		free(l->tokenizer);
	}
}
