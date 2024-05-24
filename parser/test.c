#include <criterion/criterion.h>
#include <parser/parser.h>

Test(test, parser) {
	int hello1 = 0;
	cr_assert_eq(hello1, 0);
	TokenStream strm;
	TokenTree next;
	int parsev = parse("./resources/test.fam", &strm);
	cr_assert_eq(parsev, 0);
}
