#include <base/test.h>

Suite(base);

void __attribute__((constructor)) __fn_test() {
	char *bt = backtrace_full();
	printf("bt=%s\n", bt);
	unmap(bt, 4);
}

Test(sys) {
	char *bt = backtrace_full();
	printf("bt2=%s\n", bt);
	unmap(bt, 4);
}
