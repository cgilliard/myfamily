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
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <util/rbtree.h>

_Thread_local RBTree tl_faml_rbtree = {.impl = null};
_Thread_local u64 next_local_id = 0;
volatile atomic_ullong next_global_id = 0;
pthread_mutex_t global_id_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct LookupTableEntry {
	FamlObjVisibility visibility;
	FatPtr name;
	FatPtr sub_table;
} LookupTableEntry;

typedef struct LookupTable {
	u32 size;
	u32 capacity;
	FatPtr entries;
} LookupTable;

typedef struct FamlPrototypeImpl {
	bool is_dynamic;
	bool send;
	bool sync;
	FamlType type;
	FatPtr data;
	RBTree tree;
} FamlPrototypeImpl;

typedef struct FamlObjImpl {
	FamlPrototype *proto;
	u64 id;
} FamlObjImpl;

typedef struct FamlObjKey {
	u64 namespace_id;
	FatPtr name;
} FamlObjKey;

// u128 is the largest type
#define FAML_OBJ_VALUE_SIZE sizeof(u128)

typedef struct FamlObjValueObj {
	u32 insertion_index;
} FamlObjValueObj;

typedef struct FamlObjValueU64 {
	u64 value;
} FamlObjValueU64;

typedef struct FamlObjValueU8 {
	u8 value;
} FamlObjValueU8;

typedef struct FamlObjValueI64 {
	i64 value;
} FamlObjValueI64;

typedef struct FamlObjValueI32 {
	i32 value;
} FamlObjValueI32;

void cleanup_faml_data() {
	if (RBTreeIsInit(tl_faml_rbtree)) {
		rbtree_cleanup(&tl_faml_rbtree);
	}
}

int faml_obj_key_compare(const void *v1, const void *v2) {
	const u64 *k1 = v1;
	const u64 *k2 = v2;
	if (*k1 < *k2)
		return -1;
	else if (*k1 > *k2)
		return 1;
	return 0;
}

int tl_faml_rbtree_init() {
	{
		ChainGuard _ = set_thread_local_slab_allocator();
		rbtree_create(&tl_faml_rbtree, sizeof(FamlObjKey), sizeof(u64), &faml_obj_key_compare,
					  false);
	}
	return 0;
}

int global_faml_init() {
	if (atomic_load(&next_global_id) == 0) {
		pthread_mutex_lock(&global_id_mutex);
		if (atomic_load(&next_global_id) == 0) {
			printf("global_faml_init\n");
			atomic_store(&next_global_id, 1);
		}
		pthread_mutex_unlock(&global_id_mutex);
	}
	return 0;
}

u64 faml_get_next_global_id() {
	return atomic_fetch_add(&next_global_id, 2);
}

u64 faml_get_next_local_id() {
	u64 ret = next_local_id;
	next_local_id += 2;
	return ret;
}

int faml_check_init() {
	printf("faml_check_init\n");
	if (!RBTreeIsInit(tl_faml_rbtree)) {
		if (tl_faml_rbtree_init()) {
			return -1;
		}
	}

	if (global_faml_init())
		return -1;
	return 0;
}

void famlproto_cleanup(FamlPrototype *ptr) {
	if (!nil(ptr->impl)) {

		FamlPrototypeImpl *impl = $Ref(&ptr->impl);
		ChainGuard _ = ChainSend(true);
		if (!nil(impl->data))
			chain_free(&impl->data);
		chain_free(&ptr->impl);
	}
}

int faml_prototype_create(FamlPrototype *proto, bool send, bool sync, bool is_dynamic) {
	if (faml_check_init())
		return -1;

	if (sync && !send) {
		fam_err = IllegalState;
		return -1;
	}

	FamlPrototypeImpl *impl;
	{
		ChainGuard _ = ChainSend(true);
		if (chain_malloc(&proto->impl, sizeof(FamlPrototypeImpl))) {
			proto->impl = null;
			return -1;
		}
		impl = $Ref(&proto->impl);
	}
	impl->send = send;
	impl->sync = sync;
	impl->is_dynamic = is_dynamic;
	impl->type = FamlTypeUnknown;
	impl->data = null;
	impl->tree = INIT_RBTREE;

	return 0;
}

int faml_prototype_set_u8(FamlPrototype *proto, u8 value) {
	FamlPrototypeImpl *impl = $Ref(&proto->impl);
	if (impl->type != FamlTypeUnknown) {
		fam_err = AlreadyInitialized;
		return -1;
	}
	impl->type = FamlTypeU8;
	{
		ChainGuard _ = ChainSend(true); // so we can pass to threads
		if (chain_malloc(&impl->data, sizeof(FamlObjValueU8)))
			return -1;
	}

	FamlObjValueU8 *vu8 = $Ref(&impl->data);
	vu8->value = value;

	return 0;
}
int faml_prototype_set_u64(FamlPrototype *proto, u64 value) {
	FamlPrototypeImpl *impl = $Ref(&proto->impl);
	if (impl->type != FamlTypeUnknown) {
		fam_err = AlreadyInitialized;
		return -1;
	}
	impl->type = FamlTypeU64;

	{
		ChainGuard _ = ChainSend(true); // so we can pass to threads
		if (chain_malloc(&impl->data, sizeof(FamlObjValueU64)))
			return -1;
	}

	FamlObjValueU8 *vu64 = $Ref(&impl->data);
	vu64->value = value;

	return 0;
}
int faml_prototype_set_i64(FamlPrototype *proto, i64 value) {
	FamlPrototypeImpl *impl = $Ref(&proto->impl);
	if (impl->type != FamlTypeUnknown) {
		fam_err = AlreadyInitialized;
		return -1;
	}
	impl->type = FamlTypeI32;

	{
		ChainGuard _ = ChainSend(true); // so we can pass to threads
		if (chain_malloc(&impl->data, sizeof(FamlObjValueI64)))
			return -1;
	}

	FamlObjValueU8 *vi64 = $Ref(&impl->data);
	vi64->value = value;
	return 0;
}
int faml_prototype_set_i32(FamlPrototype *proto, i32 value) {
	FamlPrototypeImpl *impl = $Ref(&proto->impl);
	if (impl->type != FamlTypeUnknown) {
		fam_err = AlreadyInitialized;
		return -1;
	}
	impl->type = FamlTypeI32;

	{
		ChainGuard _ = ChainSend(true); // so we can pass to threads
		if (chain_malloc(&impl->data, sizeof(FamlObjValueI32)))
			return -1;
	}

	FamlObjValueU8 *vi32 = $Ref(&impl->data);
	vi32->value = value;
	return 0;
}

int faml_prototype_put_u8(FamlPrototype *proto, const char *key, u8 value,
						  FamlObjVisibility visibility) {
	FamlPrototypeImpl *impl = $Ref(&proto->impl);
	if (impl->type != FamlTypeUnknown || impl->type != FamlTypeObj) {
		fam_err = AlreadyInitialized;
		return -1;
	}

	impl->type = FamlTypeObj;

	if (!RBTreeIsInit(impl->tree)) {
		if (rbtree_create(&impl->tree, sizeof(FamlObjKey), FAML_OBJ_VALUE_SIZE,
						  faml_obj_key_compare, impl->send))
			return -1;
	}

	FamlObjKey ku8;
	ku8.namespace_id = 0;
	{
		ChainGuard _ =
			ChainSend(true); // always use a 'send' for this so we can deallocate in a new thread.
	}

	//	rbtree_insert(&impl->tree, ku8, vu8);

	return 0;
}
int faml_prototype_put_u64(FamlPrototype *proto, const char *key, u64 value,
						   FamlObjVisibility visibility) {
	return 0;
}
int faml_prototype_put_i32(FamlPrototype *proto, const char *key, i32 value,
						   FamlObjVisibility visibility) {
	return 0;
}
int faml_prototype_put_i64(FamlPrototype *proto, const char *key, i64 value,
						   FamlObjVisibility visibility) {
	return 0;
}

void famlobj_cleanup(FamlObj *obj) {
	if (!nil(obj->impl)) {
		FamlObjImpl *impl = $Ref(&obj->impl);
		bool send = false;
		if (impl->proto && !nil(impl->proto->impl)) {
			FamlPrototypeImpl *pimpl = $Ref(&impl->proto->impl);
			send = pimpl->send;
		}
		ChainGuard _ = ChainSend(send);
		chain_free(&obj->impl);
	}
}

int faml_build_obj(FamlObj *obj, FamlPrototype *proto, bool send, bool sync) {
	printf("sz(famlobjimpl)=%lu key=%lu\n", sizeof(FamlObjImpl), sizeof(FamlObjKey));
	printf("sz(FamlObjValueObj)=%lu,FamlObjValueU64=%lu,u8=%lu\n", sizeof(FamlObjValueObj),
		   sizeof(FamlObjValueU64), sizeof(FamlObjValueU8));
	printf("sz(FamlObjKey)=%lu\n", sizeof(FamlObjKey));
	FamlObjImpl *impl;
	{
		ChainGuard _ = ChainSend(send);
		if (chain_malloc(&obj->impl, sizeof(FamlObjImpl))) {
			obj->impl = null;
			return -1;
		}
		impl = $Ref(&obj->impl);
	}
	impl->proto = proto;

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
	printf("faml put u32\n");

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
