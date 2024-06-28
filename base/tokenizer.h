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

#define TRAIT_TOKEN(T)                                                         \
	TRAIT_REQUIRED(T, T##Ptr, build, TokenType type, char *token)          \
	TRAIT_REQUIRED(T, T##Ptr, build_lit_num, i128)

typedef enum TokenType {
	NoType = 0,
	IdentType = 1,
	LiteralType = 2,
	PunctType = 3,
	DocType = 4
} TokenType;

CLASS(Token, FIELD(TokenType, ttype) FIELD(StringRef, text))
IMPL(Token, TRAIT_DISPLAY)
IMPL(Token, TRAIT_DEBUG)
IMPL(Token, TRAIT_TOKEN)
IMPL(Token, TRAIT_COPY)
#define Token DEFINE_CLASS(Token)

CLASS(Tokenizer, FIELD(u64, pos) FIELD(StringRef, ref) FIELD(bool, in_comment))
IMPL(Tokenizer, TRAIT_TOKENIZER)
IMPL(Tokenizer, TRAIT_COPY)
#define Tokenizer DEFINE_CLASS(Tokenizer)

StringRef Token_simple_str(Token *ptr);

#define TOKEN_INIT BUILD(Token, NoType, BUILD(Ident), {}, {NULL})
#define IDENT(s) Ident_build(s)
#define DOC(s) Doc_build(s)
#define PUNCT(...) {{&PunctVtable, "Punct"}, __VA_ARGS__}
#define TOKEN_TYPE(x) *Token_get_ttype(&x)
#define TOKEN_STR(x) to_debug(&x)
#define NEXT_TOKEN(x) Tokenizer_next_token(&x)

static GETTER(Token, ttype);
static GETTER(Token, text);
static GETTER(Tokenizer, in_comment);
static SETTER(Tokenizer, in_comment);

#define TOKEN(ttype, s)                                                        \
	_Generic((s), i128: Token_build_lit_num, default: Token_build)(ttype, s)

#endif // _BASE_TOKENIZER__
