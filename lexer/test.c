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

#include <base/test.h>
#include <lexer/lexer.h>
#include <lexer/tokenizer.h>

MySuite(lexer);

MyTest(lexer, test_parser_basic1)
{
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "this is a test"), TokenizerStateOk);
	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "this"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "is"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "a"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "test"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "testing123 \"ok\" 'x'"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "testing123"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeLiteral);
	cr_assert(!strcmp(tk.token, "\"ok\""));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeLiteral);
	cr_assert(!strcmp(tk.token, "\'x\'"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);
}
