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
	void *extended;
	byte sso_data[OBJECT_SSO_DATA_BUFFER_SIZE];
} ObjectBox;

typedef struct __attribute__((packed)) ObjectPropertyBoxData {
	OrbTreeNode seq;
	unsigned long long namespace;
	OrbTreeNode ord;
} ObjectPropertyBoxData;

// Extended data has:
// 1.) 4 byte strong count
// 2.) 4 byte weak count
// 3.) 4 byte seqno
// 4.) 0 byte key_len (we can determine key length based on total length of
// extended data)
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
		panic("sizeof(Object) (%i) != 60", sizeof(Object));
	if (sizeof(ObjectBox) != 60)
		panic("sizeof(ObjectBox) (%i) != 60", sizeof(ObjectBox));
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

int object_value_of(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	return impl->data.int_value;
}

void *object_value_function(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	return impl->data.function_value;
}
