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

#ifndef PARSER_H_
#define PARSER_H_

// TokenStream - a representation of a stream of tokens
struct TokenStream {
	char *bytes;
	int len;
	int pos;
	int line_num;
	char *file_path;
	int start_doc;
	int end_doc;
	struct TokenStream *parent;
	int pos_offset;
};
typedef struct TokenStream TokenStream;

// Span - A region of source code, along with information needed to display an error.
struct Span {
        TokenStream *strm;
	int offset;
	int line_num;
};
typedef struct Span Span;

// Error Level for displaying a Span
enum ErrorLevel {
	Error = 0,
	Warning = 1,
};
typedef enum ErrorLevel ErrorLevel;

// Delimiters for a group
enum Delimiter {
	Parenthesis = 0,
	Brace = 1,
	Bracket = 2,
	None = 3,
};
typedef enum Delimiter Delimiter;

// TokenTree - type of token tree
enum TokenTreeType {
	IdentType = 0,
	GroupType = 1,
	PunctType = 2,
	LiteralType = 3
};
typedef enum TokenTreeType TokenTreeType;

struct Ident {
	char *value;
};
typedef struct Ident Ident;

struct Group {
	TokenStream *strm;
	Delimiter delimiter;
};
typedef struct Group Group;

struct Punct {
	char ch;
	char second_ch;
	char third_ch;
};
typedef struct Punct Punct;

struct Literal {
	char *literal;
};
typedef struct Literal Literal;

// TokenTree - a tree structure representation of a token
struct TokenTree {
	TokenTreeType token_type;
	Ident *ident;
	Group *group;
	Punct *punct;
	Literal *literal;
	Span span;
};
typedef struct TokenTree TokenTree;

int parse(char *file, TokenStream *strm);
int next_token(TokenStream *strm, TokenTree *next);
int free_token_stream(TokenStream *strm);
int free_token_tree(TokenTree *tree);
int display_span(Span *span, ErrorLevel level, char *message);

#endif // PARSER_H_
