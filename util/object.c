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
#include <base/resources.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <util/object.h>
#include <util/rbtree.h>

pthread_rwlock_t global_rbtree_lock = PTHREAD_MUTEX_INITIALIZER;
_Thread_local u64 thread_local_namespace_next = 0;
atomic_ullong global_namespace_next = 0;

#define INIT_OBJECT_KEY {.key = null}

typedef struct RBTreeIttPair {
	RBTree *tree;
	RBTreeIterator *itt;
} RBTreeIttPair;

_Thread_local RBTreeIttPair *thread_local_rbtree;
RBTreeIttPair *global_rbtree;

typedef struct ObjectImpl {
	FatPtr self;
	u64 namespace;
	ObjectType type;
	bool send;
	u32 property_count;
} ObjectImpl;

typedef struct ObjectFlags {
	FatPtr self;
	u8 flags;
} ObjectFlags;

typedef struct ObjectKeyNc {
	FatPtr key;
	u64 namespace;
} ObjectKeyNc;

void object_key_cleanup(ObjectKeyNc *ptr) {
	if (!nil(ptr->key))
		fam_free(&ptr->key);
}

#define ObjectKey ObjectKeyNc __attribute__((warn_unused_result, cleanup(object_key_cleanup)))

_Thread_local ObjectKeyNc tl_start_range_key = INIT_OBJECT_KEY;
_Thread_local ObjectKeyNc tl_end_range_key = INIT_OBJECT_KEY;

typedef struct ObjectValue {
	FatPtr ptr;
} ObjectValue;

typedef struct ObjectValueData {
	ObjectType type;
	u32 pad;
	char value[];
} ObjectValueData;

#define OBJECT_FLAG_CONSUMED 1
#define OBJECT_FLAG_NO_CLEANUP 2
#define OBJECT_FLAG_SEND 3
#define OBJECT_FLAG_IMUT 4

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

RBTreeNc *object_tree_for(const Object *obj) {
	ObjectFlags *flags = $(obj->flags);
	BitFlags bf = {.flags = &flags->flags, .capacity = 1};
	bool send = bitflags_check(&bf, OBJECT_FLAG_SEND);

	RBTreeNc *tree;
	if (send)
		tree = global_rbtree->tree;
	else
		tree = thread_local_rbtree->tree;
	return tree;
}

RBTreeIttPair *init_rbtree_pair(bool global) {
	RBTreeIttPair *ret = mymalloc(sizeof(RBTreeIttPair));
	RBTreeNc *tree = mymalloc(sizeof(RBTree));
	RBTreeIteratorNc *itt = mymalloc(sizeof(RBTreeIterator));
	if (tree && ret && itt) {
		ret->tree = tree;
		ret->itt = itt;
		if (rbtree_create(tree, sizeof(ObjectKey), sizeof(ObjectValue),
						  object_property_name_compare, global)) {
			if (tree)
				myfree(tree);
			if (ret)
				myfree(ret);
			if (itt)
				myfree(itt);
			return NULL;
		}

		if (rbtree_iterator(tree, itt, NULL, false, NULL, false)) {
			if (tree)
				myfree(tree);
			if (ret)
				myfree(ret);
			if (itt)
				myfree(itt);
			return NULL;
		}
	} else {
		if (tree)
			myfree(tree);
		if (ret)
			myfree(ret);
		if (itt)
			myfree(itt);
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

int object_init_global() {
	if (nil(tl_start_range_key.key)) {
		fam_alloc(&tl_start_range_key.key, 1);
		strcpy($(tl_start_range_key.key), "");
	}
	if (nil(tl_end_range_key.key)) {
		fam_alloc(&tl_end_range_key.key, 1);
		strcpy($(tl_end_range_key.key), "");
	}

	if (global_rbtree == NULL) {
		pthread_rwlock_wrlock(&global_rbtree_lock);
		if (global_rbtree == NULL) {
			global_rbtree = init_rbtree_pair(true);
		}
		pthread_rwlock_unlock(&global_rbtree_lock);
		if (global_rbtree == NULL)
			return -1;
	}
	return 0;
}

int object_init_local() {
	if (nil(tl_start_range_key.key)) {
		fam_alloc(&tl_start_range_key.key, 1);
		strcpy($(tl_start_range_key.key), "");
	}
	if (nil(tl_end_range_key.key)) {
		fam_alloc(&tl_end_range_key.key, 1);
		strcpy($(tl_end_range_key.key), "");
	}

	if (thread_local_rbtree == NULL) {
		thread_local_rbtree = init_rbtree_pair(false);
		if (thread_local_rbtree == NULL)
			return -1;
	}
	return 0;
}

void object_check_consumed(const Object *ptr) {
	if (ptr == NULL)
		panic("Object pointer NULL!");
	ObjectFlags *flags = $(ptr->flags);
	BitFlags bf = {.flags = &flags->flags, .capacity = 1};
	bool consumed = bitflags_check(&bf, OBJECT_FLAG_CONSUMED);
	if (consumed)
		panic("Object has been consumed!");
	if (nil(ptr->impl))
		panic("Object is nil!");
}

void object_cleanup(const ObjectNc *ptr) {
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
	ObjectNc *ptr_mut = ptr;
#pragma GCC diagnostic pop
	if (ptr_mut == NULL) {
		return;
	}

	if (!nil(ptr_mut->flags)) {
		fam_free(&ptr_mut->flags);
		ptr_mut->flags = null;
	}

	if (nil(ptr_mut->impl)) {
		return;
	}
	ObjectImpl *impl = $(ptr_mut->impl);
	if (!nil(ptr_mut->impl)) {
		rc_cleanup(&ptr_mut->impl);
		ptr_mut->impl.impl = null;
	}
}

int object_move(const Object *dst, const Object *src) {
	object_check_consumed(src);
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
	ObjectNc *dst_mut = dst;
#pragma GCC diagnostic pop

	// dst_mut->flags = src->flags;
	//  src_mut->flags = null;

	ObjectFlags *src_flags = $(src_mut->flags);
	BitFlags bfsrc = {.flags = &src_flags->flags, .capacity = 1};
	bool src_send = bitflags_check(&bfsrc, OBJECT_FLAG_SEND);
	bool src_imut = bitflags_check(&bfsrc, OBJECT_FLAG_IMUT);

	{
		SendStateGuard _ = SetSend(src_send);
		if (fam_alloc(&dst_mut->flags, sizeof(ObjectFlags)))
			return -1;
	}

	ObjectFlags *dst_flags = $(dst_mut->flags);
	BitFlags bfdst = {.flags = &dst_flags->flags, .capacity = 1};

	dst_mut->impl = src->impl;
	src_mut->impl = rc_null;

	dst_flags->flags = 0;
	bitflags_set(&bfdst, OBJECT_FLAG_SEND, src_send);
	bitflags_set(&bfdst, OBJECT_FLAG_IMUT, src_imut);
	bitflags_set(&bfsrc, OBJECT_FLAG_CONSUMED, true);

	return 0;
}

int object_ref(const Object *dst, const Object *src) {
	object_check_consumed(src);
	FatPtr flags;
	bool send;
	bool src_imut;
	ObjectFlags *src_flags = $(src->flags);
	BitFlags bfsrc = {.flags = &src_flags->flags, .capacity = 1};
	send = bitflags_check(&bfsrc, OBJECT_FLAG_SEND);
	src_imut = bitflags_check(&bfsrc, OBJECT_FLAG_IMUT);
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
	bitflags_set(&bfdst, OBJECT_FLAG_IMUT, true);

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
	ObjectNc *unconstdst = dst;
#pragma GCC diagnostic pop
	unconstdst->flags = flags;

	int ret = rc_clone(&unconstdst->impl, &src->impl);

	if (ret) {
		fam_free(&flags);
	}
	return ret;
}

int object_key_for(ObjectKey *objkey, const Object *obj, const char *key) {
	ObjectImpl *impl = $(obj->impl);
	int ret;
	{
		SendStateGuard _ = SetSend(impl->send);
		ret = fam_alloc(&objkey->key, sizeof(char) * (1 + strlen(key)));
	}
	if (ret != 0)
		return -1;

	strcpy($(objkey->key), key);
	objkey->namespace = impl->namespace;

	return ret;
}

void object_cleanup_rc(ObjectImpl *impl) {
	RBTreeIttPair *pair;
	if (impl->send)
		pair = global_rbtree;
	else
		pair = thread_local_rbtree;

	if (impl->property_count) {
		tl_start_range_key.namespace = impl->namespace;
		tl_end_range_key.namespace = impl->namespace + 1;

		if (impl->send)
			pthread_rwlock_wrlock(&global_rbtree_lock);

		rbtree_iterator_reset(pair->tree, pair->itt, &tl_start_range_key, true, &tl_end_range_key,
							  false);

		FatPtr *properties[impl->property_count];
		u64 i = 0;

		loop {
			RbTreeKeyValue kv;
			if (!rbtree_iterator_next(pair->itt, &kv))
				break;
			ObjectKeyNc *k1 = kv.key;
			ObjectValue *v1 = kv.value;
			const char *v = $(k1->key);
			properties[i] = &k1->key;
			i++;
		}
		for (u64 j = 0; j < impl->property_count; j++) {
			ObjectKey k;
			k.namespace = impl->namespace;
			k.key = *properties[j];
			ObjectValue *value = rbtree_get_mut(pair->tree, &k);
			ObjectValueData *obj_data = $(value->ptr);
			if (obj_data->type == ObjectTypeObject) {
				object_cleanup((ObjectNc *)obj_data->value);
			}
			fam_free(&value->ptr);
			rbtree_remove(pair->tree, &k);
		}

		if (impl->send)
			pthread_rwlock_unlock(&global_rbtree_lock);
	}

	if (!nil(impl->self)) {
		fam_free(&impl->self);
		impl->self = null;
	}
}

int object_set_property_string(Object *obj, const char *key, const char *value) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	RBTreeNc *tree = object_tree_for(obj);

	ObjectKeyNc objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, key);

	ObjectValue objvalue;
	{
		SendStateGuard _ = SetSend(impl->send);
		if (fam_alloc(&objvalue.ptr, sizeof(ObjectValueData))) {
			return -1;
		}
	}

	ObjectValueData *objdata = $(objvalue.ptr);
	objdata->type = ObjectTypeString;
	strcpy(objdata->value, value);
	impl->property_count++;

	if (impl->send)
		pthread_rwlock_wrlock(&global_rbtree_lock);
	int ret = rbtree_put(tree, &objkey, &objvalue);
	if (impl->send)
		pthread_rwlock_unlock(&global_rbtree_lock);
	return ret;
}

int object_set_property_u64(Object *obj, const char *key, const u64 *value) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	RBTreeNc *tree = object_tree_for(obj);

	ObjectKeyNc objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, key);

	ObjectValue objvalue;
	{
		SendStateGuard _ = SetSend(impl->send);
		if (fam_alloc(&objvalue.ptr, sizeof(ObjectValueData)))
			return -1;
	}
	ObjectValueData *objdata = $(objvalue.ptr);
	objdata->type = ObjectTypeU64;
	memcpy(objdata->value, value, sizeof(u64));
	impl->property_count++;

	if (impl->send)
		pthread_rwlock_wrlock(&global_rbtree_lock);
	int ret = rbtree_put(tree, &objkey, &objvalue);
	if (impl->send)
		pthread_rwlock_unlock(&global_rbtree_lock);
	return ret;
}

ObjectType object_type(const Object *obj) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	return impl->type;
}

int object_create(Object *obj, bool send, ObjectType type, const void *primitive) {
	if (send) {
		if (object_init_global()) {
			return -1;
		}

	} else {
		if (object_init_local()) {
			return -1;
		}
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

	if (rc_build(&obj->impl, impl, sizeof(ObjectImpl), false,
				 (void (*)(void *))object_cleanup_rc)) {
		fam_free(&ptr);
		fam_free(&flags);
		return -1;
	}

	if (type == ObjectTypeString) {
		if (object_set_property_string(obj, "value", primitive)) {
			object_cleanup(obj);
		}
	} else if (type == ObjectTypeU64) {
		if (object_set_property_u64(obj, "value", primitive)) {
			object_cleanup(obj);
		}
	}

	return 0;
}
int object_send(Object *obj, Channel *channel) {
	object_check_consumed(obj);
	return 0;
}
int object_set_property(Object *obj, const char *key, const Object *value) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	ObjectKeyNc objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, key);

	ObjectValue objvalue;
	if (fam_alloc(&objvalue.ptr, sizeof(ObjectValueData) + sizeof(Object))) {
		object_key_cleanup(&objkey);
		return -1;
	}

	ObjectValueData *ovd = $(objvalue.ptr);
	ovd->type = ObjectTypeObject;
	ObjectNc *obj_ptr = (Object *)ovd->value;
	if (object_move(obj_ptr, value)) {
		object_key_cleanup(&objkey);
		return -1;
	}

	RBTreeNc *tree = object_tree_for(obj);

	ObjectKeyNc objkeyswap;
	ObjectValue objvalueswap;
	RbTreeKeyValue rbtreekv;
	rbtreekv.key = &objkeyswap;
	rbtreekv.value = &objvalueswap;
	rbtreekv.update = false;

	if (impl->send)
		pthread_rwlock_wrlock(&global_rbtree_lock);
	int ret = rbtree_put_swap(tree, &objkey, &objvalue, &rbtreekv);
	if (impl->send)
		pthread_rwlock_unlock(&global_rbtree_lock);

	if (ret)
		object_key_cleanup(&objkey);

	if (rbtreekv.update) {
		ObjectValue *value = rbtreekv.value;
		ObjectValueData *obj_data = $(value->ptr);
		if (obj_data->type == ObjectTypeObject) {
			object_cleanup((ObjectNc *)obj_data->value);
		}

		fam_free(&value->ptr);

		ObjectKeyNc *key = rbtreekv.key;
		object_key_cleanup(key);

	} else {
		impl->property_count++;
	}

	return ret;
}

Object object_get_property(const Object *obj, const char *key) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	ObjectKey objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, key);
	RBTreeNc *tree = object_tree_for(obj);

	if (impl->send)
		pthread_rwlock_rdlock(&global_rbtree_lock);
	const ObjectValue *value = rbtree_get(tree, &objkey);
	if (impl->send)
		pthread_rwlock_unlock(&global_rbtree_lock);
	if (value == NULL) {
		ObjectNc ret = NIL;
		return ret;
	}

	const ObjectValueData *value_data = $(value->ptr);
	const ObjectNc ref;

	object_ref(&ref, (ObjectNc *)value_data->value);

	return ref;
}

const char *object_as_string(const Object *obj) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	RBTreeNc *tree = object_tree_for(obj);

	ObjectKey objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, "value");

	if (impl->send)
		pthread_rwlock_rdlock(&global_rbtree_lock);
	const ObjectValue *valueret = rbtree_get(tree, &objkey);
	if (impl->send)
		pthread_rwlock_unlock(&global_rbtree_lock);

	if (valueret == NULL)
		return NULL;
	const ObjectValueData *valueretdata = $(valueret->ptr);

	if (valueretdata->type != ObjectTypeString)
		return NULL;

	return valueretdata->value;
}

int object_as_u64(const Object *obj, u64 *value) {
	object_check_consumed(obj);
	int ret = 0;

	ObjectImpl *impl = $(obj->impl);
	RBTreeNc *tree = object_tree_for(obj);

	ObjectKey objkey = INIT_OBJECT_KEY;
	object_key_for(&objkey, obj, "value");

	if (impl->send)
		pthread_rwlock_rdlock(&global_rbtree_lock);
	const ObjectValue *valueret = rbtree_get(tree, &objkey);
	if (impl->send)
		pthread_rwlock_unlock(&global_rbtree_lock);

	if (valueret == NULL) {
		ret = -1;
	} else {
		const ObjectValueData *valueretdata = $(valueret->ptr);

		if (valueretdata->type != ObjectTypeU64)
			ret = -1;
		else {
			memcpy(value, valueretdata->value, sizeof(u64));
		}
	}

	return ret;
}

void object_cleanup_thread_local() {
	if (thread_local_rbtree) {
		rbtree_iterator_reset(thread_local_rbtree->tree, thread_local_rbtree->itt, NULL, false,
							  NULL, false);
		loop {
			RbTreeKeyValue kv;
			if (!rbtree_iterator_next(thread_local_rbtree->itt, &kv))
				break;
			ObjectKeyNc *key = kv.key;
			fam_free(&key->key);

			ObjectValue *value = kv.value;
			ObjectValueData *ovd = $(value->ptr);
			if (ovd->type == ObjectTypeString || ovd->type == ObjectTypeU64) {
				fam_free(&value->ptr);
			}
		}

		if (!nil(tl_start_range_key.key))
			fam_free(&tl_start_range_key.key);
		if (!nil(tl_end_range_key.key))
			fam_free(&tl_end_range_key.key);
		if (thread_local_rbtree->itt) {
			rbtree_iterator_cleanup(thread_local_rbtree->itt);
			myfree(thread_local_rbtree->itt);
			thread_local_rbtree->itt = NULL;
		}

		rbtree_cleanup(thread_local_rbtree->tree);
		myfree(thread_local_rbtree->tree);

		myfree(thread_local_rbtree);
		thread_local_rbtree = NULL;
	}
}

#ifdef TEST
u64 get_thread_local_rbtree_size() {
	if (!thread_local_rbtree)
		return 0;
	return rbtree_size(thread_local_rbtree->tree);
}
u64 get_global_rbtree_size() {
	if (!global_rbtree)
		return 0;
	return rbtree_size(global_rbtree->tree);
}
void object_cleanup_global() {
	if (global_rbtree) {
		rbtree_iterator_reset(global_rbtree->tree, global_rbtree->itt, NULL, false, NULL, false);
		loop {
			RbTreeKeyValue kv;
			if (!rbtree_iterator_next(global_rbtree->itt, &kv))
				break;
			ObjectKeyNc *key = kv.key;
			fam_free(&key->key);

			ObjectValue *value = kv.value;
			ObjectValueData *ovd = $(value->ptr);
			if (ovd->type == ObjectTypeString || ovd->type == ObjectTypeU64) {
				fam_free(&value->ptr);
			}
		}

		rbtree_iterator_cleanup(global_rbtree->itt);
		myfree(global_rbtree->itt);
		rbtree_cleanup(global_rbtree->tree);
		myfree(global_rbtree->tree);
		myfree(global_rbtree);
		global_rbtree = NULL;
	}
}
#endif // TEST
