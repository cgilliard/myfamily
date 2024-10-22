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
#include <util/object_macros.h>
#include <util/orbtree.h>

#define MAX_CLEANUP_STACK_SIZE 100

typedef struct ORBContext {
	ORBTreeTray tray;
	ORBTreeTray sequence_tray;
	ORBTree *sequence_tree;
	ORBTree *sorted_tree;
	ORBTreeIterator *itt;
	u64 namespace_next;
	atomic_ullong global_namespace_next;
} ORBContext;

pthread_rwlock_t global_rbtree_lock = PTHREAD_RWLOCK_INITIALIZER;
_Thread_local ORBContext *tl_orb_context = NULL;
_Thread_local u32 has_global_lock = 0;
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
		tl_start_range_value.seqno = 0;
		tl_start_range_value.seqno_precision = 0;
	}
	if (nil(tl_end_range_value.name)) {
		if (fam_alloc(&tl_end_range_value.name, 1))
			return -1;
		strcpy($(tl_end_range_value.name), "");
		tl_end_range_value.seqno = 0;
		tl_end_range_value.seqno_precision = 0;
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
	ORBTreeNc *sorted_tree = mymalloc(sizeof(ORBTree));
	ORBTreeIteratorNc *itt = mymalloc(sizeof(ORBTreeIterator));

	if (ret && sequence_tree && sorted_tree && itt) {
		ret->sequence_tree = sequence_tree;
		ret->sorted_tree = sorted_tree;
		ret->itt = itt;
		if (orbtree_create(sequence_tree, sizeof(ObjectValue), object_sequence_compare)) {
			if (sequence_tree)
				myfree(sequence_tree);
			if (ret)
				myfree(ret);
			if (sorted_tree)
				myfree(sorted_tree);
			if (itt)
				myfree(itt);
			return NULL;
		}

		if (orbtree_create(sorted_tree, sizeof(ObjectValue), object_property_name_compare)) {
			if (sequence_tree)
				myfree(sequence_tree);
			if (ret)
				myfree(ret);
			if (sorted_tree)
				myfree(sorted_tree);
			if (itt)
				myfree(itt);
			return NULL;
		}

		if (orbtree_iterator(sorted_tree, itt, NULL, false, NULL, false, send)) {
			if (sequence_tree)
				myfree(sequence_tree);
			if (ret)
				myfree(ret);
			if (sorted_tree)
				myfree(sorted_tree);
			if (itt)
				myfree(itt);
			return NULL;
		}
	} else {
		if (sequence_tree)
			myfree(sequence_tree);
		if (ret)
			myfree(ret);
		if (sorted_tree)
			myfree(sorted_tree);
		if (itt)
			myfree(itt);
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
	u8 data[];
} ObjectImpl;

void object_cleanup(const Object *ptr) {
	if (ptr == NULL || nil(ptr->impl) || ptr->flags & OBJECT_FLAG_NO_CLEANUP)
		return;
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

ORBContext *object_get_context_and_lock(const ObjectImpl *impl) {
	assert(impl);
	bool send = impl->send;
	if (send) {
		if (!has_global_lock && pthread_rwlock_wrlock(&global_rbtree_lock))
			panic("rwlock error!");
		has_global_lock++;
		return global_orb_context;
	} else {
		return tl_orb_context;
	}
}

void object_unlock(const ObjectImpl *impl) {
	assert(impl);
	bool send = impl->send;
	if (send) {
		if (--has_global_lock == 0)
			if (pthread_rwlock_unlock(&global_rbtree_lock))
				panic("rwlock error!");
	}
}

void object_cleanup_rc(ObjectImpl *impl) {
	assert(impl);
	if (!nil(impl->self)) {
		fam_free(&impl->self);
		impl->self = null;
	}
	if (impl->property_count) {
		u64 size = impl->property_count;
		if (size > MAX_CLEANUP_STACK_SIZE)
			size = 1;
		ObjectNc *to_cleanup_stack[size];
		ObjectNc **to_cleanup;
		if (impl->property_count > MAX_CLEANUP_STACK_SIZE) {
			to_cleanup = mymalloc(impl->property_count * sizeof(Object *));
			if (to_cleanup == NULL)
				panic("Could not allocate enoough memory to cleanup!");
		} else
			to_cleanup = (ObjectNc **)to_cleanup_stack;
		u64 cleanup_count = 0;
		tl_start_range_value.namespace = impl->namespace;
		tl_end_range_value.namespace = impl->namespace + 1;

		bool send = impl->send;

		FatPtr *properties_stack[size];
		FatPtr **properties;

		if (impl->property_count > MAX_CLEANUP_STACK_SIZE) {
			properties = mymalloc(impl->property_count * sizeof(FatPtr *));
			if (properties == NULL)
				panic("Could not allocate enoough memory to cleanup!");
		} else
			properties = properties_stack;

		ORBContext *ctx = object_get_context_and_lock(impl);
		ORBTreeIteratorNc *itt = ctx->itt;
		ORBTreeNc *tree = ctx->sorted_tree;
		ORBTreeNc *sequence_tree = ctx->sequence_tree;

		assert(
			!orbtree_iterator_reset(itt, &tl_start_range_value, true, &tl_end_range_value, false));

		u64 i = 0;
		while (orbtree_iterator_next(itt, &ctx->tray)) {
			ObjectValueNc *v = ctx->tray.value;
			char *nx = $(v->name);
			u64 seqno = v->seqno;
			properties[i] = &v->name;
			i++;
		}
		assert(i == impl->property_count);

		for (i = 0; i < impl->property_count; i++) {
			ObjectValueNc value = INIT_OBJECT_VALUE;
			value.namespace = impl->namespace;
			value.name = *properties[i];
			assert(!orbtree_remove(tree, &value, &ctx->tray));
			ObjectValueNc *rem = ctx->tray.value;
			assert(rem);
			u64 seqno = rem->seqno;
			u32 precision = rem->seqno_precision;
			ObjectValueData *obj_data = $(rem->value);
			assert(obj_data);
			if (obj_data->type == ObjectTypeObject) {
				to_cleanup[cleanup_count++] = (ObjectNc *)obj_data->value;
			}

			fam_free(&rem->value);
			fam_free(&rem->name);
			assert(!orbtree_deallocate_tray(tree, &ctx->tray));

			value.seqno = seqno;
			value.seqno_precision = precision;
			assert(!orbtree_remove(sequence_tree, &value, &ctx->tray));
			rem = ctx->tray.value;
			fam_free(&rem->value);
			fam_free(&rem->name);

			assert(!orbtree_deallocate_tray(sequence_tree, &ctx->tray));
		}

		object_unlock(impl);

		for (i = 0; i < cleanup_count; i++) {
			object_cleanup(to_cleanup[i]);
		}

		if (impl->property_count > MAX_CLEANUP_STACK_SIZE) {
			myfree(to_cleanup);
			myfree(properties);
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
	ObjectNc dst = {.flags = src->flags, .impl = src->impl};
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
	ObjectNc dst = {.flags = src->flags, .impl = rc_null};
	rc_clone(&dst.impl, &src->impl);
	return dst;
}

bool object_is_send(const Object *obj) {
	return (obj->flags & OBJECT_FLAG_SEND) != 0;
}

int object_value_for(ObjectValue *value, const Object *obj, const char *name, u64 seqno,
					 u32 seqno_precision) {
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
	value->seqno = seqno;
	value->seqno_precision = seqno_precision;

	return 0;
}

ObjectValueNc *object_get_property_value(ORBContext *ctx, const Object *obj, const char *name,
										 bool remove) {
	ORBTreeNc *sorted_tree = ctx->sorted_tree;
	ORBTreeNc *sequence_tree = ctx->sequence_tree;
	ctx->tray.updated = false;

	ObjectImpl *impl = $(obj->impl);
	if (impl == NULL) {
		SetErr(IllegalState);
		return NULL;
	}

	ObjectValue objvalue = INIT_OBJECT_VALUE;
	if (object_value_for(&objvalue, obj, name, -1, 0))
		return NULL;
	int v;
	if (remove) {
		v = orbtree_remove(sorted_tree, &objvalue, &ctx->tray);
		if (v == 0 && ctx->tray.updated) {
			ObjectValueNc *rem = ctx->tray.value;
			u64 seqno = rem->seqno;
			u32 precision = rem->seqno_precision;
			ObjectValueData *obj_data = $(rem->value);
			assert(obj_data);
			fam_free(&rem->name);
			assert(!orbtree_deallocate_tray(sorted_tree, &ctx->tray));
			objvalue.seqno = seqno;
			objvalue.seqno_precision = precision;
			ctx->sequence_tray.updated = false;
			assert(!orbtree_remove(sequence_tree, &objvalue, &ctx->sequence_tray));
			assert(ctx->sequence_tray.updated);

			ObjectValueNc *sequence_rem = ctx->sequence_tray.value;
			fam_free(&sequence_rem->name);
			fam_free(&sequence_rem->value);

			assert(!orbtree_deallocate_tray(sequence_tree, &ctx->sequence_tray));
		}
	} else {
		v = orbtree_get(sorted_tree, &objvalue, &ctx->tray);
	}

	if (v || !ctx->tray.updated)
		return NULL;

	ObjectValueNc *valueret = ctx->tray.value;
	return valueret;
}

ObjectValueData *object_get_property_data(ORBContext *ctx, const Object *obj, const char *name,
										  bool remove) {
	ObjectValueNc *valueret = object_get_property_value(ctx, obj, name, remove);
	if (valueret != NULL) {
		ObjectValueData *valueretdata = $(valueret->value);
		return valueretdata;
	} else
		return NULL;
}

int object_set_property_value(Object *obj, const char *name, const void *value, u64 size,
							  ObjectType type) {
	Object to_cleanup = NIL;
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

	int ret = 0;
	ORBContext *ctx = object_get_context_and_lock(impl);

	// first check if it is in the object
	ObjectValueData *vd = object_get_property_data(ctx, obj, name, false);

	if (vd) {
		if (vd->type == ObjectTypeObject) {
			to_cleanup = *(Object *)vd->value;
		}

		ObjectValueNc *valueret = object_get_property_value(ctx, obj, name, false);

		// it exists, overwrite it
		memcpy(vd->value, value, size);

	} else {
		// it doesn't exist, insert it
		int res1 = orbtree_allocate_tray(ctx->sorted_tree, &ctx->tray);
		int res2 = orbtree_allocate_tray(ctx->sequence_tree, &ctx->sequence_tray);

		if (res1 || res2) {
			orbtree_deallocate_tray(ctx->sorted_tree, &ctx->tray);
			orbtree_deallocate_tray(ctx->sequence_tree, &ctx->sequence_tray);
		}

		if (res1 == 0 && res2 == 0) {

			ObjectValueNc val = INIT_OBJECT_VALUE;
			ObjectValueNc val2 = INIT_OBJECT_VALUE;
			int res3 = object_value_for(&val, obj, name, impl->property_seqno_next, 0);
			int res4 = object_value_for(&val2, obj, name, impl->property_seqno_next, 0);
			impl->property_seqno_next++;

			int fres1 = fam_alloc(&val.value, sizeof(ObjectValueData) + size);
			int fres2 = fam_alloc(&val2.value, sizeof(ObjectValueData) + size);

			if (fres1 || fres2) {
				fam_free(&val.value);
				fam_free(&val2.value);
				orbtree_deallocate_tray(ctx->sorted_tree, &ctx->tray);
				orbtree_deallocate_tray(ctx->sequence_tree, &ctx->sequence_tray);
				ret = -1;
			} else {

				ObjectValueData *objdata = $(val.value);
				objdata->type = type;
				ObjectValueData *objdata2 = $(val2.value);
				objdata2->type = type;

				memcpy(objdata->value, value, size);
				memcpy(objdata2->value, value, size);

				if (res3 == 0) {
					memcpy(ctx->tray.value, &val, sizeof(ObjectValue));
					memcpy(ctx->sequence_tray.value, &val2, sizeof(ObjectValue));
					ORBTreeTray retval = {.updated = false};
					int res5 = orbtree_put(ctx->sorted_tree, &ctx->tray, &retval);
					assert(!retval.updated);
					assert(!res5);
					int res6 = orbtree_put(ctx->sequence_tree, &ctx->sequence_tray, &retval);
					assert(!retval.updated);
					assert(!res6);
					impl->property_count++;
				} else {
					ret = -1;
				}
			}

		} else {
			ret = -1;
		}
	}

	object_unlock(impl);

	return ret;
}

Object object_create(bool send, ObjectType type, const void *primitive) {
	if (send) {
		if (object_init_global()) {
			return NIL;
		}

	} else {
		if (object_init_local()) {
			return NIL;
		}
	}

	u64 data_size = 0;
	if (type == ObjectTypeU64) {
		data_size = sizeof(u64);
	}

	FatPtr ptr = null;
	{
		SendStateGuard _ = SetSend(send);
		if (fam_alloc(&ptr, sizeof(ObjectImpl) + data_size)) {
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
	ObjectNc obj = {.flags = 0};
	if (send)
		obj.flags |= OBJECT_FLAG_SEND;

	impl->type = type;
	impl->property_count = 0;
	impl->property_seqno_next = 0;

	if (type == ObjectTypeU64) {
		memcpy(impl->data, primitive, sizeof(u64));
	}

	if (rc_build(&obj.impl, impl, sizeof(ObjectImpl) + data_size, false,
				 (void (*)(void *))object_cleanup_rc)) {
		fam_free(&ptr);
		return NIL;
	}

	u64 size = 0;
	if (type == ObjectTypeString) {
		size = sizeof(char) * (1 + strlen((char *)primitive));
	} else if (type == ObjectTypeU64) {
	}

	if (size) {
		if (object_set_property_value(&obj, "value", primitive, size, type)) {
			object_cleanup(&obj);
			return NIL;
		}
	}

	return obj;
}

ObjectType object_type(const Object *obj) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	return impl->type;
}

int object_send(Object *obj, Channel *channel) {
	return 0;
}

i64 object_properties(const Object *obj) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	return impl->property_count;
}
Object object_set_property(Object *obj, const char *name, const Object *value) {
	object_check_consumed(obj);
	object_check_consumed(value);
	ObjectNc vmove = object_move(value);
	if (object_set_property_value(obj, name, &vmove, sizeof(Object), ObjectTypeObject))
		return NIL;
	return UNIT;
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

	ORBContext *ctx = object_get_context_and_lock(impl);
	const ObjectValueData *vd = object_get_property_data(ctx, obj, name, false);
	object_unlock(impl);

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

	ORBContext *ctx = object_get_context_and_lock(impl);
	const ObjectValueData *vd = object_get_property_data(ctx, obj, name, true);
	if (vd != NULL) {
		impl->property_count--;

		ObjectNc *ret = (Object *)vd->value;

		ObjectNc ref_ret = object_move(ret);
		ObjectValueNc *rem = ctx->tray.value;
		fam_free(&rem->value);
		object_unlock(impl);
		return ref_ret;
	}
	object_unlock(impl);

	return NIL;
}

ObjectValue *object_get_property_index_impl(ORBContext *ctx, const Object *obj, u32 index,
											ObjectIndexType itype) {
	object_check_consumed(obj);

	ObjectImpl *impl = $(obj->impl);

	if (impl == NULL) {
		SetErr(IllegalState);
		return NULL;
	}

	ORBTreeNc *tree;
	if (itype == ObjectIndexTypeSequential)
		tree = ctx->sequence_tree;
	else
		tree = ctx->sorted_tree;

	ctx->tray.updated = false;
	tl_start_range_value.namespace = impl->namespace;
	int v = orbtree_get_index_ranged(tree, index, &ctx->tray, &tl_start_range_value, true);

	if (v || !ctx->tray.updated) {
		return NULL;
	}

	ObjectValueNc *valueret = ctx->tray.value;

	if (valueret->namespace != impl->namespace) {
		return NULL;
	}
	return valueret;
}

Object object_get_property_index(const Object *obj, u32 index, ObjectIndexType itype) {
	ObjectNc ret;
	{
		ObjectImpl *impl = $(obj->impl);

		if (impl == NULL) {
			SetErr(IllegalState);
			return NIL;
		}
		ORBContext *ctx = object_get_context_and_lock(impl);
		const ObjectValueNc *valueret = object_get_property_index_impl(ctx, obj, index, itype);
		object_unlock(impl);
		if (valueret == NULL) {
			return NIL;
		}
		ObjectValueData *vd = $(valueret->value);
		ret = *(Object *)vd->value;
		if (nil(ret)) {
			return NIL;
		}
	}
	return object_ref(&ret);
}

Object object_remove_property_index(Object *obj, u32 index, ObjectIndexType itype) {
	ObjectImpl *impl = $(obj->impl);

	if (impl == NULL) {
		SetErr(IllegalState);
		return NIL;
	}

	ORBContext *ctx = object_get_context_and_lock(impl);

	ObjectValueNc *valueret = object_get_property_index_impl(ctx, obj, index, itype);
	ObjectValueData *vd = $(valueret->value);
	if (vd == NULL) {

		return NIL;
	}

	const char *name = $(valueret->name);
	let res = object_remove_property(obj, name);
	if (nil(res)) {
		return NIL;
	}

	return object_ref(&res);
}

Object object_set_property_index(Object *obj, u32 index, const Object *value,
								 ObjectIndexType itype) {
	ObjectImpl *impl = $(obj->impl);

	if (impl == NULL) {
		SetErr(IllegalState);
		return NIL;
	}

	ORBContext *ctx = object_get_context_and_lock(impl);
	const ObjectValueNc *valueret = object_get_property_index_impl(ctx, obj, index, itype);

	ObjectValueData *vd;
	if (valueret == NULL || valueret->namespace != impl->namespace ||
		(vd = $(valueret->value)) == NULL) {
		object_unlock(impl);
		return NIL;
	}

	const char *name = $(valueret->name);
	ObjectValueNc *obj_val = ctx->tray.value;
	ObjectValueData *obj_data = $(obj_val->value);
	Object to_cleanup;
	memcpy(&to_cleanup, obj_data->value, sizeof(Object));
	memcpy(obj_data->value, value, sizeof(Object));

	ctx->tray.updated = false;
	ObjectValue objvalue = INIT_OBJECT_VALUE;
	object_value_for(&objvalue, obj, name, -1, 0);

	orbtree_get(ctx->sorted_tree, &objvalue, &ctx->tray);
	assert(ctx->tray.updated);
	obj_val = ctx->tray.value;
	obj_data = $(obj_val->value);
	memcpy(obj_data->value, value, sizeof(Object));

	object_unlock(impl);

	return UNIT;
}

Object object_insert_property_before_index(Object *obj, u32 index, const char *name,
										   const Object *value, ObjectIndexType itype) {
	return NIL;
}

const char *object_as_string(const Object *obj) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	ORBContext *ctx = object_get_context_and_lock(impl);
	const ObjectValueData *v = object_get_property_data(ctx, obj, "value", false);
	object_unlock(impl);
	if (v == NULL || v->type != ObjectTypeString) {
		SetErr(ExpectedTypeMismatch);
		return NULL;
	}

	return (char *)v->value;
}
u64 object_as_u64(const Object *obj) {
	object_check_consumed(obj);
	ObjectImpl *impl = $(obj->impl);
	u64 value;
	memcpy(&value, impl->data, sizeof(u64));
	return value;
}

void object_cleanup_thread_local() {
	object_value_cleanup(&tl_start_range_value);
	object_value_cleanup(&tl_end_range_value);
	if (tl_orb_context != NULL) {
		orbtree_iterator_cleanup(tl_orb_context->itt);
		orbtree_cleanup(tl_orb_context->sorted_tree);
		orbtree_cleanup(tl_orb_context->sequence_tree);
		myfree(tl_orb_context->sequence_tree);
		myfree(tl_orb_context->itt);
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
		orbtree_iterator_cleanup(global_orb_context->itt);
		orbtree_cleanup(global_orb_context->sorted_tree);
		orbtree_cleanup(global_orb_context->sequence_tree);
		myfree(global_orb_context->sequence_tree);
		myfree(global_orb_context->itt);
		myfree(global_orb_context->sorted_tree);
		myfree(global_orb_context);
	}
}
#endif // TEST
