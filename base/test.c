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

#include <base/test.h>

Suite(base);

MyTest(base, test_lock) {
	Lock l1 = INIT_LOCK;
	lock_read(&l1);
	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 1);
	// lower bits also incremeneted by 1.
	cr_assert_eq(l1 & 0xFFFFFFFF, 1);

	lock_unlock(&l1);
	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 2);
	// lower bits also decremented by 1.
	cr_assert_eq(l1 & 0xFFFFFFFF, 0);

	lock_write(&l1);
	// upper bits incremented by 2.
	cr_assert_eq(l1 >> 32, 4);
	// lower bits high bit set
	cr_assert_eq(l1 & 0xFFFFFFFF, 0x80000000);

	lock_unlock(&l1);

	// upper bits incremented by 1.
	cr_assert_eq(l1 >> 32, 5);
	// lower bits are set to 0 now
	cr_assert_eq(l1 & 0xFFFFFFFF, 0x00000000);
}

MyTest(base, test_lock_macros) {
	int x = 0;
	Lock l = INIT_LOCK;
	wsync(l, { x++; });
	cr_assert(x == 1);
	int y;
	rsync(l, { y = x; });
	cr_assert(y == 1);
}

MyTest(base, test_memmap) {
	MemMap mm1;
	int size = 64;
	int count = 3000;
	memmap_init(&mm1, size);

	for (int i = 0; i < count; i++) {
		Ptr p = memmap_allocate(&mm1);
		if (p != i + 2) println("p=%u,i+1=%i", p, i + 2);
		cr_assert_eq(p, i + 2);
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			data[j] = 'a' + (j % 26);
		}
	}

	for (int i = 0; i < count; i++) {
		Ptr p = i + 2;
		byte *data = memmap_data(&mm1, p);
		for (int j = 0; j < size; j++) {
			cr_assert_eq(data[j], 'a' + (j % 26));
		}
	}

	memmap_free(&mm1, 7);
	memmap_free(&mm1, 8);
	memmap_free(&mm1, 167);

	Ptr p = memmap_allocate(&mm1);
	cr_assert_eq(p, 7);
	p = memmap_allocate(&mm1);
	cr_assert_eq(p, 8);
	p = memmap_allocate(&mm1);
	cr_assert_eq(p, 167);

	p = memmap_allocate(&mm1);
	cr_assert_eq(p, count + 2);

	memmap_cleanup(&mm1);
}

MyTest(base, test_memmap_recycle) {
	MemMap mm1;
	int size = 64;
	int count = 3000000;
	int itt = 4;
	memmap_init(&mm1, size);
	Ptr ptrs[itt];

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < itt; j++) {
			ptrs[j] = memmap_allocate(&mm1);
			cr_assert(ptrs[j] == 2 + j);
		}

		for (int j = 0; j < itt; j++) {
			cr_assert(ptrs[j] == 2 + j);
			memmap_free(&mm1, ptrs[j]);
		}
	}
	memmap_cleanup(&mm1);
}

MyTest(base, test_slab_allocator) {
	SlabAllocator sa1;
	cr_assert(!slab_allocator_init(&sa1, 16, 100, 200));
	int size = 5;
	Ptr arr[size];

	for (int i = 0; i < size; i++) {
		Ptr p = slab_allocator_allocate(&sa1);
		arr[i] = p;
		byte *parr = slab_get(&sa1, p);
		parr[0] = i + 2;
	}

	for (int i = 0; i < size; i++) {
		byte *parr = slab_get(&sa1, arr[i]);
		cr_assert_eq(parr[0], i + 2);
		slab_allocator_free(&sa1, arr[i]);
	}

	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_slab_allocator_recycle) {
	long long size = 1024 * 1024 * 1;
	int count = 5;
	int alloc_size = 64;

	SlabAllocator sa1;
	cr_assert(!slab_allocator_init(&sa1, alloc_size, count + 5, count + 5));
	Ptr slabs[count];

	for (long long i = 0; i < size; i++) {
		for (int j = 0; j < count; j++) {
			slabs[j] = slab_allocator_allocate(&sa1);
			cr_assert(slabs[j] != null);
		}
		for (int j = 0; j < count; j++) {
			slab_allocator_free(&sa1, slabs[j]);
		}
	}

	slab_allocator_cleanup(&sa1);
}

MyTest(base, test_malloc_recycle) {
	long long size = 1024 * 1024 * 1;
	int count = 5;
	int alloc_size = 64;

	byte *slabs[count];

	for (long long i = 0; i < size; i++) {
		for (int j = 0; j < count; j++) {
			slabs[j] = malloc(alloc_size);
			cr_assert(slabs[j] != NULL);
		}

		for (int j = 0; j < count; j++) {
			free(slabs[j]);
		}
	}
}

MyTest(base, test_slab_allocator_adv) {
	SlabAllocator sa1;
	int slab_len = 128;
	cr_assert(!slab_allocator_init(&sa1, slab_len, 100000, 2000000));
	int size = 50000;
	Ptr *arr = malloc(sizeof(Ptr) * size);

	for (int i = 0; i < size; i++) {
		Ptr p = slab_allocator_allocate(&sa1);
		arr[i] = p;
		byte *parr = slab_get(&sa1, p);
		parr[0] = (i + 2) % 256;
		for (int j = 1; j < slab_len; j++) {
			parr[j] = (((i + 2) + j) % 26) + 'a';
		}
	}

	for (int i = 0; i < size; i++) {
		byte *parr = slab_get(&sa1, arr[i]);
		cr_assert_eq(parr[0], (i + 2) % 256);
		for (int j = 1; j < slab_len; j++)
			cr_assert_eq(parr[j], (((i + 2) + j) % 26) + 'a');
		slab_allocator_free(&sa1, arr[i]);
	}

	slab_allocator_cleanup(&sa1);
	free(arr);
}

typedef struct MyObject {
	int x;
	Ptr ptr;
	OrbTreeNode node1;
	OrbTreeNode node2;
} MyObject;

SlabAllocator sa;

int my_obj_search(const OrbTreeNode *root, const OrbTreeNode *value,
				  OrbTreeNodePair *retval) {
	retval->parent = null;
	retval->is_right = true;
	const OrbTreeNode *cur = root;
	loop {
		int x1 = container_of(cur, MyObject, node1)->x;
		int x2 = container_of(value, MyObject, node1)->x;

		retval->self = orbtree_node_ptr(cur, retval->is_right);

		if (x1 == x2) {
			break;
		} else if (x1 > x2) {
			MyObject *right = orbtree_node_right(cur);
			if (right == NULL) {
				retval->parent = retval->self;
				retval->self = null;
				retval->is_right = true;
				break;
			}
			Ptr rptr = orbtree_node_ptr(&right->node1, true);
			retval->parent = orbtree_node_ptr(cur, retval->is_right);
			retval->is_right = true;
			cur = (const OrbTreeNode *)(slab_get(&sa, rptr) +
										offsetof(MyObject, node1));
		} else {
			MyObject *left = orbtree_node_right(cur);
			if (left == NULL) {
				retval->parent = retval->self;
				retval->self = null;
				retval->is_right = false;
				break;
			}
			Ptr lptr = orbtree_node_ptr(&left->node1, false);
			retval->parent = orbtree_node_ptr(cur, retval->is_right);
			retval->is_right = false;
			cur = (const OrbTreeNode *)(slab_get(&sa, lptr) +
										offsetof(MyObject, node1));
		}
	}
	return 0;
}

MyObject *create_my_object(int value) {
	Ptr ptr = slab_allocator_allocate(&sa);
	MyObject *ret = (MyObject *)slab_get(&sa, ptr);
	ret->ptr = ptr;
	ret->x = value;
	return ret;
}

void free_my_object(MyObject *obj) {
	slab_allocator_free(&sa, obj->ptr);
}

OrbTreeNodeWrapper wrapper_for(MyObject *obj) {
	OrbTreeNodeWrapper ret = {.ptr = obj->ptr, offsetof(MyObject, node1)};
	return ret;
}

OrbTreeNodeWrapper wrap_obj(int value) {
	MyObject *obj = create_my_object(value);
	return wrapper_for(obj);
}

void free_wrapper(OrbTreeNodeWrapper wrapper) {
	slab_allocator_free(&sa, wrapper.ptr);
}

MyTest(base, test_orbtree) {
	OrbTree t;
	slab_allocator_init(&sa, sizeof(MyObject), 100, 200);
	orbtree_init(&t, &sa);

	OrbTreeNodeWrapper obj1 = wrap_obj(1);
	orbtree_put(&t, &obj1, my_obj_search);

	OrbTreeNodeWrapper obj2 = wrap_obj(0);

	orbtree_put(&t, &obj2, my_obj_search);

	MyObject *obj2_out = orbtree_get(&t, &obj2, my_obj_search, 0);
	cr_assert(obj2_out);
	cr_assert_eq(obj2_out->x, 0);

	MyObject *obj_out = orbtree_get(&t, &obj1, my_obj_search, 0);
	cr_assert(obj_out);
	cr_assert_eq(obj_out->x, 1);

	free_wrapper(obj1);
	free_wrapper(obj2);

	slab_allocator_cleanup(&sa);
}
