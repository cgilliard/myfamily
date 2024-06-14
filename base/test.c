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

#include <base/class.h>
#include <base/traits.h>
#include <base/unit.h>
#include <criterion/criterion.h>
#include <stdio.h>

int default_favorite_number2(void *obj) { return 2; }

int favorite_number3(void *obj) {
	int (*do_favorite_number3)(Object *) =
	    find_fn((Object *)obj, "favorite_number3");

	if (do_favorite_number3 == NULL)
		panic("do_favorite_number3 not implemented for this type");

	return do_favorite_number3(obj);
}

int favorite_number2(void *obj) {
	int (*do_favorite_number2)(Object *) =
	    find_fn((Object *)obj, "favorite_number2");

	if (do_favorite_number2 == NULL)
		panic("do_favorite_number2 not implemented for this type");

	return do_favorite_number2(obj);
}

int favorite_number(void *obj) {
	int (*do_favorite_number)(Object *) =
	    find_fn((Object *)obj, "favorite_number");

	if (do_favorite_number == NULL)
		panic("do_favorite_number not implemented for this type");
	return do_favorite_number(obj);
}

#define TRAIT_FAVORITE_NUMBER3(T)                                              \
	TRAIT_REQUIRED(T, int, favorite_number3, T##Ptr *obj)

#define TRAIT_FAVORITE_NUMBER(T)                                               \
	TRAIT_REQUIRED(T, int, favorite_number, T##Ptr *obj)                   \
	TRAIT_IMPL(T, favorite_number2, default_favorite_number2)              \
	TRAIT_SUPER(T, TRAIT_FAVORITE_NUMBER3)

CLASS(testself, FIELD(u32, x) FIELD(u64, y))
IMPL(testself, TRAIT_SIZE)
IMPL(testself, TRAIT_FAVORITE_NUMBER);
#define testself DEFINE_CLASS(testself)
GETTER(testself, y)
SETTER(testself, y)
GETTER(testself, x)
SETTER(testself, x)

int testself_favorite_number(testself *self) { return 3; }

int testself_favorite_number3(testself *self) { return 300; }

void testself_cleanup(testselfPtr *ptr) {
	printf("cleanup.x=%i,x.y=%i\n", *testself_get_x(ptr),
	       *testself_get_y(ptr));
}
size_t testself_size(testselfPtr *ptr) { return sizeof(testselfPtr); }

CLASS(testself2, FIELD(u32, x) FIELD(u64, y) FIELD(u64, z))
IMPL(testself2, TRAIT_SIZE)
IMPL(testself2, TRAIT_FAVORITE_NUMBER);
#define testself2 DEFINE_CLASS(testself2)
int testself2_favorite_number2(testself2 *self);
OVERRIDE(testself2, favorite_number2, testself2_favorite_number2);
GETTER(testself2, y)
SETTER(testself2, y)
GETTER(testself2, x)
SETTER(testself2, x)

int testself2_favorite_number3(testself2 *self) { return 400; }

int testself2_favorite_number2(testself2 *self) { return 100; }

int testself2_favorite_number(testself2 *self) { return 4; }

void testself2_cleanup(testself2Ptr *ptr) {
	printf("cleanup2.x=%i,x.y=%i\n", *testself2_get_x(ptr),
	       *testself2_get_y(ptr));
}
size_t testself2_size(testself2Ptr *ptr) { return sizeof(testself2Ptr); }

Test(base, test_class) {
	testself ts = BUILD(testself, 10, 20);
	printf("ts.x=%u,ts.y=%llu\n", *testself_get_x(&ts),
	       *testself_get_y(&ts));
	cr_assert_eq(*testself_get_y(&ts), 20);
	cr_assert_eq(*testself_get_x(&ts), 10);

	testself_set_y(&ts, 200);
	testself_set_x(&ts, 100);

	cr_assert_eq(*testself_get_y(&ts), 200);
	cr_assert_eq(*testself_get_x(&ts), 100);

	cr_assert_eq(size(&ts), sizeof(testself));

	cr_assert_eq(favorite_number2(&ts), 2);
	cr_assert_eq(favorite_number(&ts), 3);
	cr_assert_eq(favorite_number3(&ts), 300);

	testself2 ts2 = BUILD(testself2, 11, 22);
	cr_assert_eq(*testself2_get_y(&ts2), 22);
	cr_assert_eq(*testself2_get_x(&ts2), 11);

	cr_assert_eq(size(&ts2), sizeof(testself2));
	cr_assert_neq(size(&ts), size(&ts2));

	cr_assert_eq(favorite_number(&ts2), 4);
	cr_assert_eq(favorite_number2(&ts2), 100);
	cr_assert_eq(favorite_number3(&ts2), 400);
}
