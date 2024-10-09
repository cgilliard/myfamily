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

#include <base/chain_alloc.h>
#include <base/fam_err.h>
#include <base/resources.h>
#include <faml/parser.h>
#include <stdio.h>
#include <string.h>
#include <util/rbtree.h>

typedef struct LookupTableEntry {
	FamlObjVisibility visibility;
	FatPtr name;
	FatPtr sub_table;
} LookupTableEntry;

typedef struct FamlPrototypeImpl {
	bool is_dynamic;
	FatPtr lookup_table;
	u32 size;
	u32 capacity;
} FamlPrototypeImpl;

void famlproto_cleanup(FamlPrototype *ptr) {
}

int faml_prototype_create(FamlPrototype *proto, const char *name) {
	return 0;
}

typedef struct FamlObjImpl {
	FamlPrototype *proto;
	FamlType type;
	bool send;
	bool sync;
	FatPtr data;
} FamlObjImpl;

typedef struct FamlObjKey {
	u32 variant_id;
	FatPtr name;
} FamlObjKey;

typedef struct FamlObjValueObj {
	RBTree names;
	RBTree indices;
	u32 insertion_index;
} FamlObjValueObj;

typedef struct FamlObjValueU64 {
	u64 value;
} FamlObjValueU64;

typedef struct FamlObjValueU8 {
	u8 value;
} FamlObjValueU8;

int faml_obj_key_compare(const void *v1, const void *v2) {
	const u64 *k1 = v1;
	const u64 *k2 = v2;
	if (*k1 < *k2)
		return -1;
	else if (*k1 > *k2)
		return 1;
	return 0;
}

void famlobj_cleanup(FamlObj *obj) {
	if (!nil(obj->impl)) {
		FamlObjImpl *impl = $Ref(&obj->impl);
		ChainGuard _ = ChainSend(impl->send);
		if (!nil(impl->data)) {
			chain_free(&impl->data);
		}

		chain_free(&obj->impl);
	}
}

int faml_build_obj(FamlObj *obj, FamlPrototype *proto, bool send, bool sync) {
	printf("sz(famlobjimpl)=%lu key=%lu\n", sizeof(FamlObjImpl), sizeof(FamlObjKey));
	printf("sz(FamlObjValueObj)=%lu,FamlObjValueU64=%lu,u8=%lu\n", sizeof(FamlObjValueObj),
		   sizeof(FamlObjValueU64), sizeof(FamlObjValueU8));
	FamlObjImpl *impl;
	FamlObjValueObj *data;
	{
		ChainGuard _ = ChainSend(send);
		if (chain_malloc(&obj->impl, sizeof(FamlObjImpl))) {
			obj->impl = null;
			return -1;
		}
		impl = $Ref(&obj->impl);
		if (chain_malloc(&impl->data, sizeof(FamlObjValueObj))) {
			chain_free(&obj->impl);
			obj->impl = null;
			impl->data = null;
			return -1;
		}
		data = $Ref(&impl->data);
	}
	impl->send = send;
	impl->proto = proto;
	data->insertion_index = 0;

	return 0;
}

int famlobj_put_u8(FamlObj *obj, const char *name, const u8 v) {
	if (obj == NULL || name == NULL) {
		fam_err = IllegalArgument;
		return -1;
	}
	FamlObjImpl *impl = $Ref(&obj->impl);

	return 0;
}

int famlobj_put_u64(FamlObj *obj, const char *name, const u64 v) {
	if (obj == NULL || name == NULL) {
		fam_err = IllegalArgument;
		return -1;
	}

	FamlObjImpl *impl = $Ref(&obj->impl);

	return 0;
}

int faml_put_i32(FamlObj *obj, const char *name, const i32 v) {
	if (obj == NULL || name == NULL) {
		fam_err = IllegalArgument;
		return -1;
	}
	if (nil(obj->impl)) {
		fam_err = InitErr;
		return -1;
	}

	FamlObjImpl *impl = $Ref(&obj->impl);
	if (impl == NULL) {
		fam_err = InitErr;
		return -1;
	}

	FamlObjValueObj *data = $Ref(&impl->data);
	u32 insertion_index = data->insertion_index;

	data->insertion_index++;

	printf("faml put i32 ii=%u\n", insertion_index);

	return 0;
}

int faml_put_i64(FamlObj *obj, const char *name, const i64 v) {
	printf("faml put i64\n");
	return 0;
}

int faml_put_obj(FamlObj *obj, const char *name, const FamlObj *v) {
	printf("faml put obj\n");
	return 0;
}
