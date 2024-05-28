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

/**
 * Data structure that represents a stream of tokens.
 * @see [next_token]
 * @see [free_token_stream]
 * @see [parse]
 */
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

/**
 * A region of source code, along with information needed to display an error.
 */
struct Span {
        TokenStream *strm;
	int offset;
	int line_num;
};
typedef struct Span Span;

/**
 * Error Level for displaying a Span/Error message.
 */
enum ErrorLevel {
	Error = 0,
	Warning = 1,
};
typedef enum ErrorLevel ErrorLevel;

/**
 * Delimiters for a group.
 * Option | Opening | Closing | Value
 * -------|---------|---------|------
 *  Parenthesis | '(' | ')' | 0
 *  Brace | '{' | '}' | 1
 *  Bracket | '[' | ']' | 2
 *  None (Not currently used) | - | - | 3
 */
enum Delimiter {
	Parenthesis = 0,
	Brace = 1,
	Bracket = 2,
	None = 3,
};
typedef enum Delimiter Delimiter;

/**
 * The type of the token tree.
 * @see [TokenTree]
 * @see [TokenStream]
 */
enum TokenTreeType {
	IdentType = 0,
	GroupType = 1,
	PunctType = 2,
	LiteralType = 3
};
typedef enum TokenTreeType TokenTreeType;

/**
 * A representation of a ident token.
 * @see [TokenTree]
 * @see [TokenStream]
 */
struct Ident {
	char *value;
};
typedef struct Ident Ident;

/**
 * A representation of a group token.
 * @see [TokenTree]
 * @see [TokenStream]
 */
struct Group {
	TokenStream *strm;
	Delimiter delimiter;
};
typedef struct Group Group;

/**
 * A representation of a punctuation token.
 * @see [TokenTree]
 * @see [TokenStream]
 */
struct Punct {
	char ch;
	char second_ch;
	char third_ch;
};
typedef struct Punct Punct;

/**
 * A representation of a literal token.
 * @see [TokenTree]
 * @see [TokenStream]
 */
struct Literal {
	char *literal;
};
typedef struct Literal Literal;

/**
 * Structure that represents a token tree.
 * @see [next_token]
 * @see [free_token_tree]
 */
struct TokenTree {
	TokenTreeType token_type;
	Ident *ident;
	Group *group;
	Punct *punct;
	Literal *literal;
	Span span;
};
typedef struct TokenTree TokenTree;

/**
 * Parse the specified file as a [TokenStream] represented by `strm`.
 * @param file The file to parse.
 * @param strm A pointer to the [TokenStream] to store the results in.
 * @param debug_flags flags used in testing. These should always be set to 0
 * in a non-testing environment.
 * @return -1 if an error occurs, otherwise return 0. If an error occurs
 * a message will be printed to stderr indicated what error occurred.
 * @see [TokenStream]
 * @see [next_token]
 * @see [free_token_stream]
 */
int parse(char *file, TokenStream *strm, int debug_flags);

/**
 * Retrieve the next token from the specified TokenStream.
 * @param strm A pointer to the token stream to retrieve the
 * next token from.
 * @param next A pointer to the TokenTree to store the next
 * token in.
 * @return 1 if there are more tokens to be read, 0 if there are no
 * more tokens in this stream, and 2 if an error has occurred.
 * @see [parse]
 * @see [free_token_tree]
 * @see [free_token_stream]
 */
int next_token(TokenStream *strm, TokenTree *next);

/**
 * Free a token stream releasing all of its allocated memory to the OS.
 * @param strm the [TokenStream] to free.
 * @see [TokenStream]
 */
void free_token_stream(TokenStream *strm);

/**
 * Free a token tree releasing all of its allocated memory to the OS.
 * @param tree the [TokenTree] to free.
 * @see [TokenTree]
 */
void free_token_tree(TokenTree *tree);

/**
 * Display the specified span with a message at the specified error level.
 * @param span the span to display.
 * @param level the [ErrorLevel] to display at.
 * @param message the message to display with this span/error.
 * @see [Span]
 */
void display_span(Span *span, ErrorLevel level, char *message);

#endif // PARSER_H_
