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

#include <errno.h>
#include <parser/tokenizer.h>
#include <stdlib.h>
#include <string.h>

int tokenizer_init(Tokenizer *t, char *line) {
	// check input
	if (line == NULL) {
		errno = EINVAL;
		return TokenizerStateErr;
	}

	// copy the input string
	t->len = strlen(line);
	t->s = malloc(sizeof(char) * (1 + t->len));
	if (t->s == NULL) {
		return TokenizerStateErr;
	}
	strcpy(t->s, line);

	// set pos to 0
	t->pos = 0;

	return TokenizerStateOk;
}

bool tokenizer_is_white_space(Tokenizer *t) {
	char ch = t->s[t->pos];
	// check for std whitespace chars
	if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' ||
	    ch == '\r')
		return true;
	return false;
}

int tokenizer_skip_comments(Tokenizer *t, Token *next) {
	if (t->pos + 1 >= t->len)
		return TokenizerStateOk;

	char ch1 = t->s[t->pos];
	char ch2 = t->s[t->pos + 1];

	if (ch1 == '/' && ch2 == '/') {
		while (true) {
			if (t->pos >= t->len || t->s[t->pos] == '\n')
				break;
			t->pos += 1;
		}
	}

	return TokenizerStateOk;
}

int tokenizer_skip_white_space(Tokenizer *t, Token *next) {
	while (true) {
		// if we're at the end of the string or not in white space break
		if (t->pos >= t->len || !tokenizer_is_white_space(t))
			break;

		t->pos += 1;
	}

	return TokenizerStateOk;
}

int tokenizer_skip_to_token(Tokenizer *t, Token *next) {
	while (true) {
		tokenizer_skip_white_space(t, next);
		tokenizer_skip_comments(t, next);

		if (t->pos >= t->len || t->s[t->pos] != '\n')
			break;
	}

	if (t->pos >= t->len)
		return TokenizerStateComplete;

	return TokenizerStateOk;
}

int tokenizer_proc_ident(Tokenizer *t, Token *next) {
	next->type = TokenTypeIdent;
	u64 start = t->pos;
	while (true) {
		// if we're at the end of the string, break
		if (t->pos >= t->len)
			break;

		char ch = t->s[t->pos];

		// continue until non-ident char
		if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
		      ch == '_' || (ch >= '0' && ch <= '9'))) {
			break;
		}
		t->pos += 1;
	}
	u64 end = t->pos;
	u64 tlen = end - start;

	// allocate and copy over the token
	next->token = malloc(sizeof(char) * (tlen + 1));
	if (next->token == NULL)
		return TokenizerStateErr;
	memcpy(next->token, t->s + start, tlen);
	next->token[tlen] = 0;

	return TokenizerStateOk;
}

int tokenizer_proc_string_literal(Tokenizer *t, Token *next) {
	next->type = TokenTypeLiteral;
	u64 start = t->pos;
	while (true) {
		// if we're at the end of the string, break
		if (t->pos >= t->len)
			break;

		char ch = t->s[t->pos];

		// continue until literal termination char
		if (ch == '\"' && t->pos != start) {
			t->pos += 1;
			break;
		}
		t->pos += 1;
	}
	u64 end = t->pos;
	u64 tlen = end - start;

	// allocate and copy over the token
	next->token = malloc(sizeof(char) * (tlen + 1));
	if (next->token == NULL)
		return TokenizerStateErr;
	memcpy(next->token, t->s + start, tlen);
	next->token[tlen] = 0;

	return TokenizerStateOk;
}

int tokenizer_proc_char_literal(Tokenizer *t, Token *next) {
	next->type = TokenTypeLiteral;
	u64 start = t->pos;
	while (true) {
		// if we're at the end of the string, break
		if (t->pos >= t->len)
			break;

		char ch = t->s[t->pos];

		// continue until literal termination char
		if (ch == '\'' && t->pos != start) {
			t->pos += 1;
			break;
		}
		t->pos += 1;
	}
	u64 end = t->pos;
	u64 tlen = end - start;

	// allocate and copy over the token
	next->token = malloc(sizeof(char) * (tlen + 1));
	if (next->token == NULL)
		return TokenizerStateErr;
	memcpy(next->token, t->s + start, tlen);
	next->token[tlen] = 0;

	return TokenizerStateOk;
}

int tokenizer_proc_num_literal(Tokenizer *t, Token *next) {
	next->type = TokenTypeLiteral;
	u64 start = t->pos;
	while (true) {
		// if we're at the end of the string, break
		if (t->pos >= t->len)
			break;

		char ch = t->s[t->pos];

		// continue until literal termination char
		if (!((ch >= '0' && ch <= '9') || ch == '.')) {
			break;
		}
		t->pos += 1;
	}
	u64 end = t->pos;
	u64 tlen = end - start;

	// allocate and copy over the token
	next->token = malloc(sizeof(char) * (tlen + 1));
	if (next->token == NULL)
		return TokenizerStateErr;
	memcpy(next->token, t->s + start, tlen);
	next->token[tlen] = 0;

	return TokenizerStateOk;
}

bool is_second_punct(char ch1, char ch2) {
	if (ch1 == '!' && ch2 == '=')
		return true;
	if (ch1 == '%' && ch2 == '%')
		return true;
	if (ch1 == '&' && (ch2 == '&' || ch2 == '='))
		return true;
	if (ch1 == '*' && ch2 == '=')
		return true;
	if (ch1 == '+' && ch2 == '=')
		return true;
	if (ch1 == '-' && (ch2 == '=' || ch2 == '>'))
		return true;
	if (ch1 == '/' && ch2 == '=')
		return true;
	if (ch1 == '^' && ch2 == '=')
		return true;
	if (ch1 == '|' && (ch2 == '=' || ch2 == '|'))
		return true;
	if (ch1 == '=' && (ch2 == '>' || ch2 == '='))
		return true;
	if (ch1 == ':' && ch2 == ':')
		return true;
	if (ch1 == '<' && (ch2 == '<' || ch2 == '='))
		return true;
	if (ch1 == '>' && (ch2 == '>' || ch2 == '='))
		return true;
	if (ch1 == '.' && ch2 == '.')
		return true;

	return false;
}
bool is_third_punct(char ch1, char ch2, char ch3) {
	if (ch1 == '<' && ch2 == '<' && ch3 == '=')
		return true;
	if (ch1 == '>' && ch2 == '>' && ch3 == '=')
		return true;
	if (ch1 == '.' && ch2 == '.' && ch3 == '.')
		return true;
	return false;
}

int tokenizer_proc_punct(Tokenizer *t, Token *next) {
	next->type = TokenTypePunct;

	char ch1 = t->s[t->pos];
	char ch2 = t->pos + 1 < t->len && is_second_punct(ch1, t->s[t->pos + 1])
		       ? t->s[t->pos + 1]
		       : 0;
	char ch3 = t->pos + 2 < t->len && ch3 != 0 &&
			   is_third_punct(ch1, ch2, t->s[t->pos + 2])
		       ? t->s[t->pos + 2]
		       : 0;

	// allocate and copy over the token
	next->token = malloc(sizeof(char) * 4);
	if (next->token == NULL)
		return TokenizerStateErr;

	next->token[0] = ch1;
	next->token[1] = ch2;
	next->token[2] = ch3;
	next->token[4] = 0;

	t->pos += 1;
	if (ch2 != 0)
		t->pos += 1;
	if (ch3 != 0 && ch2 != 0)
		t->pos += 1;

	return TokenizerStateOk;
}

int tokenizer_next_token(Tokenizer *t, Token *next) {
	// first skip all whitespace / comments
	int skip = tokenizer_skip_to_token(t, next);
	if (skip != TokenizerStateOk)
		return skip;

	// check first char to determine which type of token we're in
	char first = t->s[t->pos];
	if ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
	    first == '_') {
		// ident
		int ret = tokenizer_proc_ident(t, next);

		// special cases
		if (ret == TokenizerStateOk &&
		    (!strcmp(next->token, "true") ||
		     !strcmp(next->token, "false"))) {
			next->type = TokenTypeLiteral;
		}
		return ret;
	} else if (first == '\"') {
		// string literal
		return tokenizer_proc_string_literal(t, next);
	} else if (first == '\'') {
		// char literal
		return tokenizer_proc_char_literal(t, next);
	} else if (first >= '0' && first <= '9') {
		// number literal
		return tokenizer_proc_num_literal(t, next);
	} else {
		// punct or error
		return tokenizer_proc_punct(t, next);
	}
}

void tokenizer_cleanup(Tokenizer *t) {
	// check if it's null to be safe in case it's called twice
	if (t->s != NULL) {
		free(t->s);
		t->s = NULL;
	}
}

void token_cleanup(Token *t) {
	// check if it's null to be safe in case it's called twice
	if (t->token != NULL) {
		free(t->token);
		t->token = NULL;
	}
}
