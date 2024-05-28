#include <criterion/criterion.h>
#include <syn/syn.h>
#include <syn/type.h>
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

Test(test, types) {
	// u32[3] x;
	Type arr;
	// u32 y;
	Type u32_type;
	init_type(
		&u32_type,
		"u32",
		FALSE,
		FALSE,
		NULL,
		NULL,
	       	NULL
	);
	ArrayTypeInfo art;
	init_array_type_info(&art, &u32_type, 3);
	init_type(&arr, NULL, FALSE, FALSE, &art, NULL, NULL);

	cr_assert_eq(strcmp(u32_type.name, "u32"), 0);
	cr_assert_eq(u32_type.is_ref, FALSE);
	cr_assert_eq(u32_type.is_mut, FALSE);
	cr_assert_eq(u32_type.array_type_info, NULL);
	cr_assert_eq(u32_type.type, TypeRegular);

	cr_assert_eq(arr.name, NULL);
        cr_assert_eq(arr.is_ref, FALSE);
        cr_assert_eq(arr.is_mut, FALSE);
        cr_assert_neq(arr.array_type_info, NULL);
        cr_assert_eq(arr.type, TypeArray);
	cr_assert_eq(arr.array_type_info->len, 3);
	cr_assert_eq(strcmp(arr.array_type_info->type->name, "u32"), 0);
	cr_assert_eq(arr.array_type_info->type->is_ref, FALSE);
	cr_assert_eq(arr.array_type_info->type->is_mut, FALSE);
	cr_assert_eq(arr.array_type_info->type->type, TypeRegular);
	cr_assert_eq(arr.array_type_info->type->array_type_info, NULL);

	free_array_type_info(&art);
	free_type(&arr);

	// test slice 
	// [&u64] x;
	Type slice;
	// u64 &y;
	Type u64_type;

	init_type(
                &u64_type,
                "u64",
                TRUE,
                FALSE,
                NULL,
                NULL,
                NULL
        );

	SliceTypeInfo st;
	init_slice_type_info(&st, &u64_type);
	init_type(&slice, NULL, FALSE, FALSE, NULL, NULL, &st);

	cr_assert_eq(strcmp(u64_type.name, "u64"), 0);
        cr_assert_eq(u64_type.is_ref, TRUE);
        cr_assert_eq(u64_type.is_mut, FALSE);
        cr_assert_eq(u64_type.array_type_info, NULL);
        cr_assert_eq(u64_type.type, TypeRegular);

	cr_assert_eq(slice.name, NULL);
        cr_assert_eq(slice.is_ref, FALSE);
        cr_assert_eq(slice.is_mut, FALSE);
        cr_assert_eq(slice.array_type_info, NULL);
	cr_assert_neq(slice.slice_type_info, NULL);
        cr_assert_eq(slice.type, TypeSlice);
        cr_assert_eq(strcmp(slice.slice_type_info->type->name, "u64"), 0);
        cr_assert_eq(slice.slice_type_info->type->is_ref, TRUE);
        cr_assert_eq(slice.slice_type_info->type->is_mut, FALSE);
        cr_assert_eq(slice.slice_type_info->type->type, TypeRegular);
        cr_assert_eq(slice.slice_type_info->type->array_type_info, NULL);
	cr_assert_eq(slice.slice_type_info->type->slice_type_info, NULL);

	free_slice_type_info(&st);
	free_type(&slice);

	// i32 x;	
	Type i32_type;
	init_type(
                &i32_type,
                "i32",
                FALSE,
                FALSE,
                NULL,
                NULL,
                NULL
        );

	// test tuple
        // (&u64, u32, i32)
        Type mytuple;

	TupleTypeInfo tt;
	Type tarr[3];
	tarr[0] = u64_type;
	tarr[1] = u32_type;
	tarr[2] = i32_type;
	init_tuple_type_info(&tt, tarr, 3);

	init_type(&mytuple,
		NULL,
		FALSE,
		FALSE,
		NULL,
		&tt,
		NULL
	);

	cr_assert_eq(mytuple.name, NULL);
        cr_assert_eq(mytuple.is_ref, FALSE);
        cr_assert_eq(mytuple.is_mut, FALSE);
        cr_assert_eq(mytuple.array_type_info, NULL);
        cr_assert_eq(mytuple.slice_type_info, NULL);
	cr_assert_neq(mytuple.tuple_type_info, NULL);
        cr_assert_eq(mytuple.type, TypeTuple);
        cr_assert_eq(strcmp(mytuple.tuple_type_info->type[0].name, "u64"), 0);
        cr_assert_eq(mytuple.tuple_type_info->type[0].is_ref, TRUE);
        cr_assert_eq(mytuple.tuple_type_info->type[0].is_mut, FALSE);
        cr_assert_eq(mytuple.tuple_type_info->type[0].type, TypeRegular);
        cr_assert_eq(mytuple.tuple_type_info->type[0].array_type_info, NULL);
        cr_assert_eq(mytuple.tuple_type_info->type[0].tuple_type_info, NULL);

        cr_assert_eq(strcmp(mytuple.tuple_type_info->type[1].name, "u32"), 0);
        cr_assert_eq(mytuple.tuple_type_info->type[1].is_ref, FALSE);
        cr_assert_eq(mytuple.tuple_type_info->type[1].is_mut, FALSE);
        cr_assert_eq(mytuple.tuple_type_info->type[1].type, TypeRegular);
        cr_assert_eq(mytuple.tuple_type_info->type[1].array_type_info, NULL);
        cr_assert_eq(mytuple.tuple_type_info->type[1].tuple_type_info, NULL);

        cr_assert_eq(strcmp(mytuple.tuple_type_info->type[2].name, "i32"), 0);
        cr_assert_eq(mytuple.tuple_type_info->type[2].is_ref, FALSE);
        cr_assert_eq(mytuple.tuple_type_info->type[2].is_mut, FALSE);
        cr_assert_eq(mytuple.tuple_type_info->type[2].type, TypeRegular);
        cr_assert_eq(mytuple.tuple_type_info->type[2].array_type_info, NULL);
        cr_assert_eq(mytuple.tuple_type_info->type[2].tuple_type_info, NULL);

	cr_assert_eq(mytuple.tuple_type_info->len, 3);

	free_type(&mytuple);
	free_tuple_type_info(&tt);

	free_type(&u32_type);
	free_type(&u64_type);
	free_type(&i32_type);
}

