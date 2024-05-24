#include <criterion/criterion.h>

Test(test, util) {
	int x = 1;
	int y = 1;
	cr_assert_eq(x, y);
}
