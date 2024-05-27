// Copyright (c) 2024, The MyFamily Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syn/type.h>

int init_type(
        Type *type,
        char *name,
        int is_ref,
        int is_mut,
        ArrayTypeInfo *array_type_info,
        TupleTypeInfo *tuple_type_info,
        SliceTypeInfo *slice_type_info
) {
	int non_null_count = 0;
	type->type = TypeRegular;
	if(slice_type_info) {
		type->type = TypeSlice;
		non_null_count += 1;
	}
	if(tuple_type_info) {
		type->type = TypeTuple;
		non_null_count += 1;
	}
	if(array_type_info) {
		type->type = TypeArray;
		non_null_count += 1;
	}

	if(non_null_count > 1) {
		fputs(
		"ERROR: no more than one of ArrayTypeInfo, TupleTypeInfo, SliceTypeInfo must be specified.\n",
		stderr
		);
		return -1;
	}

	if(name == NULL) {
		type->name = NULL;
	} else {
		type->name = malloc(sizeof(char) * (strlen(name) + 1));
	
		if(type->name == NULL) {
			fputs("ERROR: could not allocate memory to init_type", stderr);
			return -1;
		}
		strcpy(type->name, name);
	}

	if(array_type_info) {
		type->array_type_info = malloc(sizeof(ArrayTypeInfo));
		if(type->array_type_info == NULL) {
			free(type->name);
			fputs("ERROR: could not allocate memory to init_type", stderr);
			return -1;
		}
		memcpy(type->array_type_info, array_type_info, sizeof(ArrayTypeInfo));
	} else {
		type->array_type_info = NULL;
	}
	if(tuple_type_info) {
		type->tuple_type_info = malloc(sizeof(TupleTypeInfo));
		if(type->tuple_type_info == NULL) {
                        free(type->name);
                        fputs("ERROR: could not allocate memory to init_type", stderr);
                        return -1;
                }
		memcpy(type->tuple_type_info, tuple_type_info, sizeof(TupleTypeInfo));
	} else {
		type->tuple_type_info = NULL;
	}
	if(slice_type_info) {
		type->slice_type_info = malloc(sizeof(SliceTypeInfo));
		if(type->slice_type_info == NULL) {
                        free(type->name);
                        fputs("ERROR: could not allocate memory to init_type", stderr);
                        return -1;
                }
		memcpy(type->slice_type_info, slice_type_info, sizeof(SliceTypeInfo));
	} else {
		type->slice_type_info = NULL;
	}

	type->is_mut = is_mut;
	type->is_ref = is_ref;

	return 0;
}

void free_type(Type *type) {
	if(type->array_type_info) {
		free(type->array_type_info);
	}
        if(type->tuple_type_info) {
		free(type->tuple_type_info);
	}
        if(type->slice_type_info) {
		free(type->slice_type_info);
	}
	free(type->name);
}

int init_array_type_info(ArrayTypeInfo *arr_type, Type *type, int len) {
	arr_type->type = malloc(sizeof(Type));
	if(arr_type->type == NULL) {
		fputs("ERROR: could not allocate memory to init_array_type_info", stderr);
		return -1;
	}
	memcpy(arr_type->type, type, sizeof(Type));
	arr_type->len = len;
	return 0;
}

void free_array_type_info(ArrayTypeInfo *arr_type) {
	free(arr_type->type);
}

int init_slice_type_info(SliceTypeInfo *slice_type, Type *type) {
	slice_type->type = malloc(sizeof(Type));
	if(slice_type->type == NULL) {
                fputs("ERROR: could not allocate memory to init_slice_type_info", stderr);
                return -1;
        }
        memcpy(slice_type->type, type, sizeof(Type));
	return 0;
}
void free_slice_type_info(SliceTypeInfo *slice_type) {
	free(slice_type->type);
}

int init_tuple_type_info(TupleTypeInfo *tuple_type, Type *type, int len) {
	tuple_type->type = malloc(sizeof(Type) * len);
	if(tuple_type->type == NULL) {
		fputs("ERROR: could not allocate memory to init_tuple_type_info", stderr);
		return -1;
	}
	memcpy(tuple_type->type, type, sizeof(Type) * len);
	tuple_type->len = len;
        return 0;
}

void free_tuple_type_info(TupleTypeInfo *tuple_type) {
	free(tuple_type->type);
}
