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

#ifndef _FAML_FAML__
#define _FAML_FAML__

#include <base/slabs.h>
#include <base/types.h>

typedef enum FamlType {
	FamlTypeUnknown,
	FamlTypeObj,
	FamlTypeArray,
	FamlTypeTuple,
	FamlTypeEnum,
	FamlTypeU8,
	FamlTypeU64,
	FamlTypeI32,
	FamlTypeI64,
} FamlType;

typedef enum FamlObjVisibility {
	FamlObjVisibilityPublic,
	FamlObjVisibilityPrivate
} FamlObjVisibility;

typedef struct FamlPrototypeNc {
	FatPtr impl;
} FamlPrototypeNc;

#ifdef TEST // cleanup function for tests
void cleanup_faml_data();
#endif // TEST

void famlproto_cleanup(FamlPrototypeNc *ptr);

#define FamlPrototype                                                                              \
	FamlPrototypeNc __attribute__((warn_unused_result, cleanup(famlproto_cleanup)))

// FAML Object type
typedef struct FamlObjNc {
	FatPtr impl;
} FamlObjNc;

void famlobj_cleanup(FamlObjNc *ptr);

#define FamlObj FamlObjNc __attribute__((warn_unused_result, cleanup(famlobj_cleanup)))

// create a FAML prototype object
int faml_prototype_create(FamlPrototype *proto, bool send, bool sync, bool is_dynamic);

// set functions allow setting to a primitive type

// set the underlying type of this prototype to u8 with a default value of 'value'
int faml_prototype_set_u8(FamlPrototype *proto, u8 value);
// set the underlying type of this prototype to u64 with a default value of 'value'
int faml_prototype_set_u64(FamlPrototype *proto, u64 value);
// set the underlying type of this prototype to i32 with a default value of 'value'
int faml_prototype_set_i32(FamlPrototype *proto, i32 value);
// set the underlying type of this prototype to i64 with a default value of 'value'
int faml_prototype_set_i64(FamlPrototype *proto, i64 value);
#define faml_prototype_set(obj, name, value)                                                       \
	_Generic((value),                                                                              \
		u8: faml_prototype_set_u8,                                                                 \
		u64: faml_prototype_set_u64,                                                               \
		i32: faml_prototype_set_i32,                                                               \
		i64: faml_prototype_set_i64,                                                               \
		default: faml_prototype_set_i32)(obj, name, value)
// note: continue for additional primitive types

// set the underlying type of this prototype to a tuple
int faml_prototype_set_tuple(FamlPrototype *proto);
// set the underlying type of this prototype to a fixed size array of the specified type
int faml_prototype_set_fixed_array(FamlPrototype *proto, const FamlPrototype *array_type, u64 len);
// set the underlying type of this prototype to a dynamic array of the specified type
int faml_prototype_set_dynamic_array(FamlPrototype *proto, const FamlPrototype *array_type);

// add a type to the tuple's list of elements
int faml_prototype_add_to_tuple_type(FamlPrototype *proto, const FamlPrototype *element);
// set the default value of a fixed array for the specified index
int faml_prototype_set_array_default(FamlPrototype *proto, u64 index, const FamlPrototype *element);

// add to the key/value for this type
int faml_prototype_put_u8(FamlPrototype *proto, const char *key, u8 value,
						  FamlObjVisibility visibility);
int faml_prototype_put_u64(FamlPrototype *proto, const char *key, u64 value,
						   FamlObjVisibility visibility);
int faml_prototype_put_i32(FamlPrototype *proto, const char *key, i32 value,
						   FamlObjVisibility visibility);
int faml_prototype_put_i64(FamlPrototype *proto, const char *key, i64 value,
						   FamlObjVisibility visibility);
// note: continue for additional primitive types
int faml_prototype_put_obj(FamlPrototype *proto, const char *key, const FamlPrototype *value);
#define faml_prototype_put(obj, name, value)                                                       \
	_Generic((value),                                                                              \
		u8: faml_prototype_put_u8,                                                                 \
		u64: faml_prototype_put_u64,                                                               \
		i32: faml_prototype_put_i32,                                                               \
		i64: faml_prototype_put_i64,                                                               \
		FamlObj *: faml_put_obj,                                                                   \
		default: faml_prototype_put_i32)(obj, name, value)

// build a faml object based on specified prototype. If proto is NULL a default prototype is used
// for the object.
int faml_build_obj(FamlObj *obj, FamlPrototype *proto, bool send, bool sync);

int faml_build_u8(FamlObj *obj, u8 value);
int faml_build_u64(FamlObj *obj, u64 value);
int faml_build_i32(FamlObj *obj, i32 value);
int faml_build_164(FamlObj *obj, i64 value);

// set value of the specified k/v pair using RBTree implementation. If the object was created using
// a prototype, 'strict' policies are in place allowing only key/value pairs that are public members
// of the prototype to be set.
int faml_put_u8(FamlObj *obj, const char *name, const u8 value);
int faml_put_u64(FamlObj *obj, const char *name, const u64 value);
int faml_put_i32(FamlObj *obj, const char *name, const i32 value);
int faml_put_i64(FamlObj *obj, const char *name, const i64 v);
int faml_put_obj(FamlObj *obj, const char *name, const FamlObj *value);
#define faml_put(obj, name, value)                                                                 \
	_Generic((value),                                                                              \
		u8: faml_put_u8,                                                                           \
		u64: faml_put_u64,                                                                         \
		i32: faml_put_i32,                                                                         \
		i64: faml_put_i64,                                                                         \
		FamlObj *: faml_put_obj,                                                                   \
		default: faml_put_i32)(obj, name, value)

/*
// Implementation notes: When adding a type via the 'push' mechanism, type must be checked. Once
// the first type is set, if another type is pushed, the result will be an error.
int famlarray_build(FamlArray *arr, bool send);
int famlarray_push_obj(FamlArray *arr, const FamlObj *value);
int famlarray_push_array(FamlArray *arr, const FamlArray *value);
int famlarray_push_tuple(FamlArray *arr, const FamlTuple *value);
int famlarray_push_i8(FamlArray *arr, const i8 value);
int famlarray_push_i16(FamlArray *arr, const i16 value);
int famlarray_push_i32(FamlArray *arr, const i32 value);
int famlarray_push_i64(FamlArray *arr, const i64 value);
int famlarray_push_i128(FamlArray *arr, const i128 value);
int famlarray_push_u8(FamlArray *arr, const u8 value);
int famlarray_push_u16(FamlArray *arr, const u16 value);
int famlarray_push_u32(FamlArray *arr, const u32 value);
int famlarray_push_u64(FamlArray *arr, const u64 value);
int famlarray_push_u128(FamlArray *arr, const u128 value);
int famlarray_push_f32(FamlArray *arr, const f32 value);
int famlarray_push_f64(FamlArray *arr, const f64 value);
int famlarray_push_bool(FamlArray *arr, const bool value);
int famlarray_push_string(FamlArray *arr, const char *value);

// Implementation notes:
// These functions are for definitions which have length and perhaps default
// MyType {
//     g = [u8 | 20]; // statically sized array size 20 defaults to 0
//     h = [u8]; // dynamically sized array must be allocated using $Alloc
//     i = [u8 | 20 | 10] // statically sized array size 20 with defaults to 10
// }
// multi dimensional array syntax: x = [[u8 | 20] | 10] // this is a statically sized array of 10
// elements. Each element is itself an array of 20 u8. This can be created by using the
// famlarray_with_defaults_array function.
int famlarray_with_defaults_obj(FamlArray *arr, const FamlObj *default_value, u64 len);
int famlarray_with_defaults_array(FamlArray *arr, const FamlArray *default_value, u64 len);
int famlarray_with_defaults_i8(FamlArray *arr, i8 default_value, u64 len);
int famlarray_with_defaults_i16(FamlArray *arr, i16 default_value, u64 len);
int famlarray_with_defaults_i32(FamlArray *arr, i32 default_value, u64 len);
int famlarray_with_defaults_i64(FamlArray *arr, i64 default_value, u64 len);
int famlarray_with_defaults_i128(FamlArray *arr, i128 default_value, u64 len);
int famlarray_with_defaults_u8(FamlArray *arr, u8 default_value, u64 len);
int famlarray_with_defaults_u16(FamlArray *arr, u16 default_value, u64 len);
int famlarray_with_defaults_u32(FamlArray *arr, u32 default_value, u64 len);
int famlarray_with_defaults_u64(FamlArray *arr, u64 default_value, u64 len);
int famlarray_with_defaults_u128(FamlArray *arr, u128 default_value, u64 len);
int famlarray_with_defaults_f32(FamlArray *arr, f32 default_value, u64 len);
int famlarray_with_defaults_f64(FamlArray *arr, f64 default_value, u64 len);
int famlarray_with_defaults_bool(FamlArray *arr, bool default_value, u64 len);
int famlarray_with_defaults_string(FamlArray *arr, char *default_value, u64 len);

// Implementation notes: Tuples can have any type so no type checking is necessary here.
int famltuple_build(FamlArray *arr, bool send);
int famltuple_push_obj(FamlArray *arr, const FamlObj *value);
int famltuple_push_array(FamlArray *arr, const FamlArray *value);
int famltuple_push_tuple(FamlArray *arr, const FamlTuple *value);
int famltuple_push_i8(FamlArray *arr, const i8 value);
int famltuple_push_i16(FamlArray *arr, const i16 value);
int famltuple_push_i32(FamlArray *arr, const i32 value);
int famltuple_push_i64(FamlArray *arr, const i64 value);
int famltuple_push_i128(FamlArray *arr, const i128 value);
int famltuple_push_u8(FamlArray *arr, const u8 value);
int famltuple_push_u16(FamlArray *arr, const u16 value);
int famltuple_push_u32(FamlArray *arr, const u32 value);
int famltuple_push_u64(FamlArray *arr, const u64 value);
int famltuple_push_u128(FamlArray *arr, const u128 value);
int famltuple_push_f32(FamlArray *arr, const f32 value);
int famltuple_push_f64(FamlArray *arr, const f64 value);
int famltuple_push_bool(FamlArray *arr, const bool value);
int famltuple_push_string(FamlArray *arr, const char *value);

// When adding to an object, it acts as an associative array. So you have key/values.
int famlobj_add_obj(FamlObj *obj, const char *key, const FamlObj *value);
int famlobj_add_array(FamlObj *obj, const char *key, const FamlArray *value);
int famlobj_add_tuple(FamlObj *obj, const char *key, const FamlTuple *value);
int famlobj_add_i8(FamlObj *obj, const char *key, const i8 value);
int famlobj_add_i16(FamlObj *obj, const char *key, const i16 value);
int famlobj_add_i32(FamlObj *obj, const char *key, const i32 value);
int famlobj_add_i64(FamlObj *obj, const char *key, const i64 value);
int famlobj_add_i128(FamlObj *obj, const char *key, const i128 value);
int famlobj_add_u8(FamlObj *obj, const char *key, const u8 value);
int famlobj_add_u16(FamlObj *obj, const char *key, const u16 value);
int famlobj_add_u32(FamlObj *obj, const char *key, const u32 value);
int famlobj_add_u64(FamlObj *obj, const char *key, const u64 value);
int famlobj_add_u128(FamlObj *obj, const char *key, const u128 value);
int famlobj_add_f32(FamlObj *obj, const char *key, const f32 value);
int famlobj_add_f64(FamlObj *obj, const char *key, const f64 value);
int famlobj_add_bool(FamlObj *obj, const char *key, const bool value);
int famlobj_add_string(FamlObj *obj, const char *key, const char *value);

int famlobj_as_obj(const FamlObj *obj, char *key, FamlObj *value);
int famlobj_as_tuple(const FamlObj *obj, char *key, FamlTuple *value);
int famlobj_as_array(const FamlObj *obj, char *key, FamlArray *value);
int famlobj_as_i8(const FamlObj *obj, char *key, i8 *value);
int famlobj_as_i16(const FamlObj *obj, char *key, i16 *value);
int famlobj_as_i32(const FamlObj *obj, char *key, i32 *value);
int famlobj_as_i64(const FamlObj *obj, char *key, i64 *value);
int famlobj_as_i128(const FamlObj *obj, char *key, i128 *value);
int famlobj_as_u8(const FamlObj *obj, char *key, u8 *value);
int famlobj_as_u16(const FamlObj *obj, char *key, u16 *value);
int famlobj_as_u32(const FamlObj *obj, char *key, u32 *value);
int famlobj_as_u64(const FamlObj *obj, char *key, u64 *value);
int famlobj_as_u128(const FamlObj *obj, char *key, u128 *value);
int famlobj_as_f32(const FamlObj *obj, char *key, f32 *value);
int famlobj_as_f64(const FamlObj *obj, char *key, f64 *value);
int famlobj_as_bool(const FamlObj *obj, char *key, bool *value);
int famlobj_as_string(const FamlObj *obj, char *key, char *value, u64 limit);

int famlarray_as_obj(const FamlArray *arr, u64 index, FamlObj *value);
int famlarray_as_tuple(const FamlArray *arr, u64 index, FamlTuple *value);
int famlarray_as_array(const FamlArray *arr, u64 index, FamlArray *value);
int famlarray_as_i8(const FamlArray *arr, u64 index, i8 *value);
int famlarray_as_i16(const FamlArray *arr, u64 index, i16 *value);
int famlarray_as_i32(const FamlArray *arr, u64 index, i32 *value);
int famlarray_as_i64(const FamlArray *arr, u64 index, i64 *value);
int famlarray_as_i128(const FamlArray *arr, u64 index, i128 *value);
int famlarray_as_u8(const FamlArray *arr, u64 index, u8 *value);
int famlarray_as_u16(const FamlArray *arr, u64 index, u16 *value);
int famlarray_as_u32(const FamlArray *arr, u64 index, u32 *value);
int famlarray_as_u64(const FamlArray *arr, u64 index, u64 *value);
int famlarray_as_u128(const FamlArray *arr, u64 index, u128 *value);
int famlarray_as_f32(const FamlArray *arr, u64 index, f32 *value);
int famlarray_as_f64(const FamlArray *arr, u64 index, f64 *value);
int famlarray_as_bool(const FamlArray *arr, u64 index, bool *value);
int famlarray_as_string(const FamlArray *arr, u64 index, char *value, u64 limit);

int famltuple_as_obj(const FamlTuple *tuple, u64 index, FamlObj *value);
int famltuple_as_array(const FamlTuple *tuple, u64 index, FamlArray *value);
int famltuple_as_tuple(const FamlTuple *tuple, u64 index, FamlTuple *value);
int famltuple_as_i8(const FamlTuple *tuple, u64 index, i8 *value);
int famltuple_as_i16(const FamlTuple *tuple, u64 index, i16 *value);
int famltuple_as_i32(const FamlTuple *tuple, u64 index, i32 *value);
int famltuple_as_i64(const FamlTuple *tuple, u64 index, i64 *value);
int famltuple_as_i128(const FamlTuple *tuple, u64 index, i128 *value);
int famltuple_as_u8(const FamlTuple *tuple, u64 index, u8 *value);
int famltuple_as_u16(const FamlTuple *tuple, u64 index, u16 *value);
int famltuple_as_u32(const FamlTuple *tuple, u64 index, u32 *value);
int famltuple_as_u64(const FamlTuple *tuple, u64 index, u64 *value);
int famltuple_as_u128(const FamlTuple *tuple, u64 index, u128 *value);
int famltuple_as_f32(const FamlTuple *tuple, u64 index, f32 *value);
int famltuple_as_f64(const FamlTuple *tuple, u64 index, f64 *value);
int famltuple_as_bool(const FamlTuple *tuple, u64 index, bool *value);
int famltuple_as_string(const FamlTuple *tuple, u64 index, char *value, u64 limit);
*/

#endif // _FAML_FAML__
