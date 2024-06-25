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
	IdentType = 1,
	LiteralType = 2,
	PunctType = 3
} TokenType;

CLASS(Ident, FIELD(char *, value))
#define Ident DEFINE_CLASS(Ident)

CLASS(Punct, FIELD(char, ch) FIELD(char, second_ch) FIELD(char, third_ch))
#define Punct DEFINE_CLASS(Punct)

CLASS(Literal, FIELD(char *, value))
#define Literal DEFINE_CLASS(Literal)

CLASS(Token, FIELD(TokenType, ttype) FIELD(Ident, ident) FIELD(Punct, punct)
		 FIELD(Literal, literal))
IMPL(Token, TRAIT_CLONE)
#define Token DEFINE_CLASS(Token)

CLASS(Tokenizer, FIELD(u64, pos) FIELD(StringRef, ref) FIELD(bool, in_comment))
IMPL(Tokenizer, TRAIT_TOKENIZER)
IMPL(Tokenizer, TRAIT_COPY)
#define Tokenizer DEFINE_CLASS(Tokenizer)

#define TOKEN_INIT BUILD(Token, IdentType, BUILD(Ident, NULL), {}, {NULL})

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
