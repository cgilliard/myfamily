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

#include <assert.h>
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
	u64 seqno;
	u64 namespace;
	FatPtr name;
	FatPtr value;
	u32 seqno_precision;
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

int object_sequence_compare(const void *v1, const void *v2) {
	const ObjectValueNc *k1 = v1;
	const ObjectValueNc *k2 = v2;
	if (k1->namespace != k2->namespace) {
		if (k1->namespace < k2->namespace) {
			return -1;
		} else {
			return 1;
		}
	} else {
		if (k1->seqno < k2->seqno)
			return -1;
		else if (k1->seqno > k2->seqno)
			return 1;
		if (k1->seqno_precision < k2->seqno_precision)
			return -1;
		else if (k1->seqno_precision > k2->seqno_precision)
			return 1;
		return 0;
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
		if (orbtree_create(sequence_tree, sizeof(ObjectValue), object_sequence_compare)) {
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

typedef struct ObjectImpl {
	FatPtr self;
	u64 namespace;
	u64 seqno;
	ObjectType type;
	u32 property_count;
	u64 property_seqno_next;
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

		ObjectNc *to_cleanup[impl->property_count];
		u64 cleanup_count = 0;
		tl_start_range_value.namespace = impl->namespace;
		tl_end_range_value.namespace = impl->namespace + 1;
		tl_start_range_value.seqno = 0;
		tl_start_range_value.seqno_precision = 0;
		tl_end_range_value.seqno = 0;
		tl_end_range_value.seqno_precision = 0;

		bool send = impl->send;
		ORBTreeTray tray;
		FatPtr *properties[impl->property_count];
		ORBTreeIteratorNc *itt;
		ORBTreeNc *tree;
		ORBTreeIteratorNc *sequence_itt;
		ORBTreeNc *sequence_tree;
		if (send) {
			if (pthread_rwlock_wrlock(&global_rbtree_lock))
				panic("rwlock error!");
			itt = global_orb_context->sorted_itt;
			tree = global_orb_context->sorted_tree;
			sequence_tree = global_orb_context->sequence_tree;
			sequence_itt = global_orb_context->sequence_itt;
		} else {
			itt = tl_orb_context->sorted_itt;
			tree = tl_orb_context->sorted_tree;
			sequence_tree = tl_orb_context->sequence_tree;
			sequence_itt = tl_orb_context->sequence_itt;
		}

		orbtree_iterator_reset(itt, &tl_start_range_value, true, &tl_end_range_value, false);

		u64 i = 0;
		while (orbtree_iterator_next(itt, &tray)) {
			ObjectValueNc *v = tray.value;
			properties[i] = &v->name;
			i++;
		}
		assert(i == impl->property_count);

		for (i = 0; i < impl->property_count; i++) {
			ObjectValueNc value;
			value.namespace = impl->namespace;
			value.name = *properties[i];
			orbtree_remove(tree, &value, &tray);
			ObjectValueNc *rem = tray.value;
			u64 seqno = rem->seqno;
			u32 precision = rem->seqno_precision;
			ObjectValueData *obj_data = $(rem->value);
			if (obj_data->type == ObjectTypeObject) {
				to_cleanup[cleanup_count++] = (ObjectNc *)obj_data->value;
			}

			fam_free(&rem->value);
			fam_free(&rem->name);
			orbtree_deallocate_tray(tree, &tray);

			value.seqno = seqno;
			value.seqno_precision = precision;
			int res = orbtree_remove(sequence_tree, &value, &tray);
			orbtree_deallocate_tray(sequence_tree, &tray);
		}

		if (send) {
			if (pthread_rwlock_unlock(&global_rbtree_lock))
				panic("rwlock error!");
		}

		for (i = 0; i < cleanup_count; i++) {
			object_cleanup(to_cleanup[i]);
		}
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

int object_value_for(ObjectValue *value, const Object *obj, const char *name, i64 seqno) {
	const ObjectImpl *impl = $(obj->impl);
	if (name) {
		bool send = object_is_send(obj);
		{
			SendStateGuard _ = SetSend(send);
			if (fam_alloc(&value->name, sizeof(char) * (1 + strlen(name))))
				return -1;
		}

		strcpy($(value->name), name);
	}
	value->namespace = impl->namespace;
	if (seqno < 0) {
		value->seqno = impl->property_seqno_next;
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
		ObjectImpl *impl_mut = impl;
#pragma GCC diagnostic pop
		impl_mut->property_seqno_next++;
	} else
		value->seqno = seqno;
	value->seqno_precision = 0;

	return 0;
}

int object_put_trees(ObjectImpl *impl, bool send, ObjectValue *val) {
	int ret = 0;
	ORBTreeNc *tree;
	ORBTreeNc *sequence_tree;
	if (send) {
		if (pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
		tree = global_orb_context->sorted_tree;
		sequence_tree = global_orb_context->sequence_tree;
	} else {
		tree = tl_orb_context->sorted_tree;
		sequence_tree = tl_orb_context->sequence_tree;
	}
	ORBTreeTray tray, retval;
	ORBTreeTray sequence_tray, sequence_retval;
	ObjectValueNc obv, sequence_obv;
	retval.value = &obv;
	sequence_retval.value = &sequence_obv;
	if (orbtree_allocate_tray(tree, &tray) || orbtree_allocate_tray(sequence_tree, &sequence_tray))
		ret = -1;
	else {
		memcpy(tray.value, val, sizeof(ObjectValue));
		retval.updated = false;
		ret = orbtree_put(tree, &tray, &retval);
		if (retval.updated) {
			ObjectValueData *obj_data = $(obv.value);
			u64 seqno = obv.seqno;
			u32 precision = obv.seqno_precision;
			if (obj_data->type == ObjectTypeObject) {
				object_cleanup((ObjectNc *)obj_data->value);
			}
			object_value_cleanup(&obv);
			orbtree_deallocate_tray(tree, &tray);
			orbtree_deallocate_tray(sequence_tree, &sequence_tray);

			// reset seqno to previous value
			orbtree_get(tree, tray.value, &retval);
			ObjectValueNc *v = retval.value;
			v->seqno = seqno;
			v->seqno_precision = precision;
		} else {
			memcpy(sequence_tray.value, val, sizeof(ObjectValue));
			orbtree_put(sequence_tree, &sequence_tray, &sequence_retval);
		}
	}
	if (send) {
		if (pthread_rwlock_unlock(&global_rbtree_lock))
			panic("rwlock error!");
	}

	if (ret == 0 && !retval.updated)
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
	if (object_value_for(&val, obj, name, -1))
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
	impl->property_seqno_next = 0;

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

const ObjectValueData *object_get_property_data(const Object *obj, const char *name, bool remove,
												ORBTreeTray *tray) {
	bool send = object_is_send(obj);
	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return NULL;
	}

	ORBTreeNc *tree;
	ORBTreeNc *sequence_tree;
	if (send) {
		if (pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
		tree = global_orb_context->sorted_tree;
		sequence_tree = global_orb_context->sequence_tree;
	} else {
		tree = tl_orb_context->sorted_tree;
		sequence_tree = tl_orb_context->sequence_tree;
	}

	ObjectValue objvalue = INIT_OBJECT_VALUE;
	object_value_for(&objvalue, obj, name, -1);
	tray->updated = false;
	int v;
	if (remove) {
		v = orbtree_remove(tree, &objvalue, tray);
		if (v == 0) {
			ObjectValueNc *rem = tray->value;
			u64 seqno = rem->seqno;
			u32 precision = rem->seqno_precision;
			ObjectValueData *obj_data = $(rem->value);
			fam_free(&rem->name);
			orbtree_deallocate_tray(tree, tray);
			objvalue.seqno = seqno;
			objvalue.seqno_precision = precision;
			orbtree_remove(sequence_tree, &objvalue, tray);
			orbtree_deallocate_tray(sequence_tree, tray);
		}
	} else {
		v = orbtree_get(tree, &objvalue, tray);
	}
	if (send) {
		pthread_rwlock_unlock(&global_rbtree_lock);
	}

	if (!tray->updated) {
		return NULL;
	}
	const ObjectValueNc *valueret = tray->value;
	const ObjectValueData *valueretdata = $(valueret->value);

	return valueretdata;
}

i64 object_properties(const Object *obj) {
	if (obj == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return -1;
	}
	return impl->property_count;
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

	ORBTreeTray tray;
	const ObjectValueData *vd = object_get_property_data(obj, name, false, &tray);
	if (vd == NULL)
		return NIL;
	ObjectNc ret = *(Object *)vd->value;

	return object_ref(&ret);
}

Object object_remove_property(Object *obj, const char *name) {
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

	ORBTreeTray tray;
	const ObjectValueData *vd = object_get_property_data(obj, name, true, &tray);
	if (vd == NULL)
		return NIL;

	impl->property_count--;
	ObjectNc *ret = (Object *)vd->value;

	ObjectNc ref_ret = object_move(ret);
	ObjectValueNc *rem = tray.value;
	fam_free(&rem->value);

	return ref_ret;
}

Object object_get_property_index(const Object *obj, u32 index) {
	object_check_consumed(obj);

	bool send = object_is_send(obj);
	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return NIL;
	}

	ORBTreeNc *tree;
	if (send) {
		tree = global_orb_context->sequence_tree;
		if (pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
	} else
		tree = tl_orb_context->sequence_tree;

	ObjectValueNc objvalue = INIT_OBJECT_VALUE;
	object_value_for(&objvalue, obj, NULL, index);
	ORBTreeTray tray;
	tray.updated = false;
	int v = orbtree_get_index(tree, index, &tray);
	if (send) {
		pthread_rwlock_unlock(&global_rbtree_lock);
	}

	if (!tray.updated) {
		return NIL;
	}
	const ObjectValueNc *valueret = tray.value;
	const ObjectValueData *vd = $(valueret->value);

	ObjectNc ret = *(Object *)vd->value;
	if (nil(ret))
		return NIL;

	return object_ref(&ret);
}

Object object_remove_property_index(Object *obj, u32 index) {
	object_check_consumed(obj);
	bool send = object_is_send(obj);
	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return NIL;
	}

	ORBTreeNc *tree;
	if (send) {
		tree = global_orb_context->sequence_tree;
		if (pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
	} else
		tree = tl_orb_context->sequence_tree;

	ObjectValueNc objvalue = INIT_OBJECT_VALUE;
	object_value_for(&objvalue, obj, NULL, index);
	ORBTreeTray tray;
	tray.updated = false;
	int v = orbtree_get_index(tree, index, &tray);
	if (send) {
		pthread_rwlock_unlock(&global_rbtree_lock);
	}

	if (!tray.updated) {
		return NIL;
	}
	const ObjectValueNc *valueret = tray.value;
	const ObjectValueData *vd = $(valueret->value);

	ObjectNc ret = *(Object *)vd->value;

	const char *name = $(valueret->name);

	let res = object_remove_property(obj, name);
	// an unexpected error occured removing the property
	if (nil(res)) {
		return NIL;
	}

	return object_ref(&ret);
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
	ORBTreeTray tray;
	const ObjectValueData *v = object_get_property_data(obj, "value", false, &tray);
	if (v == NULL || v->type != ObjectTypeString) {
		SetErr(ExpectedTypeMismatch);
		return NULL;
	}

	return (char *)v->value;
}
u64 object_as_u64(const Object *obj) {
	object_check_consumed(obj);
	ORBTreeTray tray;
	const ObjectValueData *v = object_get_property_data(obj, "value", false, &tray);
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
i64 get_thread_local_orbtree_alloc_count() {
	if (tl_orb_context == NULL)
		return 0;
	return orbtree_slabs(tl_orb_context->sorted_tree);
}
i64 get_global_orbtree_alloc_count() {
	if (global_orb_context == NULL)
		return 0;
	return orbtree_slabs(global_orb_context->sorted_tree);
}
u64 get_thread_local_orbtree_size() {
	if (tl_orb_context == NULL)
		return 0;
	return orbtree_size(tl_orb_context->sorted_tree);
}
u64 get_global_orbtree_size() {
	if (global_orb_context == NULL)
		return 0;
	return orbtree_size(global_orb_context->sorted_tree);
}

i64 get_thread_local_orbtree_seq_alloc_count() {
	if (tl_orb_context == NULL)
		return 0;
	return orbtree_slabs(tl_orb_context->sequence_tree);
}
i64 get_global_orbtree_seq_alloc_count() {
	if (global_orb_context == NULL)
		return 0;
	return orbtree_slabs(global_orb_context->sequence_tree);
}
u64 get_thread_local_orbtree_seq_size() {
	if (tl_orb_context == NULL)
		return 0;
	return orbtree_size(tl_orb_context->sequence_tree);
}
u64 get_global_orbtree_seq_size() {
	if (global_orb_context == NULL)
		return 0;
	return orbtree_size(global_orb_context->sequence_tree);
}
void object_cleanup_global() {
	if (global_orb_context != NULL) {
		orbtree_iterator_cleanup(global_orb_context->sorted_itt);
		orbtree_iterator_cleanup(global_orb_context->sequence_itt);
		orbtree_cleanup(global_orb_context->sorted_tree);
		orbtree_cleanup(global_orb_context->sequence_tree);
		myfree(global_orb_context->sequence_itt);
		myfree(global_orb_context->sequence_tree);
		myfree(global_orb_context->sorted_itt);
		myfree(global_orb_context->sorted_tree);
		myfree(global_orb_context);
	}
}
#endif // TEST
