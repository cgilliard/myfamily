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

#define OFFSET_ORDERED 0

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
	unsigned int type : 4;
	unsigned int no_cleanup : 1;
	unsigned int consumed : 1;
} ObjectImpl;

typedef struct ObjectProperty {
	OrbTreeNode ordered;
	const char *name;
	unsigned long long seqno;
	OrbTreeNode seq;
	Object value;
} ObjectProperty;

void __attribute__((constructor)) __setup_object_impl() {
	if (sizeof(ObjectImpl) != sizeof(Object)) {
		panic("sizeof(ObjectImpl) ({}) != sizeof(Object) ({})",
			  sizeof(ObjectImpl), sizeof(Object));
	}
	let res = slab_allocator_init(&object_slabs, OBJ_SLAB_SIZE, UINT32_MAX,
								  OBJ_SLAB_FREE_LIST_SIZE);
}

void check_consumed(const Object *obj) {
	if (obj == 0) panic("Object is NULL");
	if (((const ObjectImpl *)obj)->consumed) panic("Object consumed!");
}

Object object_move(Object *obj) {
	if (object_type(obj) != Box) return *obj;
	Object ret = *obj;
	(*(ObjectImpl *)obj).no_cleanup = (*(ObjectImpl *)obj).consumed = 1;
	return ret;
}

Object object_ref(Object *obj) {
	if (object_type(obj) != Box) return *obj;
	ObjectImpl *impl = (ObjectImpl *)obj;
	BoxSlabData *bsd = impl->value.ptr_value;
	bsd->ref_count++;
	return *obj;
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
			bsd->ordered = INIT_ORBTREE;
			bsd->seq = INIT_ORBTREE;
			bsd->ref_count = 1;
		} else {
			set_bytes(slab, '\0', OBJ_SLAB_SIZE);
			BoxSlabData *bsd = slab;
			bsd->ref_count = 1;
		}
		ObjectImpl ret = {.type = Box, .value.ptr_value = slab};
		return *((Object *)&ret);
	}
}

Object box_resize(Object *obj, long long size) {
	check_consumed(obj);
	ObjectType type = object_type(obj);
	if (type != Box) panic("Expected box found type: {}", type);
	ObjectImpl *impl = (ObjectImpl *)obj;
	BoxSlabData *bsd = impl->value.ptr_value;
	if (size <= OBJ_BOX_USER_DATA_SIZE) {
		if (bsd->pages) {
			unmap(bsd->extended, bsd->pages);
			bsd->pages = 0;
			bsd->extended = 0;
		}
	} else {
		long long needed = size - OBJ_BOX_USER_DATA_SIZE;
		long long pages = (needed + PAGE_SIZE - 1) / PAGE_SIZE;
		if (pages > bsd->pages) {
			void *naddr = map(pages);
			if (naddr == 0) return Err(AllocErr);
			if (bsd->pages) {
				copy_bytes(naddr, bsd->extended, bsd->pages * PAGE_SIZE);
			}
			bsd->extended = naddr;
			bsd->pages = pages;
		} else if (pages < bsd->pages) {
			unmap(((unsigned char *)bsd->extended) + pages * PAGE_SIZE,
				  bsd->pages - pages);
		}
	}

	return $(0);
}

void *box_get_long_bytes(const Object *obj) {
	check_consumed(obj);
	ObjectType type = object_type(obj);
	if (type != Box) panic("Expected box found type: {}", type);
	ObjectImpl *impl = (ObjectImpl *)obj;
	BoxSlabData *bsd = impl->value.ptr_value;
	return bsd->extended;
}

void *box_get_short_bytes(const Object *obj) {
	check_consumed(obj);
	ObjectType type = object_type(obj);
	if (type != Box) panic("Expected box found type: {}", type);
	ObjectImpl *impl = (ObjectImpl *)obj;
	void *ptr = impl->value.ptr_value;
	return (unsigned char *)ptr + sizeof(BoxSlabData);
}

unsigned long long box_get_page_count(const Object *obj) {
	check_consumed(obj);
	ObjectType type = object_type(obj);
	if (type != Box) panic("Expected box found type: {}", type);
	ObjectImpl *impl = (ObjectImpl *)obj;
	BoxSlabData *bsd = impl->value.ptr_value;
	return bsd->pages;
}

const void *value_of(const Object *obj) {
	check_consumed(obj);
	ObjectImpl *impl = (ObjectImpl *)obj;
	if (impl->type == Function) return impl->value.ptr_value;
	return &impl->value.bytes;
}

const void *value_of_checked(const Object *obj, ObjectType expect) {
	check_consumed(obj);
	ObjectType type = object_type(obj);
	if (type != expect)
		panic("Expected Object type {}. Found {}!", (int)expect, (int)type);
	return value_of(obj);
}

ObjectType object_type(const Object *obj) {
	check_consumed(obj);
	return ((ObjectImpl *)obj)->type;
}

void object_cleanup_node(OrbTreeNode *node) {
	if (node->right) object_cleanup_node(node->right);
	if (node->left) object_cleanup_node(node->left);
	ObjectProperty *op = (ObjectProperty *)(node - OFFSET_ORDERED);
	object_cleanup(&op->value);
	slab_allocator_free(&object_slabs, op);
}

void object_cleanup(const Object *obj) {
	if (((ObjectImpl *)obj)->type == Box) {
		ObjectImpl *impl = (ObjectImpl *)obj;
		if (impl->no_cleanup) return;
		BoxSlabData *bsd = impl->value.ptr_value;
		if (--bsd->ref_count != 0) return;

		Object drop = object_get_property(obj, "drop");
		if (!$is_err(drop)) {
			void (*drop_impl)(Object *) = (void (*)(Object *))$fn(&drop);
			drop_impl(obj);
		}

		if (bsd->pages) unmap(bsd->extended, bsd->pages);
		if (bsd->ordered.root) object_cleanup_node(bsd->ordered.root);
		slab_allocator_free(&object_slabs, impl->value.ptr_value);
	}
}

int object_search_ordered(OrbTreeNode *cur, const OrbTreeNode *value,
						  OrbTreeNodePair *retval) {
	while (cur) {
		const char *v1 =
			((ObjectProperty *)((unsigned char *)cur - OFFSET_ORDERED))->name;
		const char *v2 =
			((ObjectProperty *)((unsigned char *)value - OFFSET_ORDERED))->name;

		int c = cstring_compare(v1, v2);
		if (c == 0) {
			retval->self = cur;
			break;
		} else if (c < 0) {
			retval->parent = cur;
			retval->is_right = 1;
			cur = cur->right;
		} else {
			retval->parent = cur;
			retval->is_right = 0;
			cur = cur->left;
		}
		retval->self = cur;
	}
	return 0;
}

Object object_get_property(const Object *obj, const char *name) {
	check_consumed(obj);
	ObjectProperty op = {.name = name};
	OrbTreeNodePair retval = {};
	ObjectImpl *impl = (ObjectImpl *)obj;
	BoxSlabData *bsd = impl->value.ptr_value;
	object_search_ordered(bsd->ordered.root, &op.ordered, &retval);

	if (retval.self)
		return ((ObjectProperty *)(retval.self - OFFSET_ORDERED))->value;
	else
		return Err(NotFound);
}

Object object_set_property(Object *obj, const char *name, const Object *value) {
	check_consumed(obj);
	check_consumed(value);

	ObjectProperty *opptr = slab_allocator_allocate(&object_slabs);
	opptr->name = name;
	opptr->value = object_ref(value);
	ObjectImpl *impl = (ObjectImpl *)obj;
	BoxSlabData *bsd = impl->value.ptr_value;
	OrbTreeNode *ret =
		orbtree_put(&bsd->ordered, &opptr->ordered, object_search_ordered);
	if (ret) slab_allocator_free(&object_slabs, ret - OFFSET_ORDERED);
	return $(0);
}

Object object_remove_property(Object *obj, const char *name) {
	check_consumed(obj);
	return Err(NotYetImplemented);
}
