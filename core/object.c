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
#include <base/osdef.h>
#include <base/print_util.h>
#include <base/slabs.h>
#include <base/util.h>
#include <core/err.h>
#include <core/object.h>
#include <core/orbtree.h>
#include <sys/mman.h>

#define OBJECT_MAX_FREE_LIST_SIZE 1024

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
	OrbTreeNode seq;
	OrbTreeNode ord;
	unsigned int seqno;
	byte key_value_sso[OBJECT_SSO_DATA_BUFFER_SIZE -
					   (sizeof(unsigned int) + sizeof(OrbTreeNode) * 2)];
} ObjectPropertyBoxData;

// Extended data has spill over from the key_value_sso data:
// 5.) [n bytes of key data]
// 6.) [16 bytes value data]

/*
typedef struct __attribute__((packed)) ObjectBox {
	unsigned int seqno;
	OrbTreeNode seq;
	OrbTreeNode ord;
	unsigned long long namespace;
	Ptr self;
	Ptr data;
} ObjectBox;

typedef struct __attribute__((packed)) ObjectBoxData {
	unsigned int strong_count;
	void *extended;
	byte sso_data[OBJECT_SSO_DATA_BUFFER_SIZE];
	unsigned int weak_count;
	unsigned int len;
} ObjectBoxData;

typedef struct __attribute__((packed)) ObjectPropertyBoxData {
	unsigned int key_len;
	byte sso_key[31];
	byte value_type;
	byte property_value[4];
} ObjectPropertyBoxData;
*/

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
				MUNMAP(box->extended, box->size - OBJECT_SSO_DATA_BUFFER_SIZE);
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

Object object_create_box(unsigned int size) {
	Ptr ptr = slab_allocator_allocate(&sa);
	if (ptr == null) return Err(fam_err);
	ObjectBox *box = (ObjectBox *)slab_get(&sa, ptr);
	box->size = size;
	box->lock = INIT_LOCK;
	box->resize_seqno = 0;

	if (size > OBJECT_SSO_DATA_BUFFER_SIZE) {
		box->extended = MMAP(size - OBJECT_SSO_DATA_BUFFER_SIZE);
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
				MMAP_ALIGNED_SIZE(size - OBJECT_SSO_DATA_BUFFER_SIZE);
		size_t cur_aligned_size = 0;
		if (box->size > OBJECT_SSO_DATA_BUFFER_SIZE)
			cur_aligned_size =
				MMAP_ALIGNED_SIZE(box->size - OBJECT_SSO_DATA_BUFFER_SIZE);

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

			if (MUNMAP((byte *)extended + aligned_size, diff))
				panic("Unexpected error returned by munmap!");

			return Unit;
		} else {
			void *extended = box->extended;
			// we need more memory so call mmap
			void *tmp = MMAP(size - OBJECT_SSO_DATA_BUFFER_SIZE);
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
				if (MUNMAP(extended, cur_aligned_size))
					panic("Unexpected error returned by munmap!");
			}
			return Unit;
		}
	}
}
