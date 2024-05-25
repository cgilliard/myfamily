#include <criterion/criterion.h>
#include <parser/parser.h>
#include <util/constants.h>

Test(test, parser) {
	TokenStream strm;
	TokenTree next;

	// parse a test file
	int parsev = parse("./resources/test.fam", &strm, 0);
	cr_assert_eq(parsev, 0);

	// x - ident
	int value = next_token(&strm, &next);
	cr_assert_eq(value, 1);
	cr_assert_eq(next.token_type, IdentType);
	cr_assert_eq(strcmp(next.ident->value, "x"), 0);
	display_span(&next.span, Warning, "test123");
	free_token_tree(&next);

	// = - punct
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '=');
	free_token_tree(&next);

	// 0 - literal
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, LiteralType);
        cr_assert_eq(strcmp(next.literal->literal, "120"), 0);
	free_token_tree(&next);

	// semicolon
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, ';');
	free_token_tree(&next);

	// end of the token stream
	value = next_token(&strm, &next);
	cr_assert_eq(value, 0);
	free_token_tree(&next);

	// if called again it still returns 0
        value = next_token(&strm, &next);
        cr_assert_eq(value, 0);
        free_token_tree(&next);

	free_token_stream(&strm);
}

Test(test, parser2) {
	TokenStream strm;
        TokenTree next;

        // parse a test file
        int parsev = parse("./resources/test2.fam", &strm, 0);
	cr_assert_eq(parsev, 0);

	// MyObject - ident
        int value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, IdentType);
        cr_assert_eq(strcmp(next.ident->value, "MyObject"), 0);
        free_token_tree(&next);


	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, GroupType);
        cr_assert_eq(next.group->delimiter, Brace);

	TokenTree group_token;
	value = next_token(next.group->strm, &group_token);
	cr_assert_eq(group_token.token_type, IdentType);
        cr_assert_eq(strcmp(group_token.ident->value, "x"), 0);
	cr_assert_eq(value, 1);
	display_span(&group_token.span, Error, "test123");

	free_token_tree(&group_token);
	free_token_tree(&next);

	value = next_token(&strm, &next);
        cr_assert_eq(value, 0);
	free_token_tree(&next);

	free_token_stream(&strm);
}

Test(test, parser_file_not_found) {
	TokenStream strm;

        // parse a test file
        int parsev = parse("./resources/testx.fam", &strm, DEBUG_FLAG_OOM);
        cr_assert_eq(parsev, -1);
}

Test(test, parser_doc) {
	TokenStream strm;
	TokenTree next;

	// parse a test file
        int parsev = parse("./resources/test_doc.fam", &strm, 0);
	cr_assert_eq(parsev, 0);

	// # - punct (like rust /// is turned into #[doc=""])
        int value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '#');
        free_token_tree(&next);

	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, GroupType);
        cr_assert_eq(next.group->delimiter, Bracket);

	TokenTree group_token;
        value = next_token(next.group->strm, &group_token);
        cr_assert_eq(group_token.token_type, IdentType);
        cr_assert_eq(strcmp(group_token.ident->value, "doc"), 0);
        cr_assert_eq(value, 1);
	free_token_tree(&group_token);

	value = next_token(next.group->strm, &group_token);
        cr_assert_eq(value, 1);
        cr_assert_eq(group_token.token_type, PunctType);
        cr_assert_eq(group_token.punct->ch, '=');
        free_token_tree(&group_token);

	value = next_token(next.group->strm, &group_token);
        cr_assert_eq(value, 1);
        cr_assert_eq(group_token.token_type, LiteralType);
        cr_assert_eq(strcmp(group_token.literal->literal, "\"This is a doc comment\""), 0);
        free_token_tree(&group_token);
	free_token_tree(&next);

	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, IdentType);
        cr_assert_eq(strcmp(next.ident->value, "abc"), 0);
        free_token_tree(&next);


	free_token_stream(&strm);
}

Test(test, parser_single_quote) {
	TokenStream strm;
        TokenTree next;

        // parse a test file
        int parsev = parse("./resources/test_single_quote_str.fam", &strm, 0);
        cr_assert_eq(parsev, 0);

	// x - ident
        int value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, IdentType);
        cr_assert_eq(strcmp(next.ident->value, "x"), 0);
        display_span(&next.span, Warning, "test123");
        free_token_tree(&next);

	// = - punct
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '=');
        free_token_tree(&next);

	// = - literal
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, LiteralType);
        cr_assert_eq(strcmp(next.literal->literal, "\'abcd\'"), 0);
        free_token_tree(&next);

	// = - punct
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, ';');
        free_token_tree(&next);

	// end of the token stream
        value = next_token(&strm, &next);
        cr_assert_eq(value, 0);
        free_token_tree(&next);

	free_token_stream(&strm);
}

Test(test, parser_oom) {
	TokenStream strm;
	int parsev = parse("./resources/test_other_groups.fam", &strm, DEBUG_FLAG_OOM);
	cr_assert_eq(parsev, -1);
}

Test(test, parser_puncts) {
TokenStream strm;
        TokenTree next;

        // parse a test file
        int parsev = parse("./resources/test_punct.fam", &strm, 0);
        cr_assert_eq(parsev, 0);
	int value;

        // =>
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '=');
	cr_assert_eq(next.punct->second_ch, '>');
        free_token_tree(&next);

	// <=
	value = next_token(&strm, &next);
	cr_assert_eq(value, 1);
	cr_assert_eq(next.token_type, PunctType);
	cr_assert_eq(next.punct->ch, '<');
	cr_assert_eq(next.punct->second_ch, '=');
	free_token_tree(&next);

	// &=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '&');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// !=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '!');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// *=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '*');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// -=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '-');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// +=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '+');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// /=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '/');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// %=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '%');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// |=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '|');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// ^=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '^');
        cr_assert_eq(next.punct->second_ch, '=');
        free_token_tree(&next);

	// <<=
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '<');
        cr_assert_eq(next.punct->second_ch, '<');
	cr_assert_eq(next.punct->third_ch, '=');
        free_token_tree(&next);

	 // >>=
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '>');
        cr_assert_eq(next.punct->second_ch, '>');
        cr_assert_eq(next.punct->third_ch, '=');
        free_token_tree(&next);

	// ...
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '.');
        cr_assert_eq(next.punct->second_ch, '.');
        cr_assert_eq(next.punct->third_ch, '.');
        free_token_tree(&next);

	// ( unclosed
	value = next_token(&strm, &next);
	// this returns 2 indicating err
        cr_assert_eq(value, 2);

	free_token_stream(&strm);

}

Test(test, test_unexpected_close_bracket) {
	TokenStream strm;
        TokenTree next;

        // parse a test file
        int parsev = parse("./resources/test_unexpected_close_bracket.fam", &strm, 0);
        cr_assert_eq(parsev, 0);

	int value;

	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, IdentType);
        cr_assert_eq(strcmp(next.ident->value, "abc"), 0);
        free_token_tree(&next);

	value = next_token(&strm, &next);
	// unexpected close bracket err
        cr_assert_eq(value, 2);

	free_token_stream(&strm);
}

Test(test, test_inner_lit) {
	TokenStream strm;
        TokenTree next;
	TokenTree group_token;
	TokenTree group2_token;

        // parse a test file
        int parsev = parse("./resources/test_inner_lit.fam", &strm, 0);
        cr_assert_eq(parsev, 0);

        int value;

        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, GroupType);
        cr_assert_eq(next.group->delimiter, Parenthesis);

	value = next_token(next.group->strm, &group_token);
        cr_assert_eq(value, 1);
        cr_assert_eq(group_token.token_type, GroupType);

	value = next_token(group_token.group->strm, &group2_token);
	cr_assert_eq(value, 1);
	cr_assert_eq(group2_token.token_type, LiteralType);

	free_token_tree(&group_token);
        free_token_tree(&next);

        free_token_stream(&strm);
}

Test(test, parser_other_groups) {
	TokenStream strm;
        TokenTree next;
        
        // parse a test file
        int parsev = parse("./resources/test_other_groups.fam", &strm, 0);
        cr_assert_eq(parsev, 0); 

	// x - ident
        int value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, IdentType);
        cr_assert_eq(strcmp(next.ident->value, "x"), 0);
        free_token_tree(&next);

        // = - punct
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '=');
        free_token_tree(&next);

	// paren group
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, GroupType);
        cr_assert_eq(next.group->delimiter, Parenthesis);
	free_token_tree(&next);

        // ; - punct
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, ';');
        free_token_tree(&next);

        // y - ident
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, IdentType);
        cr_assert_eq(strcmp(next.ident->value, "y"), 0);
        free_token_tree(&next);

        // = - punct
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '=');
        free_token_tree(&next);

        // bracket group
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, GroupType);
        cr_assert_eq(next.group->delimiter, Bracket);
        free_token_tree(&next);

        // ; - punct
        value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, ';');
        free_token_tree(&next);

	free_token_stream(&strm);
}

