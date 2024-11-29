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

#include <base/err.h>
#include <base/limits.h>
#include <base/object.h>
#include <base/print_util.h>
#include <base/slabs.h>
#include <base/sys.h>
#include <base/util.h>

#define OBJ_SLAB_SIZE (128 - SLAB_LIST_SIZE)
#define OBJ_SLAB_FREE_LIST_SIZE 1024

SlabAllocator object_slabs;

typedef struct ObjectImpl {
	union {
		unsigned char bytes[8];
		long long int_value;
		unsigned long long uint_value;
		double float_value;
		int code_value;
		void *ptr_value;
	} value;
	unsigned long long type;
} ObjectImpl;

typedef struct BoxSlabData {
	void *extended;
	unsigned long long pages;
} BoxSlabData;

#define OBJ_BOX_USER_DATA_SIZE (OBJ_SLAB_SIZE - sizeof(BoxSlabData))

void __attribute__((constructor)) __setup_object_impl() {
	if (sizeof(ObjectImpl) != sizeof(__int128_t)) {
		panic("sizeof(ObjectImpl) ({}) != sizeof(Object) ({})",
			  sizeof(ObjectImpl), sizeof(Object));
	}

	let res = slab_allocator_init(&object_slabs, OBJ_SLAB_SIZE, UINT32_MAX,
								  OBJ_SLAB_FREE_LIST_SIZE);
}

Object object_int(long long value) {
	ObjectImpl ret = {.type = Int, .value.int_value = value};
	return *((Object *)&ret);
}

Object object_uint(unsigned long long value) {
	ObjectImpl ret = {.type = UInt, .value.uint_value = value};
	return *((Object *)&ret);
}

Object object_float(double value) {
	ObjectImpl ret = {.type = Float, .value.float_value = value};
	return *((Object *)&ret);
}

Object object_function(void *fn) {
	ObjectImpl ret = {.type = Function, .value.ptr_value = fn};
	return *((Object *)&ret);
}

Object object_err(int code) {
	ObjectImpl ret = {.type = Err, .value.code_value = code};
	return *((Object *)&ret);
}

Object object_string(const char *s) {
	ObjectImpl ret = {.type = Err, .value.code_value = NotYetImplemented};
	return *((Object *)&ret);
}

Object box(long long size) {
	void *slab = slab_allocator_allocate(&object_slabs);
	if (slab == 0)
		return Err(AllocErr);
	else {
		if (size > OBJ_BOX_USER_DATA_SIZE) {
			set_bytes(slab + sizeof(BoxSlabData), '\0', OBJ_BOX_USER_DATA_SIZE);
			BoxSlabData *bsd = slab;
			long long needed = size - OBJ_BOX_USER_DATA_SIZE;
			bsd->pages = (needed + PAGE_SIZE - 1) / PAGE_SIZE;
			bsd->extended = map(bsd->pages);
			if (bsd->extended == 0) {
				slab_allocator_free(&object_slabs, slab);
				return Err(AllocErr);
			}
		} else
			set_bytes(slab, '\0', OBJ_SLAB_SIZE);
		ObjectImpl ret = {.type = Box, .value.ptr_value = slab};
		return *((Object *)&ret);
	}
}

const void *value_of(const Object *obj) {
	ObjectImpl *impl = (ObjectImpl *)obj;
	if (impl->type == Function) return impl->value.ptr_value;
	return &impl->value.bytes;
}

const void *value_of_checked(const Object *obj, ObjectType expect) {
	ObjectType type = object_type(obj);
	if (type != expect)
		panic("Expected Object type {}. Found {}!", (int)expect, (int)type);
	return value_of(obj);
}

ObjectType object_type(const Object *obj) {
	return ((ObjectImpl *)obj)->type;
}

void object_cleanup(const Object *obj) {
	if (object_type(obj) == Box) {
		ObjectImpl *impl = (ObjectImpl *)obj;
		BoxSlabData *bsd = impl->value.ptr_value;
		if (bsd->pages) unmap(bsd->extended, bsd->pages);
		slab_allocator_free(&object_slabs, impl->value.ptr_value);
	}
}
