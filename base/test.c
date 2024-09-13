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

#include <base/base.h>
#include <base/test.h>

MySuite(base);

MyTest(base, test_mymalloc)
{
	u64 begin_mymalloc_sum = mymalloc_sum();
	u64 begin_myfree_sum = myfree_sum();
	u64 begin_diff = begin_mymalloc_sum - begin_myfree_sum;
	u64 diff = mymalloc_sum - myfree_sum;
	void *tmp = mymalloc(100);
	u64 end_mymalloc_sum = mymalloc_sum();
	u64 end_myfree_sum = myfree_sum();
	u64 end_diff = end_mymalloc_sum - end_myfree_sum;
	cr_assert_neq(end_diff, begin_diff);
	myfree(tmp);
	end_mymalloc_sum = mymalloc_sum();
	end_myfree_sum = myfree_sum();
	end_diff = end_mymalloc_sum - end_myfree_sum;
	cr_assert_eq(end_diff, begin_diff);
}

MyTest(base, test_path)
{
	Path path;
	// create a path
	cr_assert(!path_for(&path, "resources/test1.txt"));
	Path path2;
	// create an invalid path
	path_for(&path2, "resources/test1.txt2");
	// we can canonicalize this
	cr_assert(!path_canonicalize(&path));
	// this, we can't
	cr_assert(path_canonicalize(&path2));

	// we find this at the end
	cr_assert(strstr(path_to_string(&path), "resources/test1.txt"));

	Path path3;
	// creata a path to a directory
	cr_assert(!path_for(&path3, "resources"));
	// push a file
	path_push(&path3, "test1.txt");
	// canonicalize works
	cr_assert(!path_canonicalize(&path3));

	// we find this at the end
	cr_assert(strstr(path_to_string(&path3), "resources/test1.txt"));
	path_push(&path3, "test");
	cr_assert(path_canonicalize(&path3));

	Path path4;
	cr_assert(!path_for(&path4, "resources/dir1/dir2/test.txt"));
	cr_assert(!path_canonicalize(&path4));
	cr_assert(!path_pop(&path4));
	cr_assert(!path_pop(&path4));
	path_push(&path4, "test2.txt");
	cr_assert(!path_canonicalize(&path4));

	// we find this at the end
	cr_assert(strstr(path_to_string(&path4), "resources/dir1/test2.txt"));

	// test with separators included
	Path path5;
	// creata a path to a directory
	cr_assert(!path_for(&path5, "resources/"));
	// push a file
	path_push(&path5, "test1.txt");
	// canonicalize works
	cr_assert(!path_canonicalize(&path5));

	// we find this at the end
	cr_assert(strstr(path_to_string(&path5), "resources/test1.txt"));

	Path path6;
	cr_assert(!path_for(&path6, "~/.fam/resources"));
	cr_assert(!path_canonicalize(&path6));
	// we find this at the end (comment out, this works but if .fam doesn't
	// exist it's an error. We will keep tests from creating directories outside
	// of the project directory.
	// cr_assert(strstr(path_to_string(&path6), ".fam/resources"));
}
