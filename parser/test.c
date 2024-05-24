#include <criterion/criterion.h>
#include <parser/parser.h>

Test(test, parser) {
	int hello1 = 0;
	cr_assert_eq(hello1, 0);
	TokenStream strm;
	TokenTree next;
	parse("./resources/test.fam", &strm);
}
