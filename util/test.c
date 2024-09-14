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
#include <util/trie.h>

MySuite(util);

Test(util, test_trie)
{
	// create a match array to get the results
	TrieMatch tm[10];
	Trie t1;
	const bool p[] = { true, true };
	const char *s[] = { "test1", "test2" };

	// build the trie with these two params
	cr_assert(!trie_build(&t1, s, p, 2));
	// try to match with a single match
	int res1 = trie_match(&t1, "abctest1ok", tm, 10);

	// assert results returned correctly
	cr_assert_eq(res1, 1);
	cr_assert_eq(tm[0].pattern_id, 0);
	cr_assert_eq(tm[0].offset, 3);
	cr_assert_eq(tm[0].len, 5);

	// match with two matches
	int res2 = trie_match(&t1, "abctest1okteabctest2oktest", tm, 10);

	// assert results returned correctly
	cr_assert_eq(res2, 2);
	cr_assert_eq(tm[0].pattern_id, 0);
	cr_assert_eq(tm[0].offset, 3);
	cr_assert_eq(tm[0].len, 5);
	cr_assert_eq(tm[1].pattern_id, 1);
	cr_assert_eq(tm[1].offset, 15);
	cr_assert_eq(tm[1].len, 5);

	// verify that the 'limit' is followed
	int res3 = trie_match(&t1, "abctest1okteabctest2oktest", tm, 1);
	cr_assert_eq(res3, 1);
	cr_assert_eq(tm[0].pattern_id, 0);
	cr_assert_eq(tm[0].offset, 3);
	cr_assert_eq(tm[0].len, 5);

	Trie t2;
	const bool p2[] = { true, false, true };
	const char *s2[] = { "MATCH1", "OtHeR", "last" };

	// build the trie with new params (case match test)
	cr_assert(!trie_build(&t2, s2, p2, 3));

	int res4 = trie_match(&t2, "abctest1MATCH1bctest2oktest", tm, 10);
	cr_assert_eq(res4, 1);

	int res5 = trie_match(&t2, "abOtHeRctest1MATCH1bctest2oktest", tm, 10);
	cr_assert_eq(res5, 2);
	cr_assert_eq(tm[0].pattern_id, 0);
	cr_assert_eq(tm[0].offset, 13);
	cr_assert_eq(tm[0].len, 6);
	cr_assert_eq(tm[1].pattern_id, 1);
	cr_assert_eq(tm[1].offset, 2);
	cr_assert_eq(tm[1].len, 5);

	int res6 = trie_match(&t2, "abOtHeRctest1mATCH1bctest2oktest", tm, 10);
	cr_assert_eq(res6, 1);
	cr_assert_eq(tm[0].pattern_id, 1);
	cr_assert_eq(tm[0].offset, 2);
	cr_assert_eq(tm[0].len, 5);

	// this time the case sensitive is not a match
	int res7 = trie_match(&t2, "abOtHERctest1mATCH1bctest2oktest", tm, 10);
	cr_assert_eq(res7, 1);
}
