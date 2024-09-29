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

#include <base/resources.h>
#include <base/test.h>
#include <lexer/tokenizer.h>
#include <lexer/tokenizer_state.h>

MySuite(lexer);

MyTest(lexer, test_tokenizer) {
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
	cr_assert_eq(tk.type, TokenTypeStringLiteral);
	cr_assert(!strcmp(tk.token, "\"ok\""));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeCharLiteral);
	cr_assert(!strcmp(tk.token, "\'x\'"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
}

MyTest(lexer, test_tokenizer_negative) {
	cr_assert_eq(tokenizer_init(NULL, NULL), TokenizerStateErr);

	Tokenizer tk;
	__is_debug_malloc = true;
	cr_assert_eq(tokenizer_init(&tk, "test"), TokenizerStateErr);
	__is_debug_malloc = false;
}

MyTest(lexer, test_tokenizer2) {
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "1234 ok 5.9"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1234"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "ok"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeFloatLiteral);
	cr_assert(!strcmp(tk.token, "5.9"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "1234 true truex false false1"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1234"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeBoolLiteral);
	cr_assert(!strcmp(tk.token, "true"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "truex"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeBoolLiteral);
	cr_assert(!strcmp(tk.token, "false"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "false1"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);
}

MyTest(lexer, test_tokenizer3) {
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "1234; ok - != 5"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1234"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "ok"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "-"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "!="));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "5"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "1234;-!=x;;"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1234"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "-"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "!="));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);
}

MyTest(lexer, test_tokenizer4) {
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if(x == y) \r\n{ x = y + 10; } \n\telse { while(true)"
									"{ x += 1; if (x > 100) break; }}"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "("));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "=="));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "y"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ")"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "{"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "="));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "y"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "+"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "10"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "}"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "else"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "{"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "while"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "("));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeBoolLiteral);
	cr_assert(!strcmp(tk.token, "true"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ")"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "{"));
	token_cleanup(&tk);
	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "+="));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "("));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ">"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "100"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ")"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "break"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "}"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, "}"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
}

MyTest(lexer, test_tokenizer5) {
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x // this is a comment ; 8 ok \"abc\"\nhi;"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);
}

MyTest(lexer, test_tokenizer6) {
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x // this is a comment \n // ; 8 ok \"abc\"\nhi;"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x //\n//\n// this is a comment \n"
									"// ; 8 ok \"abc\" \nhi; //"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
}

MyTest(lexer, test_tokenizer7) {
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x //\n//\n// this is a comment \n "
									"// ; 8 ok \"abc\" \nhi; //    end comments"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x /*\n//\n// this is a comment \n "
									"// ; 8 ok \"abc\" */\nhi; //    end comments"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x /* // // // */\nhi; //    end"
									"comments"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
}

MyTest(lexer, test_tokenizer8) {
	Tokenizer t;
	Token tk;

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x /**/"
									"hi;"
									"/*    end comments"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateCompleteInComment);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x /**/"
									""
									"hi; /*    end comments*/"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x /// this is a test\nhi; "
									"/*    end comments*/"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "this is a test"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
}

MyTest(lexer, test_tokenizer9) {
	Tokenizer t;
	Token tk;
	// init tokenizer
	cr_assert_eq(tokenizer_init(&t, "if x /// this is a test\n"
									"///       ok ok ok\n"
									"    \t///aslkjdflkasf jadl "
									"asdk \" kadlfs 3224\n"
									"hi; /*    end comments*/"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "this is a test"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "ok ok ok"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "aslkjdflkasf jadl asdk \" kadlfs 3224"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	cr_assert_eq(tokenizer_init(&t, "if x /// this is a test\n"
									"///       ok ok ok\n"
									"    \t///aslkjdflkasf jadl "
									"asdk \" kadlfs 3224\n"
									"hi; /*    end comments*/ /// "
									"end on doc comment"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "this is a test"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "ok ok ok"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "aslkjdflkasf jadl asdk \" kadlfs 3224"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "hi"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeDoc);
	cr_assert(!strcmp(tk.token, "end on doc comment"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	cr_assert_eq(tokenizer_init(&t, "if x /**//**//* ok */ 2"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "if"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "x"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "2"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	cr_assert_eq(tokenizer_init(&t, "\"ok1\"\"ok2\"3;"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeStringLiteral);
	cr_assert(!strcmp(tk.token, "\"ok1\""));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeStringLiteral);
	cr_assert(!strcmp(tk.token, "\"ok2\""));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "3"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ";"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	cr_assert_eq(tokenizer_init(&t, "_1 1_ 1_2"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "_1"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1_"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1_2"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	cr_assert_eq(tokenizer_init(&t, "123u64, 777isize, 1_234usize "
									"1.9f64 2.2f32 0x1234 0y10"),
				 TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "123u64"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ","));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "777isize"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypePunct);
	cr_assert(!strcmp(tk.token, ","));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "1_234usize"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeFloatLiteral);
	cr_assert(!strcmp(tk.token, "1.9f64"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeFloatLiteral);
	cr_assert(!strcmp(tk.token, "2.2f32"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "0x1234"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeNumLiteral);
	cr_assert(!strcmp(tk.token, "0"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateOk);
	cr_assert_eq(tk.type, TokenTypeIdent);
	cr_assert(!strcmp(tk.token, "y10"));
	token_cleanup(&tk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);

	cr_assert_eq(tokenizer_init(&t, "  \t\n\n    // nothing"), TokenizerStateOk);

	cr_assert_eq(tokenizer_next_token(&t, &tk), TokenizerStateComplete);
	tokenizer_cleanup(&t);
}
