#include <stdio.h>

// This is a script used to generate the FOR_EACH macros.
int main() {
	for (int i = 1; i <= 256; i++) {
		char param_list[1000];
		char action_list[10000];
		int offset = 0;
		for (int j = 0; j < i; j++) {
			int len = snprintf(param_list + offset, 1000 - offset,
					   " v%i", j);
			offset += len;
			if (j != i - 1) {
				len = snprintf(param_list + offset,
					       1000 - offset, ",");
				offset += len;
			}
		}
		offset = 0;
		for (int j = 0; j < i; j++) {
			int len = snprintf(action_list + offset, 10000 - offset,
					   " action(v%i)", j);
			offset += len;
			if (j != i - 1) {
				len = snprintf(action_list + offset,
					       10000 - offset, ",");
				offset += len;
			}
		}
		printf("#define FOR_EACH_%i(action,%s)%s\n", i, param_list,
		       action_list);
	}
	return 0;
}
