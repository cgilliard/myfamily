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

#include <base/lock.h>
#include <base/macros.h>
#include <base/mmap.h>
#include <base/print_util.h>
#include <base/slabs.h>
#include <base/util.h>
#include <core/err.h>
#include <core/macros.h>
#include <core/object.h>
#include <core/orbtree.h>
#include <stddef.h>

// #define offsetof(type, member) ((unsigned long long)&(((type *)0)->member))

#define OBJECT_MAX_FREE_LIST_SIZE 1024
#define OBJECT_PROPERTY_KV_SSO_LEN \
	OBJECT_SSO_DATA_BUFFER_SIZE -  \
		(sizeof(unsigned int) + sizeof(OrbTreeNode) * 2)

static OrbTree seqtree;
static OrbTree ordtree;
static SlabAllocator sa;
static unsigned long long next_namespace = 0;

typedef struct ObjectImpl {
	union {
		int int_value;
		float float_value;
		byte byte_value;
		bool bool_value;
		Ptr ptr_value;
		void *function_value;
	} data;
	unsigned int type : 6;
	unsigned int consumed : 1;
	unsigned int no_cleanup : 1;
} ObjectImpl;

typedef struct __attribute__((packed)) ObjectBox {
	unsigned int size;
	unsigned long long namespace;
	byte sso_data[OBJECT_SSO_DATA_BUFFER_SIZE];
	unsigned int resize_seqno;
	Lock lock;
	void *extended;
	unsigned int strong_count;
	unsigned int weak_count;
} ObjectBox;

typedef struct __attribute__((packed)) ObjectPropertyBoxData {
	OrbTreeNode ord;
	OrbTreeNode seq;
	unsigned int seqno;
	byte key_value_sso[OBJECT_PROPERTY_KV_SSO_LEN];
} ObjectPropertyBoxData;

// Extended data has spill over from the key_value_sso data:
// 5.) [n bytes of key data]
// 6.) [16 bytes value data]

static ObjectBox *object_get_box_data(ObjectImpl *impl) {
	Ptr ptr = impl->data.ptr_value;
	if (ptr == null) return NULL;
	return (ObjectBox *)slab_get(&sa, ptr);
}

static int ordtree_search(const OrbTreeNode *root, const OrbTreeNode *value,
						  OrbTreeNodePair *retval) {
	retval->parent = null;
	retval->is_right = true;
	const OrbTreeNode *cur = root;
	unsigned int offsetof = offsetof(ObjectBox, sso_data);

	while (cur) {
		ObjectBox *v1 = (ObjectBox *)((byte *)cur - offsetof);
		ObjectBox *v2 = (ObjectBox *)((byte *)value - offsetof);
		retval->self = orbtree_node_ptr(cur, retval->is_right);

		if (v1->namespace == v2->namespace) {
			ObjectPropertyBoxData *v1prop =
				(ObjectPropertyBoxData *)v1->sso_data;
			ObjectPropertyBoxData *v2prop =
				(ObjectPropertyBoxData *)v2->sso_data;
			byte *key1 = v1prop->key_value_sso;
			byte *key2 = v2prop->key_value_sso;
			unsigned int len;
			if (v2->size < v1->size)
				len = v2->size;
			else
				len = v1->size;
			unsigned int overhead =
				sizeof(OrbTreeNode) * 2 + sizeof(unsigned int) + sizeof(Object);
			if (len < overhead) break;
			len -= overhead;
			int v = cstring_compare_n(key1, key2, len);
			if (v == 0 && v1->size == v2->size)
				break;
			else if ((v == 0 && v1->size > v2->size) || v > 0) {
				ObjectBox *right = orbtree_node_right(cur);
				if (right == NULL) {
					retval->parent = retval->self;
					retval->self = null;
					retval->is_right = true;
					break;
				}
				ObjectPropertyBoxData *opbd =
					(ObjectPropertyBoxData *)right->sso_data;
				Ptr rptr = orbtree_node_ptr(&opbd->ord, true);
				retval->parent = orbtree_node_ptr(cur, retval->is_right);
				retval->is_right = true;
				cur = (const OrbTreeNode *)(slab_get(&sa, rptr) + offsetof);
			} else {
				ObjectBox *left = orbtree_node_left(cur);
				if (left == NULL) {
					retval->parent = retval->self;
					retval->self = null;
					retval->is_right = false;
					break;
				}
				ObjectPropertyBoxData *opbd =
					(ObjectPropertyBoxData *)left->sso_data;
				Ptr lptr = orbtree_node_ptr(&opbd->ord, false);
				retval->parent = orbtree_node_ptr(cur, retval->is_right);
				retval->is_right = false;
				cur = (const OrbTreeNode *)(slab_get(&sa, lptr) + offsetof);
			}

		} else if (v1->namespace > v2->namespace) {
			ObjectBox *right = orbtree_node_right(cur);
			if (right == NULL) {
				retval->parent = retval->self;
				retval->self = null;
				retval->is_right = true;
				break;
			}
			ObjectPropertyBoxData *opbd =
				(ObjectPropertyBoxData *)right->sso_data;
			Ptr rptr = orbtree_node_ptr(&opbd->ord, true);
			retval->parent = orbtree_node_ptr(cur, retval->is_right);
			retval->is_right = true;
			cur = (const OrbTreeNode *)(slab_get(&sa, rptr) + offsetof);
		} else {
			ObjectBox *left = orbtree_node_left(cur);
			if (left == NULL) {
				retval->parent = retval->self;
				retval->self = null;
				retval->is_right = false;
				break;
			}
			ObjectPropertyBoxData *opbd =
				(ObjectPropertyBoxData *)left->sso_data;
			Ptr lptr = orbtree_node_ptr(&opbd->ord, false);
			retval->parent = orbtree_node_ptr(cur, retval->is_right);
			retval->is_right = false;
			cur = (const OrbTreeNode *)(slab_get(&sa, lptr) + offsetof);
		}
	}

	return 0;
}

static __attribute__((constructor)) void object_init() {
	if (sizeof(Object) != sizeof(ObjectImpl))
		panic("sizeof(ObjectImpl) (%i) != sizeof(Object) (%i)",
			  sizeof(ObjectImpl), sizeof(Object));
	if (sizeof(Object) != 16)
		panic("sizeof(Object) (%i) != 16", sizeof(Object));
	if (sizeof(ObjectBox) != 124)
		panic("sizeof(ObjectBox) (%i) != 124", sizeof(ObjectBox));
	if (sizeof(ObjectPropertyBoxData) != OBJECT_SSO_DATA_BUFFER_SIZE)
		panic("sizeof(ObjectPropertyBoxData) (%i) != %i",
			  sizeof(ObjectPropertyBoxData), OBJECT_SSO_DATA_BUFFER_SIZE);

	ASTORE(&next_namespace, 0);
	slab_allocator_init(&sa, sizeof(ObjectBox), OBJECT_MAX_FREE_LIST_SIZE,
						UINT32_MAX);
	orbtree_init(&seqtree, &sa);
	orbtree_init(&ordtree, &sa);
}

void object_cleanup(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	if (impl->no_cleanup) return;
	if (impl->type == ObjectTypeBox) {
		if (impl->data.ptr_value != null) {
			ObjectBox *box = (ObjectBox *)slab_get(&sa, impl->data.ptr_value);

			if (box->extended) {
				mmap_free(box->extended,
						  box->size - OBJECT_SSO_DATA_BUFFER_SIZE);
				box->extended = NULL;
			}

			slab_allocator_free(&sa, impl->data.ptr_value);
			impl->data.ptr_value = null;
		}
	}
}

ObjectType object_type(const Object *obj) {
	return ((ObjectImpl *)obj)->type;
}

Object object_create_int(int value) {
	ObjectImpl ret = {.data.int_value = value, .type = ObjectTypeInt};
	return *((Object *)&ret);
}

Object object_create_byte(byte b) {
	ObjectImpl ret = {.data.byte_value = b, .type = ObjectTypeByte};
	return *((Object *)&ret);
}

Object object_create_unit() {
	ObjectImpl ret = {.data.int_value = 0, .type = ObjectTypeUnit};
	return *((Object *)&ret);
}

Object object_create_err(int code) {
	ObjectImpl ret = {.data.int_value = code, .type = ObjectTypeErr};
	return *((Object *)&ret);
}

Object object_create_function(void *fn) {
	ObjectImpl ret = {.data.function_value = fn, .type = ObjectTypeFunction};
	return *((Object *)&ret);
}

Object object_set_property(Object *obj, const char *key, const Object *value) {
	ObjectBox *impl = object_get_box_data((ObjectImpl *)obj);
	unsigned long long key_len = cstring_len(key);
	unsigned int box_size = sizeof(OrbTreeNode) * 2 + sizeof(unsigned int) +
							sizeof(Object) + key_len;
	Object property = object_create_box(box_size);
	Ptr property_ptr = ((ObjectImpl *)&property)->data.ptr_value;
	ObjectBox *property_impl = object_get_box_data((ObjectImpl *)&property);

	property_impl->namespace = impl->namespace;
	ObjectPropertyBoxData *property_data = object_box_sso(&property);
	property_data->seqno = 0;
	unsigned int sso_copy_len = OBJECT_PROPERTY_KV_SSO_LEN;
	if (key_len < OBJECT_PROPERTY_KV_SSO_LEN) sso_copy_len = key_len;
	copy_bytes(property_data->key_value_sso, key, sso_copy_len);
	unsigned int rem = sso_copy_len - OBJECT_PROPERTY_KV_SSO_LEN;
	if (rem) {
		if (rem > sizeof(Object)) rem = sizeof(Object);
		copy_bytes(property_data->key_value_sso + sso_copy_len, (byte *)value,
				   rem);

		ObjectImpl *tmp =
			(ObjectImpl *)(property_data->key_value_sso + sso_copy_len);
	}

	if (rem != sizeof(Object)) {
		// we have to write more data to the extended buffer
	}

	unsigned long long offsetof = offsetof(ObjectBox, sso_data);
	OrbTreeNodeWrapper wrap = {.ptr = property_ptr, .offsetof = offsetof};
	orbtree_put(&ordtree, &wrap, &ordtree_search);

	return Unit;
}

Object object_delete_property(Object *obj, const char *key) {
	return Err(NotYetImplemented);
}

Object object_get_property(const Object *obj, const char *key) {
	ObjectBox *impl = object_get_box_data((ObjectImpl *)obj);
	unsigned int key_len = cstring_len(key);
	unsigned int box_size = sizeof(OrbTreeNode) * 2 + sizeof(unsigned int) +
							key_len + sizeof(Object);
	unsigned long long namespace = impl->namespace;

	Object property = object_create_box(box_size);
	Ptr property_ptr = ((ObjectImpl *)&property)->data.ptr_value;
	ObjectBox *box = object_get_box_data((ObjectImpl *)&property);

	/*
		Ptr ptr = slab_allocator_allocate(&sa);
		ObjectBox *box = (ObjectBox *)slab_get(&sa, ptr);
	*/
	box->namespace = namespace;
	box->size = box_size;

	ObjectPropertyBoxData *property_data = object_box_sso(&property);

	unsigned int sso_copy_len = OBJECT_PROPERTY_KV_SSO_LEN;
	if (key_len < OBJECT_PROPERTY_KV_SSO_LEN) sso_copy_len = key_len;
	copy_bytes(property_data->key_value_sso, key, sso_copy_len);

	unsigned long long offsetof = offsetof(ObjectBox, sso_data);
	OrbTreeNodeWrapper wrap = {.ptr = property_ptr, .offsetof = offsetof};
	Ptr ret = orbtree_get(&ordtree, &wrap, &ordtree_search, 0);

	// for now we return an error, eventually we'll implement Option
	if (ret == null) return Err(FileNotFound);

	ObjectBox *ret_box = (ObjectBox *)slab_get(&sa, ret);
	ObjectPropertyBoxData *property_data_out =
		(ObjectPropertyBoxData *)ret_box->sso_data;
	unsigned int overhead =
		sizeof(OrbTreeNode) * 2 + sizeof(unsigned int) + sizeof(Object);
	if (overhead > ret_box->size) return Err(FileNotFound);
	unsigned int offset = ret_box->size - overhead;
	ObjectNc ret_obj;
	*(&ret_obj) = *(Object *)(property_data_out->key_value_sso + offset);

	return ret_obj;
}

Object object_create_box(unsigned int size) {
	Ptr ptr = slab_allocator_allocate(&sa);
	if (ptr == null) return Err(fam_err);
	ObjectBox *box = (ObjectBox *)slab_get(&sa, ptr);
	box->namespace = AADD(&next_namespace, 1);
	box->size = size;
	box->lock = INIT_LOCK;
	box->resize_seqno = 0;

	if (size > OBJECT_SSO_DATA_BUFFER_SIZE) {
		box->extended = mmap_allocate(size - OBJECT_SSO_DATA_BUFFER_SIZE);
		if (box->extended == NULL) {
			slab_allocator_free(&sa, ptr);
			return Err(AllocErr);
		}
	} else
		box->extended = NULL;

	ObjectImpl ret = {.data = ptr, .type = ObjectTypeBox};
	return *((Object *)&ret);
}

void *object_box_sso(const Object *obj) {
	Ptr ptr = ((ObjectImpl *)obj)->data.ptr_value;
	if (ptr == null) return NULL;
	ObjectBox *box = (ObjectBox *)slab_get(&sa, ptr);
	if (box == NULL) return NULL;
	return box->sso_data;
}

void *object_box_extended(const Object *obj) {
	Ptr ptr = ((ObjectImpl *)obj)->data.ptr_value;
	if (ptr == null) return NULL;
	ObjectBox *box = (ObjectBox *)slab_get(&sa, ptr);
	if (box == NULL) return NULL;
	return box->extended;
}

unsigned int object_box_size(const Object *obj) {
	Ptr ptr = ((ObjectImpl *)obj)->data.ptr_value;
	if (ptr == null) return 0;
	ObjectBox *box = (ObjectBox *)slab_get(&sa, ptr);
	if (box == NULL) return 0;
	return box->size;
}

int object_value_of(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	return impl->data.int_value;
}

void *object_value_function(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	return impl->data.function_value;
}

#pragma clang diagnostic ignored "-Waddress-of-packed-member"
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
Object object_resize_box(Object *obj, unsigned int size) {
	Ptr ptr = ((ObjectImpl *)obj)->data.ptr_value;
	if (ptr == null) return Err(IllegalState);
	ObjectBox *box = (ObjectBox *)slab_get(&sa, ptr);
	if (box == NULL) return Err(IllegalState);
	int count = 0;

	loop {
		if (++count >= 10000) {
			return Err(Busy);
		}
		unsigned int resize_seqno = box->resize_seqno;
		size_t aligned_size = 0;
		if (size > OBJECT_SSO_DATA_BUFFER_SIZE)
			aligned_size =
				mmap_aligned_size(size - OBJECT_SSO_DATA_BUFFER_SIZE);
		size_t cur_aligned_size = 0;
		if (box->size > OBJECT_SSO_DATA_BUFFER_SIZE)
			cur_aligned_size =
				mmap_aligned_size(box->size - OBJECT_SSO_DATA_BUFFER_SIZE);

		if (aligned_size == cur_aligned_size) {
			// no resize needed

			lockw(&box->lock);
			if (box->resize_seqno != resize_seqno) {
				unlock(&box->lock);
				continue;
			}
			box->size = size;
			unlock(&box->lock);
			return Unit;
		} else if (aligned_size < cur_aligned_size) {
			// shrink memeory map
			unsigned int diff = cur_aligned_size - aligned_size;
			void *extended = box->extended;
			if (extended == NULL) panic("extended must not be NULL!");

			lockw(&box->lock);

			if (box->resize_seqno != resize_seqno) {
				unlock(&box->lock);
				continue;
			}

			box->size = size;
			if (aligned_size == 0) box->extended = NULL;

			unlock(&box->lock);

			mmap_free((byte *)extended + aligned_size, diff);

			return Unit;
		} else {
			void *extended = box->extended;
			// we need more memory so call mmap
			void *tmp = mmap_allocate(size - OBJECT_SSO_DATA_BUFFER_SIZE);
			if (tmp == NULL) return Err(AllocErr);

			lockw(&box->lock);

			if (box->resize_seqno != resize_seqno) {
				unlock(&box->lock);
				continue;
			}

			if (cur_aligned_size) {
				if (extended == NULL) panic("extended must not be NULL!");
				copy_bytes(tmp, extended, cur_aligned_size);
			}

			box->size = size;
			box->extended = tmp;

			unlock(&box->lock);

			if (cur_aligned_size) {
				if (extended == NULL) panic("extended must not be NULL!");
				mmap_free(extended, cur_aligned_size);
			}
			return Unit;
		}
	}
}
