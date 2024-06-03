#include <criterion/criterion.h>

int real_main(int argc, char** argv);

Test(test, main)
{
    int r = real_main(0, NULL);
    cr_assert_eq(r, 0);
}
