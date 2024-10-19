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

#include <base/fam_alloc.h>
#include <base/panic.h>
#include <base/resources.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <util/object.h>
#include <util/orbtree.h>

typedef struct ORBContext {
	ORBTree *sequence_tree;
	ORBTreeIterator *sequence_itt;
	ORBTree *sorted_tree;
	ORBTreeIterator *sorted_itt;
	u64 namespace_next;
	atomic_ullong global_namespace_next;
} ORBContext;

pthread_rwlock_t global_rbtree_lock = PTHREAD_RWLOCK_INITIALIZER;
_Thread_local ORBContext *tl_orb_context = NULL;
ORBContext *global_orb_context = NULL;

typedef struct ObjectValueNc {
	u64 namespace;
	u64 seqno;
	FatPtr name;
	FatPtr value;
} ObjectValueNc;

void object_value_cleanup(ObjectValueNc *ptr) {
	if (!nil(ptr->name))
		fam_free(&ptr->name);
	if (!nil(ptr->value))
		fam_free(&ptr->value);
}

#define INIT_OBJECT_VALUE {.name = null, .value = null}
#define ObjectValue ObjectValueNc __attribute__((warn_unused_result, cleanup(object_value_cleanup)))
_Thread_local ObjectValueNc tl_start_range_value = INIT_OBJECT_VALUE;
_Thread_local ObjectValueNc tl_end_range_value = INIT_OBJECT_VALUE;

int init_tl_range_values() {
	if (nil(tl_start_range_value.name)) {
		if (fam_alloc(&tl_start_range_value.name, 1))
			return -1;
		strcpy($(tl_start_range_value.name), "");
	}
	if (nil(tl_end_range_value.name)) {
		if (fam_alloc(&tl_end_range_value.name, 1))
			return -1;
		strcpy($(tl_end_range_value.name), "");
	}
	return 0;
}

int object_property_name_compare(const void *v1, const void *v2) {
	const ObjectValueNc *k1 = v1;
	const ObjectValueNc *k2 = v2;
	if (k1->namespace != k2->namespace) {
		if (k1->namespace < k2->namespace)
			return -1;
		else
			return 1;
	} else {
		int ret = strcmp($(k1->name), $(k2->name));
		return ret;
	}
}

ORBContext *init_orb_context(bool send) {
	ORBContext *ret = mymalloc(sizeof(ORBContext));
	ORBTreeNc *sequence_tree = mymalloc(sizeof(ORBTree));
	ORBTreeIteratorNc *sequence_itt = mymalloc(sizeof(ORBTreeIterator));
	ORBTreeNc *sorted_tree = mymalloc(sizeof(ORBTree));
	ORBTreeIteratorNc *sorted_itt = mymalloc(sizeof(ORBTreeIterator));

	if (ret && sequence_tree && sequence_itt && sorted_tree && sorted_itt) {
		ret->sequence_tree = sequence_tree;
		ret->sequence_itt = sequence_itt;
		ret->sorted_tree = sorted_tree;
		ret->sorted_itt = sorted_itt;
		if (orbtree_create(sequence_tree, sizeof(ObjectValue), object_property_name_compare)) {
			if (sequence_tree)
				myfree(sequence_tree);
			if (ret)
				myfree(ret);
			if (sequence_itt)
				myfree(sequence_itt);
			if (sorted_tree)
				myfree(sorted_tree);
			if (sorted_itt)
				myfree(sorted_itt);
			return NULL;
		}

		if (orbtree_iterator(sequence_tree, sequence_itt, NULL, false, NULL, false, send)) {
			if (sequence_tree)
				myfree(sequence_tree);
			if (ret)
				myfree(ret);
			if (sequence_itt)
				myfree(sequence_itt);
			if (sorted_tree)
				myfree(sorted_tree);
			if (sorted_itt)
				myfree(sorted_itt);
			return NULL;
		}

		if (orbtree_create(sorted_tree, sizeof(ObjectValue), object_property_name_compare)) {
			if (sequence_tree)
				myfree(sequence_tree);
			if (ret)
				myfree(ret);
			if (sequence_itt)
				myfree(sequence_itt);
			if (sorted_tree)
				myfree(sorted_tree);
			if (sorted_itt)
				myfree(sorted_itt);
			return NULL;
		}

		if (orbtree_iterator(sorted_tree, sorted_itt, NULL, false, NULL, false, send)) {
			if (sequence_tree)
				myfree(sequence_tree);
			if (ret)
				myfree(ret);
			if (sequence_itt)
				myfree(sequence_itt);
			if (sorted_tree)
				myfree(sorted_tree);
			if (sorted_itt)
				myfree(sorted_itt);
			return NULL;
		}
	} else {
		if (sequence_tree)
			myfree(sequence_tree);
		if (ret)
			myfree(ret);
		if (sequence_itt)
			myfree(sequence_itt);
		if (sorted_tree)
			myfree(sorted_tree);
		if (sorted_itt)
			myfree(sorted_itt);
	}
	ret->namespace_next = 1;
	ret->global_namespace_next = 1;
	return ret;
}

int object_init_global() {
	if (init_tl_range_values())
		return -1;
	if (global_orb_context == NULL) {
		if (pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
		if (global_orb_context == NULL) {
			global_orb_context = init_orb_context(true);
		}

		if (pthread_rwlock_unlock(&global_rbtree_lock))
			panic("rwlock error!");
		if (global_orb_context == NULL)
			return -1;
	}
	return 0;
}

int object_init_local() {
	if (init_tl_range_values())
		return -1;
	if (tl_orb_context == NULL) {
		tl_orb_context = init_orb_context(false);
		if (tl_orb_context == NULL)
			return -1;
	}
	return 0;
}

typedef struct ObjectValueData {
	ObjectType type;
	u32 pad;
	u8 value[];
} ObjectValueData;

typedef struct ObjectFlags {
	FatPtr self;
} ObjectFlags;

typedef struct ObjectImpl {
	FatPtr self;
	u64 namespace;
	u64 seqno;
	ObjectType type;
	u32 property_count;
	bool send;
} ObjectImpl;

void object_cleanup(const Object *ptr) {
	if (ptr == NULL)
		return;
	if (ptr->flags & OBJECT_FLAG_NO_CLEANUP) {
		return;
	}
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
	if (nil(ptr_mut->impl)) {
		return;
	}

	ObjectImpl *impl = $(ptr_mut->impl);
	if (!nil(ptr_mut->impl)) {
		rc_cleanup(&ptr_mut->impl);
		ptr_mut->impl.impl = null;
	}
}

void object_cleanup_rc(ObjectImpl *impl) {
	if (!nil(impl->self)) {
		fam_free(&impl->self);
		impl->self = null;
	}

	if (impl->property_count) {
		tl_start_range_value.namespace = impl->namespace;
		tl_end_range_value.namespace = impl->namespace + 1;
		bool send = impl->send;
		ORBTreeIteratorNc *itt;
		ORBTreeNc *tree;
		if (send) {
			if (pthread_rwlock_wrlock(&global_rbtree_lock))
				panic("rwlock error!");
			itt = global_orb_context->sorted_itt;
			tree = global_orb_context->sorted_tree;
		} else {
			itt = tl_orb_context->sorted_itt;
			tree = tl_orb_context->sorted_tree;
		}

		orbtree_iterator_reset(itt, &tl_start_range_value, true, &tl_end_range_value, false);
		ORBTreeTray tray;
		FatPtr *properties[impl->property_count];
		u64 i = 0;
		while (orbtree_iterator_next(itt, &tray)) {
			ObjectValueNc *v = tray.value;
			properties[i] = &v->name;
			i++;
		}

		for (i = 0; i < impl->property_count; i++) {
			ObjectValueNc value;
			value.namespace = impl->namespace;
			value.name = *properties[i];
			orbtree_remove(tree, &value, &tray);
			ObjectValueNc *rem = tray.value;
			ObjectValueData *obj_data = $(rem->value);
			if (obj_data->type == ObjectTypeObject) {
				object_cleanup((ObjectNc *)obj_data->value);
			}
			fam_free(&rem->value);
			fam_free(&rem->name);
			orbtree_deallocate_tray(tree, &tray);
		}

		if (send)
			if (pthread_rwlock_unlock(&global_rbtree_lock))
				panic("rwlock error!");
	}
}

void object_check_consumed(const Object *ptr) {
	if (ptr == NULL)
		panic("Object pointer NULL!");
	u8 flags = ptr->flags;
	bool consumed = flags & OBJECT_FLAG_CONSUMED;
	if (consumed)
		panic("Object has been consumed!");
	if (nil(ptr->impl))
		panic("Object is nil!");
}

// move the object to a new memory location
Object object_move(const Object *src) {
	object_check_consumed(src);
	ObjectNc dst = NIL;
	dst.flags = src->flags;
	dst.impl = src->impl;
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
	src_mut->flags |= OBJECT_FLAG_NO_CLEANUP | OBJECT_FLAG_CONSUMED;

	return dst;
}
// create a reference counted reference of the object
Object object_ref(const Object *src) {
	object_check_consumed(src);
	ObjectNc dst = NIL;
	dst.flags = src->flags;
	rc_clone(&dst.impl, &src->impl);
	return dst;
}

bool object_is_send(const Object *obj) {
	return (obj->flags & OBJECT_FLAGS_SEND) != 0;
}

int object_value_for(ObjectValue *value, const Object *obj, const char *name) {
	const ObjectImpl *impl = $(obj->impl);
	bool send = object_is_send(obj);
	{
		SendStateGuard _ = SetSend(send);
		if (fam_alloc(&value->name, sizeof(char) * (1 + strlen(name))))
			return -1;
	}

	strcpy($(value->name), name);
	value->namespace = impl->namespace;

	return 0;
}

int object_put_trees(ObjectImpl *impl, bool send, ObjectValue *val) {
	int ret = 0;
	ORBTreeNc *tree;
	if (send) {
		tree = global_orb_context->sorted_tree;
		if (pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
	} else
		tree = tl_orb_context->sorted_tree;
	ORBTreeTray tray, retval;
	if (orbtree_allocate_tray(tree, &tray))
		ret = -1;
	else {
		memcpy(tray.value, val, sizeof(ObjectValue));
		ret = orbtree_put(tree, &tray, &retval);
	}
	if (send)
		if (pthread_rwlock_unlock(&global_rbtree_lock))
			panic("rwlock error!");

	if (ret == 0)
		impl->property_count++;
	return ret;
}

int object_set_property_value(Object *obj, const char *name, const void *value, u64 size,
							  ObjectType type) {
	if (obj == NULL || name == NULL || value == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	bool send = object_is_send(obj);
	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}

	ObjectValueNc val;
	if (object_value_for(&val, obj, name))
		return -1;

	if (fam_alloc(&val.value, sizeof(ObjectValueData) + size))
		return -1;

	ObjectValueData *objdata = $(val.value);
	objdata->type = type;

	memcpy(objdata->value, value, size);

	return object_put_trees(impl, send, &val);
}

Object object_create(bool send, ObjectType type, const void *primitive) {
	ObjectNc obj = NIL;
	if (send) {
		if (object_init_global()) {
			return NIL;
		}

	} else {
		if (object_init_local()) {
			return NIL;
		}
	}

	FatPtr ptr = null;
	{
		SendStateGuard _ = SetSend(send);
		if (fam_alloc(&ptr, sizeof(ObjectImpl))) {
			return NIL;
		}
	}

	ObjectImpl *impl = $(ptr);
	impl->send = send;

	impl->self = ptr;

	ORBContext *ctx;

	if (send) {
		ctx = global_orb_context;
		impl->namespace = atomic_fetch_add(&ctx->global_namespace_next, 1);
	} else {
		ctx = tl_orb_context;
		impl->namespace = ctx->namespace_next;
		ctx->namespace_next++;
	}
	obj.flags = 0;
	if (send)
		obj.flags |= OBJECT_FLAGS_SEND;

	impl->type = type;
	impl->property_count = 0;

	if (rc_build(&obj.impl, impl, sizeof(ObjectImpl), false, (void (*)(void *))object_cleanup_rc)) {
		fam_free(&ptr);
		return NIL;
	}

	u64 size = 0;
	if (type == ObjectTypeString) {
		size = sizeof(char) * (1 + strlen((char *)primitive));
	} else if (type == ObjectTypeU64) {
		size = sizeof(u64);
	}

	if (size) {
		if (object_set_property_value(&obj, "value", primitive, size, type)) {
			object_cleanup(&obj);
			return NIL;
		}
	}

	return obj;
}

Object object_set_property(Object *obj, const char *name, const Object *value) {
	object_check_consumed(obj);
	object_check_consumed(value);
	ObjectNc vmove = object_move(value);
	if (object_set_property_value(obj, name, &vmove, sizeof(Object), ObjectTypeObject))
		return NIL;
	return UNIT;
}

const ObjectValueData *object_get_property_data(const Object *obj, const char *name) {
	bool send = object_is_send(obj);
	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return NULL;
	}

	ORBTreeNc *tree;
	if (send) {
		tree = global_orb_context->sorted_tree;
		if (pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
	} else
		tree = tl_orb_context->sorted_tree;

	ObjectValue objvalue = INIT_OBJECT_VALUE;
	object_value_for(&objvalue, obj, name);
	if (send)
		pthread_rwlock_rdlock(&global_rbtree_lock);
	ORBTreeTray tray;
	tray.updated = false;
	int v = orbtree_get(tree, &objvalue, &tray);
	if (send)
		pthread_rwlock_unlock(&global_rbtree_lock);

	if (!tray.updated) {
		return NULL;
	}
	const ObjectValueNc *valueret = tray.value;
	const ObjectValueData *valueretdata = $(valueret->value);

	return valueretdata;
}

Object object_get_property(const Object *obj, const char *name) {
	if (obj == NULL || name == NULL) {
		SetErr(IllegalArgument);
		return NIL;
	}
	object_check_consumed(obj);

	bool send = object_is_send(obj);
	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return NIL;
	}

	const ObjectValueData *vd = object_get_property_data(obj, name);
	if (vd == NULL)
		return NIL;
	ObjectNc ret = *(Object *)vd->value;

	return object_ref(&ret);
}

Object object_remove_property(Object *obj, const char *name) {
	object_check_consumed(obj);
	return NIL;
}

Object object_get_property_index(const Object *obj, u32 index) {
	object_check_consumed(obj);
	return NIL;
}

Object object_remove_property_index(Object *obj, u32 index) {
	object_check_consumed(obj);
	return NIL;
}

Object object_set_property_index(Object *obj, const Object *value, u64 index) {
	object_check_consumed(obj);
	return NIL;
}

Object object_insert_property_before_index(Object *obj, const char *name, const Object *value,
										   u64 index) {
	object_check_consumed(obj);
	return NIL;
}
Object object_insert_property_after_index(Object *obj, const char *name, const Object *value,
										  u64 index) {
	object_check_consumed(obj);
	return NIL;
}

ObjectType object_type(const Object *obj) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	return impl->type;
}
int object_send(Object *obj, Channel *channel) {
	return 0;
}

const char *object_as_string(const Object *obj) {
	object_check_consumed(obj);
	const ObjectValueData *v = object_get_property_data(obj, "value");
	if (v == NULL || v->type != ObjectTypeString) {
		SetErr(ExpectedTypeMismatch);
		return NULL;
	}

	return (char *)v->value;
}
u64 object_as_u64(const Object *obj) {
	object_check_consumed(obj);
	const ObjectValueData *v = object_get_property_data(obj, "value");
	if (v == NULL || v->type != ObjectTypeU64) {
		SetErr(ExpectedTypeMismatch);
		return 0;
	}
	return *(u64 *)v->value;
}

void object_cleanup_thread_local() {
	object_value_cleanup(&tl_start_range_value);
	object_value_cleanup(&tl_end_range_value);
	if (tl_orb_context != NULL) {
		orbtree_iterator_cleanup(tl_orb_context->sorted_itt);
		orbtree_iterator_cleanup(tl_orb_context->sequence_itt);
		orbtree_cleanup(tl_orb_context->sorted_tree);
		orbtree_cleanup(tl_orb_context->sequence_tree);
		myfree(tl_orb_context->sequence_itt);
		myfree(tl_orb_context->sequence_tree);
		myfree(tl_orb_context->sorted_itt);
		myfree(tl_orb_context->sorted_tree);
		myfree(tl_orb_context);
	}
}

#ifdef TEST
u64 get_thread_local_rbtree_size() {
	if (tl_orb_context == NULL)
		return 0;
	return orbtree_size(tl_orb_context->sorted_tree);
}
u64 get_global_rbtree_size() {
	if (global_orb_context == NULL)
		return 0;
	return orbtree_size(global_orb_context->sorted_tree);
}
void object_cleanup_global() {
}
#endif // TEST
