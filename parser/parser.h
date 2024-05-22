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
};
typedef struct TokenStream TokenStream;

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

// Spacing - for punct
enum Spacing {
	Joint = 0,
	Alone = 1
};
typedef enum Spacing Spacing;

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
	Spacing spacing;
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
};
typedef struct TokenTree TokenTree;

int parse(char *file, TokenStream *strm);
int next_token(TokenStream *strm, TokenTree *next);
int free_token_stream(TokenStream *strm);
int free_token_tree(TokenTree *tree);

#endif // PARSER_H_
