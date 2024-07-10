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

#include <core/colors.h>
#include <core/resources.h>
#include <errno.h>
#include <lexer/lexer.h>
#include <stdlib.h>
#include <string.h>

char *lexer_read_line(Lexer *l) {
	char *ret = NULL;
	u64 count = 0;
	while (true) {
		if (ret == NULL) {
			ret = mymalloc(sizeof(char) * LEXER_BUF_SIZE);
			strcpy(ret, "");
		} else {
			char *tmp = myrealloc(
			    ret, sizeof(char) * LEXER_BUF_SIZE * (1 + count));
			if (tmp == NULL) {
				myfree(ret);
				return NULL;
			}
			ret = tmp;
		}
		u64 str_len = strlen(ret);
		if (fgets(ret + str_len, LEXER_BUF_SIZE, l->fp) == NULL) {
			myfree(ret);
			return NULL;
		}

		count += 1;
		str_len = strlen(ret);
		if (str_len >= (count * (LEXER_BUF_SIZE - 1)))
			continue;

		return ret;
	}
}

int lexer_init(Lexer *l, char *file) {
	// file/lexer cannot be null
	if (file == NULL || l == NULL) {
		errno = EINVAL;
		return LexerStateErr;
	}

	l->fp = NULL;
	l->tokenizer = NULL;
	l->line_num = 1;
	l->file = mymalloc(sizeof(char) * (strlen(file) + 1));
	if (l->file == NULL) {
		lexer_cleanup(l);
		return -1;
	}
	strcpy(l->file, file);

	// open the file for reading
	l->fp = myfopen(file, "r");

	// if file cannot be read return error
	if (l->fp == NULL) {
		return LexerStateErr;
	}

	// allocate memory for the tokenizer
	l->tokenizer = mymalloc(sizeof(Tokenizer));

	if (l->tokenizer == NULL) {
		// memory allocation err
		lexer_cleanup(l);
		return LexerStateErr;
	}

	char *buf = lexer_read_line(l);
	if (buf == NULL) {
		lexer_cleanup(l);
		return LexerStateErr;
	}

	// try to init the tokenizer
	if (tokenizer_init(l->tokenizer, buf) != TokenizerStateOk) {
		lexer_cleanup(l);
		myfree(buf);
		return LexerStateErr;
	} else {
		myfree(buf);
		return LexerStateOk;
	}
}

int lexer_next_token(Lexer *l, Token *token) {
	// read the next token
	int state = tokenizer_next_token(l->tokenizer, token);
	char *line = NULL;
	token->span = NULL;

	// if we're complete keep trying to get more lines until we have a token
	// or we're at the end of file
	while (state == TokenizerStateComplete ||
	       state == TokenizerStateCompleteInComment) {
		// cleanup the previous tokenizer
		tokenizer_cleanup(l->tokenizer);

		char *buf = lexer_read_line(l);

		if (buf == NULL) {
			if (line)
				myfree(line);
			if (state == TokenizerStateCompleteInComment)
				return LexerStateErr;
			else
				return LexerStateComplete;
		}

		l->line_num += 1;

		// try to init again
		if (tokenizer_init(l->tokenizer, buf) != TokenizerStateOk) {
			myfree(buf);
			if (line) {
				myfree(line);
			}
			return LexerStateErr;
		}

		state = tokenizer_next_token(l->tokenizer, token);
		if (line)
			myfree(line);
		line = mymalloc(sizeof(char) * (1 + strlen(buf)));
		strcpy(line, buf);
		myfree(buf);
	}

	if (line == NULL) {
		line = mymalloc((strlen(l->tokenizer->s) + 1) * sizeof(char));
		strcpy(line, l->tokenizer->s);
	}

	if (state == TokenizerStateOk) {
		token->line_num = l->line_num;
		if (line == NULL)
			token->span = NULL;
		else {
			int num_width = snprintf(NULL, 0, "%llu", l->line_num);
			int token_width = snprintf(NULL, 0, "%s", token->token);
			char spacer[100];
			int lsl = (l->tokenizer->pos + 1) - (token_width - 1);
			if (lsl <= 0)
				lsl = 1;
			char long_spacer[lsl];
			bool has_tab = false;
			for (int i = 0; i < lsl; i++) {
				if (i < l->tokenizer->pos &&
				    l->tokenizer->s[i] == '\t') {
					has_tab = true;
					long_spacer[i] = '\t';
				} else {
					long_spacer[i] = ' ';
				}
			}
			if (has_tab)
				long_spacer[lsl - 2] = 0;
			long_spacer[lsl - 1] = 0;
			for (int i = 0; i < num_width && i < 100; i++) {
				spacer[i] = ' ';
			}
			if (num_width < 100)
				spacer[num_width] = 0;

			int span_len =
			    snprintf(NULL, 0,
				     "%s%s-->%s %s:%llu:%i\n%s %s|\n%llu | %s%s"
				     "%s %s|%s%s^\n",
				     spacer, CYAN, RESET, l->file, l->line_num,
				     lsl, spacer, CYAN, l->line_num, RESET,
				     line, spacer, CYAN, RESET, long_spacer);
			token->span = mymalloc((span_len + 1) * sizeof(char));
			if (token->span == NULL) {
				myfree(line);
				return LexerStateErr;
			}
			snprintf(token->span, span_len + 1,
				 "%s%s-->%s %s:%llu:%i\n%s %s|\n%llu | %s%s"
				 "%s %s|%s%s^\n",
				 spacer, CYAN, RESET, l->file, l->line_num, lsl,
				 spacer, CYAN, l->line_num, RESET, line, spacer,
				 CYAN, RESET, long_spacer);
			myfree(line);
		}

		return LexerStateOk;
	} else {
		if (line)
			myfree(line);
		return LexerStateErr;
	}
}

void lexer_cleanup(Lexer *l) {
	// check if fp is NULL and close it then set to NULL so this can be
	// called multiple times
	if (l->fp != NULL) {
		myfclose(l->fp);
		l->fp = NULL;
	}

	// same for the tokenizer
	if (l->tokenizer != NULL) {
		tokenizer_cleanup(l->tokenizer);
		myfree(l->tokenizer);
	}

	// same for file
	if (l->file != NULL) {
		myfree(l->file);
		l->file = NULL;
	}
}
