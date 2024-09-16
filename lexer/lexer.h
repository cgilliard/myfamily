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

#ifndef _LEXER_LEXER__
#define _LEXER_LEXER__

#include <lexer/tokenizer.h>

#define LEXER_BUF_SIZE 1024

typedef enum LexerState { LexerStateOk = 0, LexerStateErr = 1, LexerStateComplete = 2 } LexerState;

typedef struct Lexer {
	Tokenizer *tokenizer;
	FILE *fp;
	char *file;
	u64 line_num;
} Lexer;

int lexer_init(Lexer *l, char *file);
int lexer_next_token(Lexer *l, Token *token);
char *lexer_read_line(Lexer *l);
void lexer_cleanup(Lexer *l);

#endif // _LEXER_LEXER__
