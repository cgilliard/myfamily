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

#ifndef _PARSER_PARSER__
#define _PARSER_PARSER__

#include <base/class.h>
#include <base/result.h>

#define DEBUG_FLAG_OOM 0x1

#define TRAIT_TOKEN_STREAM(T)                                                  \
	TRAIT_REQUIRED(T, Result, parse, char *path, u64 flags)                \
	TRAIT_REQUIRED(T, Result, next, T##Ptr *strm)

#define TRAIT_SPAN(T)                                                          \
	TRAIT_REQUIRED(T, Result, display, T##Ptr *span, ErrorLevel level,     \
		       char *message)

typedef enum ErrorLevel {
	LvlError = 0,
	LvlWarning = 1,
} ErrorLevel;

typedef enum Delimiter {
	Parenthesis = 0,
	Brace = 1,
	Bracket = 2,
} Delimiter;

typedef enum TokenTreeType {
	IdentType = 0,
	GroupType = 1,
	PunctType = 2,
	LiteralType = 3
} TokenTreeType;

CLASS(TokenStream, FIELD(FILE *, fp) FIELD(u64, pos) FIELD(u64, line_num)
		       FIELD(char *, file_name) FIELD(TokenStream *, parent)
			   FIELD(char *, line) FIELD(u64, line_off)
			       FIELD(u64, line_len) FIELD(u64, flags))
IMPL(TokenStream, TRAIT_TOKEN_STREAM)
#define TokenStream DEFINE_CLASS(TokenStream)

CLASS(Span,
      FIELD(u64, offset) FIELD(u64, line_num) FIELD(TokenStreamPtr *, strm))
IMPL(Span, TRAIT_SPAN)
#define Span DEFINE_CLASS(Span)

CLASS(Ident, FIELD(char *, value))
#define Ident DEFINE_CLASS(Ident)

CLASS(Group, FIELD(TokenStream *, strm) FIELD(Delimiter, delimiter))
#define Group DEFINE_CLASS(Group)

CLASS(Punct, FIELD(char, ch) FIELD(char, second_ch) FIELD(char, third_ch))
#define Punct DEFINE_CLASS(Punct)

CLASS(Literal, FIELD(char *, literal))
#define Literal DEFINE_CLASS(Literal)

CLASS(TokenTree,
      FIELD(TokenTreeType, token_type) FIELD(Ident, ident) FIELD(Group, group)
	  FIELD(Punct, punct) FIELD(Literal, literal) FIELD(Span, span))
#define TokenTree DEFINE_CLASS(TokenTree)

#endif // _PARSER_PARSER__
