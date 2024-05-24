#include <criterion/criterion.h>
#include <parser/parser.h>

Test(test, parser) {
	TokenStream strm;
	TokenTree next;
	int parsev = parse("./resources/test.fam", &strm);
	int value = next_token(&strm, &next);
	cr_assert_eq(value, 1);
	display_span(&next.span, Warning, "test123");
	free_token_stream(&strm);
	cr_assert_eq(parsev, 0);
}
