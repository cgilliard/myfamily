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

#ifndef _LEXER_TOKENIZER__
#define _LEXER_TOKENIZER__

#include <base/types.h>

typedef enum TokenType {
	TokenTypePunct = 0,
	TokenTypeLiteral = 1,
	TokenTypeIdent = 2,
	TokenTypeDoc = 3,
} TokenType;

typedef struct Token {
	TokenType type;
	char *token;
	u64 line_num;
	char *span;
} Token;

typedef struct Tokenizer {
	char *s;
	u64 pos;
	u64 len;
	bool in_comment;
} Tokenizer;

typedef enum TokenizerState {
	TokenizerStateOk = 0,
	TokenizerStateErr = 1,
	TokenizerStateComplete = 2,
	TokenizerStateCompleteInComment = 3,
} TokenizerState;

int token_display_error(Token *token, char *fmt, ...);
int token_display_warning(Token *token, char *fmt, ...);
int tokenizer_init(Tokenizer *t, char *line);
int tokenizer_next_token(Tokenizer *t, Token *next);
void tokenizer_cleanup(Tokenizer *t);
void token_cleanup(Token *t);

#endif // _LEXER_TOKENIZER__
