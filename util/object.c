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

#include <base/bitflags.h>
#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/macro_utils.h>
#include <base/panic.h>
#include <base/resources.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <util/object.h>
#include <util/rbtree.h>
#include <util/rc.h>

pthread_mutex_t global_init_lock = PTHREAD_MUTEX_INITIALIZER;
_Thread_local RBTree *thread_local_rbtree = NULL;
RBTree *global_rbtree = NULL;

_Thread_local u64 thread_local_namespace_next = 0;
atomic_ullong global_namespace_next = 0;

#define INIT_OBJECT_KEY {.key = null}

typedef struct ObjectImpl {
	FatPtr self;
	u64 namespace;
	ObjectType type;
	bool send;
	u32 property_count;
} ObjectImpl;

typedef struct ObjectFlags {
	u8 flags;
} ObjectFlags;

typedef struct ObjectKeyNc {
	FatPtr key;
	u64 namespace;
} ObjectKeyNc;

void object_key_cleanup(ObjectKeyNc *ptr) {
	if (ptr && !nil(ptr->key)) {
		fam_free(&ptr->key);
		ptr->key = null;
	}
}

#define ObjectKey ObjectKeyNc __attribute__((warn_unused_result, cleanup(object_key_cleanup)))

int object_key_for(ObjectKey *objkey, const Object *obj, const char *key) {
	ObjectImpl *impl = $(obj->impl);
	int ret = fam_alloc(&objkey->key, sizeof(char) * (1 + strlen(key)));
	if (ret != 0)
		return -1;

	strcpy($(objkey->key), key);
	objkey->namespace = impl->namespace;

	return ret;
}

typedef struct ObjectValue {
	ObjectType type;
	FatPtr value;
} ObjectValue;

int object_property_name_compare(const void *v1, const void *v2) {
	const ObjectKeyNc *k1 = v1;
	const ObjectKeyNc *k2 = v2;
	if (k1->namespace != k2->namespace) {
		if (k1->namespace < k2->namespace)
			return -1;
		else
			return 1;
	} else
		return strcmp($(k1->key), $(k2->key));
}

RBTree *init_default_rbtree(bool global) {
	RBTreeNc *ret = mymalloc(sizeof(RBTree));
	if (ret) {
		if (rbtree_build(ret, sizeof(ObjectKey), sizeof(ObjectValue), object_property_name_compare,
						 global))
			return NULL;
	}
	return ret;
}

u64 object_namespace_id_next_local() {
	u64 ret = thread_local_namespace_next;
	thread_local_namespace_next++;
	return ret;
}

u64 object_namespace_id_next_global() {
	u64 ret = atomic_fetch_add(&global_namespace_next, 1);
	return ret;
}

int object_init_rbtrees() {
	int ret = 0;
	if (global_rbtree == NULL) {
		if (atomic_load(&global_namespace_next) == 0) {
			pthread_mutex_lock(&global_init_lock);
			if (atomic_load(&global_namespace_next) == 0) {
				global_rbtree = init_default_rbtree(true);
				if (global_rbtree == NULL) {
					ret = -1;
				} else
					atomic_store(&global_namespace_next, 1);
			}
			pthread_mutex_unlock(&global_init_lock);
		}
	}

	if (thread_local_rbtree == NULL) {
		thread_local_rbtree = init_default_rbtree(false);
		if (thread_local_rbtree == NULL)
			ret = -1;
	}

	return ret;
}

#define OBJECT_FLAG_CONSUMED 1
#define OBJECT_FLAG_NO_CLEANUP 2
#define OBJECT_FLAG_SEND 3
#define OBJECT_FLAG_IMUT 4

void object_cleanup_rc(ObjectImpl *impl) {
	printf("rc obj cleanup\n");

	RBTreeNc *tree;
	if (impl->send)
		tree = global_rbtree;
	else
		tree = thread_local_rbtree;

	RBTreeIterator itt;

	ObjectKey start = INIT_OBJECT_KEY;
	start.namespace = impl->namespace;
	fam_alloc(&start.key, 1);
	strcpy($(start.key), "");

	ObjectKey end = INIT_OBJECT_KEY;
	end.namespace = impl->namespace + 1;
	fam_alloc(&end.key, 1);
	strcpy($(end.key), "");

	rbtree_iterator(tree, &itt, &start, true, &end, false);

	printf("property count= %u\n", impl->property_count);
	u64 pc = 1;
	if (impl->property_count > pc)
		pc = impl->property_count;
	FatPtr *properties[pc];
	u64 i = 0;

	loop {
		RbTreeKeyValue kv;
		if (!rbtree_iterator_next(&itt, &kv))
			break;
		ObjectKeyNc *k1 = kv.key;
		ObjectValue *v1 = kv.value;
		const char *v = $(k1->key);
		properties[i] = &k1->key;
		i++;
	}

	for (u64 j = 0; j < impl->property_count; j++) {
		const char *v = $(*properties[j]);
		ObjectKey k;
		k.namespace = impl->namespace;
		k.key = *properties[j];
		ObjectValue *value = rbtree_get_mut(tree, &k);
		if (value->type == ObjectTypeObject) {
			object_cleanup($(value->value));
		} else if (value->type == ObjectTypeString) {
			printf("found a string\n");
		}
		fam_free(&value->value);
		rbtree_delete(tree, &k);
	}

	if (!nil(impl->self)) {
		fam_free(&impl->self);
		impl->self = null;
	}
}

void object_cleanup(ObjectNc *ptr) {
	if (ptr == NULL)
		return;
	printf("obj cleanup\n");
	if (!nil(ptr->flags)) {
		fam_free(&ptr->flags);
		ptr->flags = null;
	}

	if (nil(ptr->impl))
		return;
	ObjectImpl *impl = $(ptr->impl);
	if (!nil(ptr->impl)) {
		rc_cleanup(&ptr->impl);
		ptr->impl.impl = null;
	}
}

int object_move(Object *dst, const Object *src) {
	dst->flags = src->flags;
	dst->impl = src->impl;
// note: we want to allow immutable objects to be moved. We therefore need to update their
// flags which require this override.
#if defined(__clang__)
// Clang-specific pragma
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#elif defined(__GNUC__) && !defined(__clang__)
// GCC-specific pragma
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#else
#warning "Unknown compiler or platform. No specific warning pragmas applied."
#endif
	ObjectNc *src_mut = src;
#pragma GCC diagnostic pop
	src_mut->impl = rc_null;
	src_mut->flags = null;
	return 0;
}

int object_ref_impl(Object *dst, Object *src, bool imut) {
	FatPtr flags;
	bool send;
	bool src_imut;
	ObjectFlags *src_flags = $(src->flags);
	BitFlags bfsrc = {.flags = &src_flags->flags, .capacity = 1};
	send = bitflags_check(&bfsrc, OBJECT_FLAG_SEND);
	src_imut = bitflags_check(&bfsrc, OBJECT_FLAG_IMUT);
	if (src_imut && !imut) {
		fam_err = Permission;
		return -1;
	}
	{
		SendStateGuard _ = SetSend(send);
		if (fam_alloc(&flags, sizeof(ObjectFlags))) {
			return -1;
		}
	}
	ObjectFlags *ff = $(flags);
	ff->flags = 0;
	BitFlags bfdst = {.flags = &ff->flags, .capacity = 1};
	bitflags_set(&bfdst, OBJECT_FLAG_SEND, send);
	bitflags_set(&bfdst, OBJECT_FLAG_IMUT, imut);

	dst->flags = flags;
	int ret = rc_clone(&dst->impl, &src->impl);
	if (ret) {
		fam_free(&flags);
	}
	return ret;
}
int object_ref(Object *dst, Object *src) {
	return object_ref_impl(dst, src, true);
}
int object_mut_ref(Object *dst, Object *src) {
	return object_ref_impl(dst, src, false);
}

RBTreeNc *object_tree_for(const Object *obj) {
	ObjectFlags *flags = $(obj->flags);
	BitFlags bf = {.flags = &flags->flags, .capacity = 1};
	bool send = bitflags_check(&bf, OBJECT_FLAG_SEND);

	RBTreeNc *tree;
	if (send)
		tree = global_rbtree;
	else
		tree = thread_local_rbtree;
	return tree;
}

int object_set_property_string(Object *obj, const char *key, const char *value) {
	ObjectImpl *impl = $(obj->impl);
	RBTreeNc *tree = object_tree_for(obj);

	ObjectKeyNc objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, key);

	ObjectValue objvalue;
	objvalue.type = ObjectTypeString;
	fam_alloc(&objvalue.value, sizeof(char) * (1 + strlen(value)));
	strcpy($(objvalue.value), value);
	impl->property_count++;

	return rbtree_insert(tree, &objkey, &objvalue);
}

int object_create(Object *obj, bool send, ObjectType type, const void *primitive) {
	if (object_init_rbtrees())
		return -1;
	if (obj == NULL) {
		fam_err = IllegalArgument;
		return -1;
	}

	FatPtr ptr;
	FatPtr flags;
	{
		SendStateGuard _ = SetSend(send);
		if (fam_alloc(&ptr, sizeof(ObjectImpl))) {
			return -1;
		}
		if (fam_alloc(&flags, sizeof(ObjectFlags))) {
			fam_free(&ptr);
			return -1;
		}
	}
	ObjectImpl *impl = $(ptr);
	ObjectFlags *obj_flags = $(flags);
	impl->self = ptr;
	if (send)
		impl->namespace = object_namespace_id_next_global();
	else
		impl->namespace = object_namespace_id_next_local();
	obj->flags = flags;

	obj_flags->flags = 0;
	impl->type = type;
	impl->property_count = 0;
	impl->send = send;
	BitFlags bf = {.flags = &obj_flags->flags, .capacity = 1};
	bitflags_set(&bf, OBJECT_FLAG_SEND, send);

	if (rc_build(&obj->impl, impl, sizeof(ObjectImpl), false, (void (*)(void *))object_cleanup_rc))
		return -1;

	if (type == ObjectTypeString) {
		object_set_property_string(obj, "value", primitive);
	}

	return 0;
}

const char *object_as_string(const Object *obj) {
	ObjectImpl *impl = $(obj->impl);
	RBTreeNc *tree = object_tree_for(obj);

	ObjectKey objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, "value");

	const ObjectValue *valueret = rbtree_get(tree, &objkey);

	if (valueret == NULL || valueret->type != ObjectTypeString)
		return NULL;

	return $(valueret->value);
}

int object_set_property(Object *obj, const char *key, const Object *value) {
	ObjectImpl *impl = $(obj->impl);
	ObjectKeyNc objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, key);

	ObjectValue objvalue;
	objvalue.type = ObjectTypeObject;

	fam_alloc(&objvalue.value, sizeof(Object));
	object_move($(objvalue.value), value);

	RBTreeNc *tree = object_tree_for(obj);
	impl->property_count++;
	printf("set property\n");
	return rbtree_insert(tree, &objkey, &objvalue);
}

const Object *object_get_property(const Object *obj, const char *key) {
	return NULL;
}

int object_send(Object *obj, Channel *channel) {
	return 0;
}

#ifdef TEST
void object_cleanup_rbtree(RBTree *ptr) {
	RBTreeIterator itt;
	rbtree_iterator(ptr, &itt, NULL, false, NULL, false);
	loop {
		RbTreeKeyValue kv;
		if (!rbtree_iterator_next(&itt, &kv))
			break;
		ObjectKeyNc *key = kv.key;
		fam_free(&key->key);

		ObjectValue *value = kv.value;
		if (value->type == ObjectTypeString) {
			fam_free(&value->value);
		}
	}
	rbtree_cleanup(ptr);
	myfree(ptr);
}

void object_cleanup_global() {
	if (thread_local_rbtree)
		printf("size=%llu\n", rbtree_size(thread_local_rbtree));
	if (global_rbtree)
		printf("size=%llu\n", rbtree_size(global_rbtree));
	if (thread_local_rbtree)
		object_cleanup_rbtree(thread_local_rbtree);
	if (global_rbtree)
		object_cleanup_rbtree(global_rbtree);

	thread_local_rbtree = NULL;
	global_rbtree = NULL;
}
#endif // TEST
