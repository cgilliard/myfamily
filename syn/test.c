#include <criterion/criterion.h>
#include <syn/syn.h>
#include <util/constants.h>

Test(test, syn) {
	StateMachine sm;
	int value;

	value = build_state("./resources/syn_basic.fam", &sm, 0);
	printf("summary:\n");
	for(int i=0; i<sm.class_count; i++) {
		if(sm.class_array[i].impl_name != NULL) {
			printf("class='%s'::'%s'\n", sm.class_array[i].name, sm.class_array[i].impl_name);
		} else {
			printf("class='%s'\n", sm.class_array[i].name);
		}
		for(int j=0; j<sm.class_array[i].fn_count; j++) {
			printf(
				"   function %s %s(); is_implemented=%i\n",
				sm.class_array[i].fn_array[j].name,
				sm.class_array[i].fn_array[j].ret_type,
				sm.class_array[i].fn_array[j].is_implemented
			);
		}
	}
}
