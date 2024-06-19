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
#include <base/error.h>
#include <base/rc.h>
#include <base/string.h>
#include <base/test.h>

FamSuite(base);

FamTest(base, test_tmalloc) {
	void *ptr = tlmalloc(1);
	tlfree(ptr);
}

FamTest(base, test_cleanup) { // assert(true);
}

int xyz(int x) { return x + 10; }
int abc(int x) { return x + 20; }

FamTest(base, test_vtable_sort) {
	Vtable table = {0, 0, NULL};
	VtableEntry ent0 = {"zzz", xyz};
	VtableEntry ent1 = {"myent1", xyz};
	VtableEntry ent2 = {"next", xyz};
	VtableEntry ent3 = {"prev", xyz};
	VtableEntry ent4 = {"aaa", abc};
	vtable_add_entry(&table, ent0);
	vtable_add_entry(&table, ent1);
	vtable_add_entry(&table, ent2);
	vtable_add_entry(&table, ent3);
	vtable_add_entry(&table, ent4);

	assert_eq_str(table.entries[0].name, "aaa");
	assert_eq_str(table.entries[4].name, "zzz");

	Object obj;
	obj.vdata.vtable = &table;
	assert(implements(&obj, "zzz"));
	assert(!implements(&obj, "yyy"));
	assert(implements(&obj, "aaa"));
	assert(implements(&obj, "next"));
	assert(implements(&obj, "myent1"));
	assert(implements(&obj, "prev"));

	int (*prev_fn)(int x) = find_fn(&obj, "prev");
	assert_eq(prev_fn(7), 17);
	int (*abc_fn)(int x) = find_fn(&obj, "aaa");
	assert_eq(abc_fn(8), 28);

	vtable_cleanup(&table);
}

FamTest(base, test_error) {
	ErrorKind ILLEGAL_STATE = EKIND("IllegalState");
	ErrorKind ILLEGAL_ARGUMENT = EKIND("IllegalArgument");
	Error err1 = ERROR(ILLEGAL_STATE, "illegal state message %i", 3);
	Error err2 = ERROR(ILLEGAL_ARGUMENT, "ill arg");
	Error err3 = ERROR(ILLEGAL_STATE, "another illegal state err");
	assert(equal(KIND(err1), KIND(err1)));
	assert(!equal(KIND(err1), KIND(err2)));
	assert(equal(KIND(err1), KIND(err3)));
	assert(!equal(&err1, &err2));
	assert(equal(&err1, &err3));
}

static int rc_test_cleanup_count = 0;
CLASS(RcTest, FIELD(u32, x))
#define RcTest DEFINE_CLASS(RcTest)

void RcTest_cleanup(RcTestPtr *ptr) { rc_test_cleanup_count += 1; }
RcTest *test_build_rctest(u32 x) {
	RcTestPtr *ret = tlmalloc(sizeof(RcTest));
	ret->vdata.vtable = &RcTestVtable;
	ret->_x = x;
}

FamTest(base, test_rc) {
	{
		RcTestPtr *x = test_build_rctest(10);
		Rc rc = RC(x);
		assert_eq(rc_test_cleanup_count, 0);

		{
			Rc rc2;
			clone(&rc2, &rc);
		}

		// cleanup did not occur here because while rc2's cleanup was
		// called, the counter was not equal to 0 so RcTest's cleanup
		// was not called.
		assert_eq(rc_test_cleanup_count, 0);
	}
	assert_eq(rc_test_cleanup_count, 1);
}

FamTest(base, test_string) {
	StringPtr *s1 = STRINGPTR("this is a test");
	Result r1 = Ok(*s1);
	StringPtr *s2 = unwrap(&r1);
	assert(equal(s1, s2));
}
