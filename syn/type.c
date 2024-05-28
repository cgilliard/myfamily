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

int copy_type(Type *dst, Type *src);

int copy_array_type_info(ArrayTypeInfo *dst, ArrayTypeInfo *src) {
	if(src != NULL) {
		dst->len = src->len;
		dst->type = malloc(sizeof(Type));
                if(dst->type == NULL)
                        return -1;

		if(copy_type(dst->type, src->type))
			return -1;
	}
	return 0;
}

int copy_tuple_type_info(TupleTypeInfo *dst, TupleTypeInfo *src) {
	if(src != NULL) {
                dst->len = src->len;
                dst->type = malloc(src->len * sizeof(Type));
                if(dst->type == NULL)
                        return -1;

		for(int i=0; i<src->len; i++) {
                	if(copy_type(&dst->type[i], &src->type[i]))
                        	return -1;
		}
        }
        return 0;
}

int copy_slice_type_info(SliceTypeInfo *dst, SliceTypeInfo *src) {
	if(src != NULL) {
                dst->type = malloc(sizeof(Type));
                if(dst->type == NULL)
                        return -1;

                if(copy_type(dst->type, src->type))
                        return -1;
        }
        return 0;
}

int copy_type(Type *dst, Type *src) {
	if(src->array_type_info) {
		dst->array_type_info = malloc(sizeof(ArrayTypeInfo));
		if(copy_array_type_info(dst->array_type_info, src->array_type_info))
			return -1;
	} else {
		dst->array_type_info = NULL;
	}
	if(src->slice_type_info) {
		dst->slice_type_info = malloc(sizeof(SliceTypeInfo));
		if(copy_slice_type_info(dst->slice_type_info, src->slice_type_info))
			return -1;
	} else {
		dst->slice_type_info = NULL;
	}
	if(src->tuple_type_info) {
		dst->tuple_type_info = malloc(sizeof(TupleTypeInfo));
                if(copy_tuple_type_info(dst->tuple_type_info, src->tuple_type_info))
			return -1;
        } else {
		dst->tuple_type_info = NULL;
	}

	if(src->name != NULL) {
		dst->name = malloc(sizeof(char) * (strlen(src->name) + 1));
		if(dst->name == NULL)
			return -1;
		strcpy(dst->name, src->name);
	} else {
		dst->name = NULL;
	}
	dst->is_ref = src->is_ref;
	dst->is_mut = src->is_mut;
	dst->type = src->type;
	return 0;
}

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

	if(array_type_info != NULL) {
		type->array_type_info = malloc(sizeof(ArrayTypeInfo));
                if(type->array_type_info == NULL) {
                        free(type->name);
                        fputs("ERROR: could not allocate memory to init_type", stderr);
                        return -1;
                }
		copy_array_type_info(type->array_type_info, array_type_info);
	}
	else
		type->array_type_info = NULL;
	if(tuple_type_info) {
		type->tuple_type_info = malloc(sizeof(TupleTypeInfo));
		if(type->tuple_type_info == NULL) {
                        free(type->name);
                        fputs("ERROR: could not allocate memory to init_type", stderr);
                        return -1;
                }
		copy_tuple_type_info(type->tuple_type_info, tuple_type_info);
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
		copy_slice_type_info(type->slice_type_info, slice_type_info);
	} else {
		type->slice_type_info = NULL;
	}

	type->is_mut = is_mut;
	type->is_ref = is_ref;

	return 0;
}

void free_type(Type *type) {
	if(type->array_type_info) {
		free_array_type_info(type->array_type_info);
		free(type->array_type_info);
	}
        if(type->tuple_type_info) {
		free_tuple_type_info(type->tuple_type_info);
		free(type->tuple_type_info);
	}
        if(type->slice_type_info) {
		free_slice_type_info(type->slice_type_info);
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
	copy_type(arr_type->type, type);
	arr_type->len = len;
	return 0;
}

void free_array_type_info(ArrayTypeInfo *arr_type) {
	if(arr_type->type != NULL) {
		free_type(arr_type->type);
		free(arr_type->type);
	}
}

int init_slice_type_info(SliceTypeInfo *slice_type, Type *type) {
	slice_type->type = malloc(sizeof(Type));
	if(slice_type->type == NULL) {
                fputs("ERROR: could not allocate memory to init_slice_type_info", stderr);
                return -1;
        }
	copy_type(slice_type->type, type);
	return 0;
}
void free_slice_type_info(SliceTypeInfo *slice_type) {
	if(slice_type->type != NULL) {
                free_type(slice_type->type);
                free(slice_type->type);
        }  
}

int init_tuple_type_info(TupleTypeInfo *tuple_type, Type *type, int len) {
	tuple_type->type = malloc(sizeof(Type) * len);
	if(tuple_type->type == NULL) {
		fputs("ERROR: could not allocate memory to init_tuple_type_info", stderr);
		return -1;
	}
	for(int i=0; i<len; i++) {
		copy_type(&tuple_type->type[i], &type[i]);
	}
	tuple_type->len = len;
        return 0;
}

void free_tuple_type_info(TupleTypeInfo *tuple_type) {
	if(tuple_type->type != NULL) {
                free_type(tuple_type->type);
                free(tuple_type->type);
        }
}
