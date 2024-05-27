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

#ifndef SYN_TYPES
#define SYN_TYPES

typedef struct Type Type;

enum TypeEnum {
	TypeRegular = 1,
	TypeArray = 2,
	TypeTuple = 3,
	TypeSlice = 4,
};
typedef enum TypeEnum TypeEnum;

struct ArrayTypeInfo {
	int len;
	Type *type;
};
typedef struct ArrayTypeInfo ArrayTypeInfo;

struct TupleTypeInfo {
	int len;
	Type *type;
};
typedef struct TupleTypeInfo TupleTypeInfo;

struct SliceTypeInfo {
	Type *type;
};
typedef struct SliceTypeInfo SliceTypeInfo;

struct Type {
        char *name;
        int is_ref;
        int is_mut;
	TypeEnum type;
	ArrayTypeInfo *array_type_info;
	TupleTypeInfo *tuple_type_info;
	SliceTypeInfo *slice_type_info;
};

int init_type(
        Type *type,
        char *name,
        int is_ref,
        int is_mut,
        ArrayTypeInfo *array_type_info,
        TupleTypeInfo *tuple_type_info,
        SliceTypeInfo *slice_type_info
);
void free_type(Type *type);

int init_array_type_info(ArrayTypeInfo *arr_type, Type *type, int len);
void free_array_type_info(ArrayTypeInfo *arr_type);

int init_slice_type_info(SliceTypeInfo *slice_type, Type *type);
void free_slice_type_info(SliceTypeInfo *slice_type);

int init_tuple_type_info(TupleTypeInfo *tuple_type, Type *type, int len);
void free_tuple_type_info(TupleTypeInfo *tuple_type);

#endif /* SYN_TYPES */
