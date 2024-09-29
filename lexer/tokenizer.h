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
	TokenTypePunct,
	TokenTypeNumLiteral,
	TokenTypeFloatLiteral,
	TokenTypeStringLiteral,
	TokenTypeBoolLiteral,
	TokenTypeCharLiteral,
	TokenTypeIdent,
	TokenTypeDoc,
} TokenType;

typedef struct TokenImpl {
	TokenType type;
	char *token;
	u64 line_num;
	char *span;
} TokenImpl;

typedef struct TokenizerIml {
	char *s;
	u64 pos;
	u64 len;
	bool in_comment;
} TokenizerImpl;

void tokenizer_cleanup(TokenizerImpl *t);
void token_cleanup(TokenImpl *t);

#define Token TokenImpl __attribute__((warn_unused_result, cleanup(token_cleanup)))
#define Tokenizer TokenizerImpl __attribute__((warn_unused_result, cleanup(tokenizer_cleanup)))

int token_display_error(Token *token, char *fmt, ...);
int token_display_warning(Token *token, char *fmt, ...);
int tokenizer_init(Tokenizer *t, char *line);
int tokenizer_next_token(Tokenizer *t, Token *next);

#endif // _LEXER_TOKENIZER__
