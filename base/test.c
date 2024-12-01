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

#include <base/lib.h>
#include <base/macro_util.h>
#include <base/test.h>

// for malloc test
void *malloc(size_t);
void free(void *);

Suite(base);

Object test_fun1(int y) {
	return $(y + 10);
}

Object test_th1(Channel channel) {
	return $(0);
}

Object test_th2(Channel channel) {
	return $(0);
}

Object test_th3(Channel channel) {
	return $(0);
}

Object test_init(Channel channel) {
	run(test_th1);
	run(test_th2);
	run(test_th3);
	return $(0);
}

Test(fam_init) {
	let x = init(test_init, 4);
	assert(1);
}

Test(object) {
	var obj = $(1);
	long long v = *(long long *)value_of(&obj);
	assert_eq(v, 1);
	assert_eq($int(obj), 1);

	$int(obj)++;
	assert_eq($int(obj), 2);

	let obj2 = $(-1LL);
	long long v2 = *(long long *)value_of(&obj2);
	assert_eq(v2, -1LL);
	assert_eq($int(obj2), -1);

	double d = 1.4;
	let obj3 = object_float(d);
	double v3 = *(double *)value_of(&obj3);
	assert_eq(v3, 1.4);
	assert_eq($float(obj3), 1.4);

	float f = 1.5;
	let obj4 = object_float(f);
	double v4 = *(double *)value_of(&obj4);
	assert_eq(v4, 1.5);
	assert_eq($float(obj4), 1.5);

	let obj5 = $(1.6);
	double v5 = *(double *)value_of(&obj5);
	assert_eq(v5, 1.6);
	assert_eq($float(obj5), 1.6);

	let obj6 = $(1234ULL);
	unsigned long long v6 = *(unsigned long long *)value_of(&obj6);
	assert_eq(v6, 1234);
	assert_eq($uint(obj6), 1234);

	let obj7 = $(test_fun1);
	const ObjectNc (*v7_fn)(int) = $fn(&obj7);
	Object v7 = v7_fn(3);
	assert_eq($int(v7), 13);
}

Test(test_lock) {
	Lock l1 = INIT_LOCK;
	lock_read(&l1);
	// upper bits incremented by 1.
	assert_eq(l1 >> 32, 1);
	// lower bits also incremeneted by 1.
	assert_eq(l1 & 0xFFFFFFFF, 1);

	lock_unlock(&l1);
	// upper bits incremented by 1.
	assert_eq(l1 >> 32, 2);
	// lower bits also decremented by 1.
	assert_eq(l1 & 0xFFFFFFFF, 0);

	lock_write(&l1);
	// upper bits incremented by 2.
	assert_eq(l1 >> 32, 4);
	// lower bits high bit set
	assert_eq(l1 & 0xFFFFFFFF, 0x80000000);

	lock_unlock(&l1);

	// upper bits incremented by 1.
	assert_eq(l1 >> 32, 5);
	// lower bits are set to 0 now
	assert_eq(l1 & 0xFFFFFFFF, 0x00000000);
}

Test(util) {
	char arr1[10];
	for (int i = 0; i < 10; i++) arr1[i] = 2;
	set_bytes(arr1, '\0', 10);
	for (int i = 0; i < 10; i++) assert_eq(arr1[i], 0);

	char s1[10];
	copy_bytes(s1, "abcdefghi", 9);
	reverse(s1, cstring_len(s1));

	char s2[10];
	int len = cstring_itoau64(123, s2, 10, 9);
	s2[len] = 0;

	assert(!cstring_compare("123", s2));

	len = cstring_itoau64(0, s2, 10, 9);
	s2[len] = 0;
	assert(!cstring_compare("0", s2));

	char s3[10];
	len = cstring_itoai64(-100, s3, 10, 9);
	assert_eq(len, 4);
	s3[len] = 0;
	assert(!cstring_compare("-100", s3));

	char s4[10];
	len = cstring_itoai64(40, s4, 10, 9);
	assert_eq(len, 2);
	s4[len] = 0;
	assert(!cstring_compare("40", s4));
}

Test(bitmap) {
	BitMap bm1;
	void *addr = map(1);
	let bm = bitmap_init(&bm1, 1, addr);
	void *addr2 = map(1);
	let x = bitmap_extend(&bm1, addr2);
	int size = 10;

	for (unsigned long long i = 0; i < size; i++) {
		Object obj = bitmap_allocate(&bm1);
		assert_eq(object_type(&obj), UInt);
		assert_eq($uint(obj), i);
	}

	for (unsigned long long i = 0; i < size; i++) {
		Object obj = $(i);
		bitmap_free(&bm1, obj);
	}

	unmap(addr, 1);
}

int count = 1000;
int size = 500;
int alloc_size = 64 - SLAB_LIST_SIZE;

Test(slab_allocator) {
	SlabAllocator sa1;
	Object res = slab_allocator_init(&sa1, alloc_size, size + 5, size + 5);
	assert(object_type(&res) != Err);
	unsigned char **arr = map(1 + (size * sizeof(unsigned char *)) / PAGE_SIZE);

	for (long long i = 0; i < count; i++) {
		for (long long j = 0; j < size; j++) {
			arr[j] = slab_allocator_allocate(&sa1);
			for (int k = 0; k < alloc_size; k++)
				arr[j][k] = ((i + j + k) % 26) + 'a';
		}

		for (long long j = 0; j < size; j++) {
			for (int k = 0; k < alloc_size; k++) {
				assert_eq(arr[j][k], ((i + j + k) % 26) + 'a');
			}
			slab_allocator_free(&sa1, arr[j]);
		}
	}

	slab_allocator_cleanup(&sa1);
	unmap(arr, 1 + (size * sizeof(unsigned char *)) / PAGE_SIZE);
}

Test(malloc) {
	unsigned char **arr = map(1 + (size * sizeof(unsigned char *)) / PAGE_SIZE);

	for (long long i = 0; i < count; i++) {
		for (long long j = 0; j < size; j++) {
			arr[j] = malloc(alloc_size);
			for (int k = 0; k < alloc_size; k++)
				arr[j][k] = ((i + j + k) % 26) + 'a';
		}

		for (long long j = 0; j < size; j++) {
			for (int k = 0; k < alloc_size; k++) {
				assert_eq(arr[j][k], ((i + j + k) % 26) + 'a');
			}
			free(arr[j]);
		}
	}

	unmap(arr, 1 + (size * sizeof(unsigned char *)) / PAGE_SIZE);
}

Test(default_type) {
	// types default to unsigned int (uint64) (negative value doesn't work the
	// same due to the way they are represented)
	let x = 123ULL;
	assert_eq(object_type(&x), UInt);
	assert_eq($uint(x), 123);

	let y = $("test");
	assert_eq(object_type(&y), Err);
}

Test(print_util) {
	/*
		let obj = $(123);
		println("test 0={} 1={} 2={} 3={} 4={} 5={} 6={} ok done", 1, -20LL,
	   30ULL, "ok", 1.2, obj, $(1.5)); println("test");
	*/
}

#pragma GCC diagnostic ignored "-Woverflow"
#pragma clang diagnostic ignored "-Winteger-overflow"

Test(limits) {
	assert_eq(INT64_MIN - 1, INT64_MAX);
	assert_eq(INT64_MAX + 1, INT64_MIN);
	assert_eq(INT32_MIN - 1, INT32_MAX);
	assert_eq(INT32_MAX + 1, INT32_MIN);
}

Test(match) {
	let m1 = $(1);
	let r1 = match(m1,
				   (Int,
					{
						/*println("int"); */
						$(10);
					}),
				   (UInt,
					{
						println("uint");
						$(20);
					}),
				   (Err,
					{
						println("err");
						$(30);
					}),
				   (Function,
					{
						println("function");
						$(40);
					}),
				   (Float,
					{
						println("float");
						$(50);
					}),
				   (Box,
					{
						println("box");
						$(60);
					})

	);
	assert_eq($int(r1), 10);

	let m2 = $(0ULL);
	let r2 = match(m2,
				   (Int,
					{
						println("int");
						$(10);
					}),
				   (UInt,
					{
						/*println("uint");*/
						$(20);
					}),
				   (Err,
					{
						println("err");
						$(30);
					}),
				   ({
					   println("def");
					   $(40);
				   })

	);
	assert_eq($int(r2), 20);
	let m3 = $(1.5);
	let r3 = match(m3,
				   (Int,
					{
						println("int");
						$(10);
					}),
				   (UInt,
					{
						println("uint");
						$(20);
					}),
				   (Err,
					{
						println("err");
						$(30);
					}),
				   ({
					   /*println("def");*/
					   $(40);
				   })

	);
	assert_eq($int(r3), 40);
}

Test(test_box) {
	{
		let x = box(10);
		match(x, (Box, { $(0); }), ({ $(0); }));
		assert(box_get_long_bytes(&x) == 0);
		assert(box_get_short_bytes(&x) != 0);
		assert_eq(box_get_page_count(&x), 0);

		char *ptr = box_get_short_bytes(&x);
		for (int i = 0; i < OBJ_BOX_USER_DATA_SIZE; i++) {
			ptr[i] = 'a' + (i % 26);
		}

		for (int i = 0; i < OBJ_BOX_USER_DATA_SIZE; i++) {
			assert_eq(ptr[i], 'a' + (i % 26));
		}
	}

	{
		let x = box(300);
		assert(box_get_long_bytes(&x) != 0);
		assert(box_get_short_bytes(&x) != 0);
		assert_eq(box_get_page_count(&x), 1);
		char *ptr = box_get_long_bytes(&x);
		for (int i = 0; i < PAGE_SIZE; i++) {
			ptr[i] = 'a' + (i % 26);
		}
		for (int i = 0; i < PAGE_SIZE; i++) {
			assert_eq(ptr[i], 'a' + (i % 26));
		}
	}
}

Object my_fun(Object x) {
	if ($int(x) > 100) return Err(CapacityExceeded);
	return $($int(x) * 3);
}

Object my_fun2(Object x) {
	if ($int(x) < 50) return Err(IllegalArgument);
	let v = my_fun(x);
	return match(v, (Err, v), (Int, $($int(v) + 5)), (Err(IllegalState)));
}

Test(fn_calls) {
	let x1 = my_fun2($(50));
	assert_eq($int(x1), 155);

	let x2 = my_fun2($(40));
	assert($is_err(x2));
	assert_eq($kind(x2), IllegalArgument);

	let x3 = my_fun2($(200));
	assert($is_err(x3));
	assert_eq($kind(x3), CapacityExceeded);
}

Test(box_resize) {
	var box1 = box(OBJ_BOX_USER_DATA_SIZE + PAGE_SIZE);
	assert_eq(box_get_page_count(&box1), 1);

	unsigned char *shortb = box_get_short_bytes(&box1);
	for (int i = 0; i < OBJ_BOX_USER_DATA_SIZE; i++) shortb[i] = 'a' + (i % 26);

	unsigned char *longb = box_get_long_bytes(&box1);
	for (int i = 0; i < PAGE_SIZE; i++)
		longb[i] = 'a' + ((i + OBJ_BOX_USER_DATA_SIZE) % 26);

	let res = box_resize(&box1, OBJ_BOX_USER_DATA_SIZE + PAGE_SIZE * 2);
	assert(!$is_err(res));
	assert_eq(box_get_page_count(&box1), 2);

	shortb = box_get_short_bytes(&box1);
	for (int i = 0; i < OBJ_BOX_USER_DATA_SIZE; i++) {
		assert_eq(shortb[i], 'a' + (i % 26));
	}

	longb = box_get_long_bytes(&box1);
	for (int i = 0; i < PAGE_SIZE; i++)
		assert_eq(longb[i], 'a' + ((i + OBJ_BOX_USER_DATA_SIZE) % 26));
	for (int i = PAGE_SIZE; i < PAGE_SIZE * 2; i++)
		longb[i] = 'a' + ((i + OBJ_BOX_USER_DATA_SIZE) % 26);

	for (int i = 0; i < PAGE_SIZE * 2; i++) {
		assert_eq(longb[i], 'a' + ((i + OBJ_BOX_USER_DATA_SIZE) % 26));
	}

	let res2 = box_resize(&box1, 10);
	assert(!$is_err(res2));
	assert_eq(box_get_page_count(&box1), 0);
	assert(!box_get_long_bytes(&box1));
}

Test(properties) {
	var b1 = box(20000);
	let res0 = object_get_property(&b1, "test");
	assert($is_err(res0));
	let ins1 = $(123);
	let res1 = object_set_property(&b1, "test", &ins1);
	assert(!$is_err(res1));
	let res2 = object_get_property(&b1, "test");
	assert(!$is_err(res2));
	assert_eq($int(res2), 123);

	let ins2 = $(1234);
	let res3 = object_set_property(&b1, "test2", &ins2);
	let res4 = object_get_property(&b1, "test2");
	assert(!$is_err(res4));
	assert_eq($int(res4), 1234);

	let b2 = box(40000);
	let res5 = object_set_property(&b1, "test3", &b2);
	assert(!$is_err(res5));
}

static int drop_count = 0;

void my_drop(Object *obj) {
	char *data = box_get_short_bytes(obj);
	assert_eq(data[0], 'g');
	drop_count++;
}

Test(object_ref) {
	{
		var b1 = box(100);
		char *data = box_get_short_bytes(&b1);
		data[0] = 'g';
		let fn = $(my_drop);
		let res1 = object_set_property(&b1, "drop", &fn);
		assert(!$is_err(res1));
		let b2 = object_ref(&b1);
		assert_eq(drop_count, 0);
	}
	assert_eq(drop_count, 1);
}

static int drop_props_count = 0;
static int zdrops = 0;
static int ydrops = 0;
static int xdrops = 0;

void drop_props(Object *obj) {
	drop_props_count++;
	char *bytes = box_get_short_bytes(obj);
	if (bytes[0] == 'z') zdrops++;
	if (bytes[0] == 'y') ydrops++;
	if (bytes[0] == 'x') xdrops++;
}

Test(object_overwrite) {
	{
		let x = box(3);
		char *xbytes = box_get_short_bytes(&x);
		xbytes[0] = 'x';
		let y = box(2);
		char *ybytes = box_get_short_bytes(&y);
		ybytes[0] = 'y';
		var z = box(1);
		char *zbytes = box_get_short_bytes(&z);
		zbytes[0] = 'z';
		let r1 = set(z, "v", x);
		let r01 = get(z, "v");
		assert_eq(((char *)box_get_short_bytes(&r01))[0], 'x');
		let r2 = set(z, "v", y);
		let r02 = get(z, "v");
		assert_eq(((char *)box_get_short_bytes(&r02))[0], 'y');
		let fn = $(drop_props);
		let r3 = set(x, "drop", fn);
		let r4 = set(y, "drop", fn);
		let r5 = set(z, "drop", fn);
		assert_eq(drop_props_count, 0);
	}
	assert_eq(xdrops, 1);
	assert_eq(ydrops, 1);
	assert_eq(zdrops, 1);
	assert_eq(drop_props_count, 3);
	let xvv = $(1);
}

Test(object_delete) {
	var x = box(1);
	let y = $(10);
	let z = $(2);
	let r1 = set(x, "a", y);
	let y_out = get(x, "a");
	assert_eq($int(y_out), 10);
	let rem_out = remove(x, "a");
	assert_eq($int(rem_out), 10);
	let y_out2 = get(x, "a");
	assert($is_err(y_out2));
}
