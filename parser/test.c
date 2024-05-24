#include <criterion/criterion.h>
#include <parser/parser.h>

Test(test, parser) {
	TokenStream strm;
	TokenTree next;

	// parse a test file
	int parsev = parse("./resources/test.fam", &strm);

	// x - ident
	int value = next_token(&strm, &next);
	cr_assert_eq(value, 1);
	cr_assert_eq(next.token_type, IdentType);
	cr_assert_eq(strcmp(next.ident->value, "x"), 0);
	display_span(&next.span, Warning, "test123");

	// = - punct
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, '=');

	// 0 - literal
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, LiteralType);
        cr_assert_eq(strcmp(next.literal->literal, "0"), 0);

	// semicolon
	value = next_token(&strm, &next);
        cr_assert_eq(value, 1);
        cr_assert_eq(next.token_type, PunctType);
        cr_assert_eq(next.punct->ch, ';');

	// end of the token stream
	value = next_token(&strm, &next);
	cr_assert_eq(value, 0);

	free_token_stream(&strm);
	cr_assert_eq(parsev, 0);
}
