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

#ifndef _BASE_TOKENIZER__
#define _BASE_TOKENIZER__

#include <base/class.h>
#include <base/traits.h>

typedef enum TokenType {
	NoType = 0,
	IdentType = 1,
	LiteralType = 2,
	PunctType = 3
} TokenType;

CLASS(Ident, FIELD(StringRef, value))
IMPL(Ident, TRAIT_COPY)
IMPL(Ident, TRAIT_TO_STR)
IMPL(Ident, TRAIT_DISPLAY)
#define Ident DEFINE_CLASS(Ident)

Ident Ident_build(char *s);

CLASS(Punct, FIELD(char, ch) FIELD(char, second_ch) FIELD(char, third_ch))
IMPL(Punct, TRAIT_COPY)
IMPL(Punct, TRAIT_DISPLAY)
#define Punct DEFINE_CLASS(Punct)

CLASS(Literal, FIELD(StringRef, value))
IMPL(Literal, TRAIT_DISPLAY)
IMPL(Literal, TRAIT_TO_STR)
IMPL(Literal, TRAIT_COPY)
#define Literal DEFINE_CLASS(Literal)

Literal Literal_build(char *value);
Literal Literal_build_num(i128 value);

CLASS(Token, FIELD(TokenType, ttype) FIELD(Ident *, ident) FIELD(Punct *, punct)
		 FIELD(Literal *, literal))
IMPL(Token, TRAIT_DISPLAY)
IMPL(Token, TRAIT_COPY)
#define Token DEFINE_CLASS(Token)

Token Build_token_ident(Ident ident);
Token Build_token_literal(Literal literal);
Token Build_token_punct(Punct punct);

CLASS(Tokenizer, FIELD(u64, pos) FIELD(StringRef, ref) FIELD(bool, in_comment))
IMPL(Tokenizer, TRAIT_TOKENIZER)
IMPL(Tokenizer, TRAIT_COPY)
#define Tokenizer DEFINE_CLASS(Tokenizer)

#define TOKEN_INIT BUILD(Token, NoType, BUILD(Ident), {}, {NULL})
#define IDENT(s) Ident_build(s)
#define PUNCT(...) {{&PunctVtable, "Punct"}, __VA_ARGS__}
static GETTER(Punct, ch);
static GETTER(Punct, second_ch);
static GETTER(Punct, third_ch);
static GETTER(Token, ttype);
static GETTER(Token, ident);
static GETTER(Token, punct);
static GETTER(Token, literal);

#define LITERAL(s)                                                             \
	_Generic((s), char *: Literal_build, default: Literal_build_num)(s)

#define TOKEN(s)                                                               \
	_Generic((s),                                                          \
	    Ident: Build_token_ident,                                          \
	    Literal: Build_token_literal,                                      \
	    Punct: Build_token_punct)(s)
/*
StringRef s = STRINGP("this is a test");
Tokenizer tokenizer = TOKENIZER(&s);
while (true) {
Result next = NEXT_TOKEN(&tokenizer);
if (!next.is_ok()) {
	// handle err
	break;
}
Option opt = *(Option *)unwrap(&next);
if (!opt.is_some())
	break;
Token token = *(Token *)unwrap(&opt);
// process token tree
}
*/

#endif // _BASE_TOKENIZER__
