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

#include <base/resources.h>
#include <faml/parser.h>

typedef enum FamlType {
	FamlTypeObj,
	FamlTypeArray,
	FamlTypeTuple,
	FamlTypeI8,
	FamlTypeI16,
} FamlType;

typedef struct FamlObjImpl {
	FamlType *types;
	void *data;
} FamlObjImpl;

void famlobj_cleanup(FamlObj *obj) {
	if (obj->opaque) {
		myfree(obj->opaque);
		obj->opaque = NULL;
	}
}

int famlobj_init(FamlObj *obj) {
	obj->opaque = mymalloc(sizeof(FamlObjImpl));
	if (obj->opaque == NULL)
		return -1;
	return 0;
}
int famlobj_add_i8(FamlObj *obj, const char *key, const i8 value) {
	return 0;
}
int famlobj_add_i16(FamlObj *obj, const char *key, const i16 value) {
	return 0;
}
