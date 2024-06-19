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

FamTest(base, test_another) {}
