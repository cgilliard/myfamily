#include <criterion/criterion.h>
#include <parser/parser.h>

Test(test, parser) {
	TokenStream strm;
	TokenTree next;

	// parse a test file
	int parsev = parse("./resources/test.fam", &strm);
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
        cr_assert_eq(strcmp(next.literal->literal, "0"), 0);
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

	free_token_stream(&strm);
}

Test(test, parser2) {
	TokenStream strm;
        TokenTree next;

        // parse a test file
        int parsev = parse("./resources/test2.fam", &strm);
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

	free_token_tree(&group_token);
	free_token_tree(&next);

	value = next_token(&strm, &next);
        cr_assert_eq(value, 0);
	free_token_tree(&next);

	free_token_stream(&strm);
}
