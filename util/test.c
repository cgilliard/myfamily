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

#include <base/misc.h>
#include <base/test.h>
#include <util/replace.h>
#include <util/suffix_tree.h>
#include <util/trie.h>
#include <util/vec.h>

MySuite(util);

MyTest(util, test_trie)
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
	cr_assert_eq(tm[1].pattern_id, 0);
	cr_assert_eq(tm[1].offset, 13);
	cr_assert_eq(tm[1].len, 6);
	cr_assert_eq(tm[0].pattern_id, 1);
	cr_assert_eq(tm[0].offset, 2);
	cr_assert_eq(tm[0].len, 5);

	int res6 = trie_match(&t2, "abOtHeRctest1mATCH1bctest2oktest", tm, 10);
	cr_assert_eq(res6, 1);
	cr_assert_eq(tm[0].pattern_id, 1);
	cr_assert_eq(tm[0].offset, 2);
	cr_assert_eq(tm[0].len, 5);

	// this time the case sensitive is not a match
	int res7 = trie_match(&t2, "abOtHERctest1mATCH1bctest2oktest", tm, 10);
	cr_assert_eq(res7, 1);
}

MyTest(util, test_replace)
{
	Path pin;
	Path pout;
	path_for(&pin, "./resources/test_replace.txt");
	path_for(&pout, path_to_string(test_dir));
	path_push(&pout, "test_replace.txt.out");
	const char *patterns_in[] = { "REPLACE1", "REPLACE2", "REPLACE3" };
	const bool is_case_sensitive[] = { true, true, true };
	const char *replace[] = { "mytext1", "mytext2", "mytext3" };
	replace_file(&pin, &pout, patterns_in, is_case_sensitive, replace, 3);

	FILE *file = fopen(path_to_string(&pout), "rb");
	// Determine the file size
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char buffer[file_size + 1];
	buffer[file_size] = 0;
	cr_assert_eq(read_all(buffer, file_size, sizeof(char), file), file_size);
	cr_assert(!strcmp(buffer, "x'mytext3' ok yes 'mytext2'\nanother 'mytext1'\n"));
}

MyTest(util, test_suffix_tree)
{
	// init a SuffixTreeMatch array
	SuffixTreeMatch matches[10];
	// declare a SuffixTree
	SuffixTree t1;
	// text for testing search
	const char *text = "the sixth sheik's sixth sheep's sick";
	u64 text_len = strlen(text);

	// search pattern
	const char *pattern = "s";
	u64 pattern_len = strlen(pattern);
	// build tree
	suffix_tree_build(&t1, text);
	// search
	int r1 = suffix_tree_search(&t1, pattern, matches, 10);
	// sort results
	suffix_tree_sort_results(matches, r1);

	// seven 's's in this string
	cr_assert_eq(r1, 7);
	cr_assert_eq(matches[0].offset, 4);
	cr_assert_eq(matches[1].offset, 10);
	cr_assert_eq(matches[2].offset, 16);
	cr_assert_eq(matches[3].offset, 18);
	cr_assert_eq(matches[4].offset, 24);
	cr_assert_eq(matches[5].offset, 30);
	cr_assert_eq(matches[6].offset, 32);

	// display them in a color coded manner
	printf("r1=%i\n", r1);
	for (int i = 0; i < r1; i++) {
		printf("match[%i]=%" PRIu64 "\n", i, matches[i].offset);
	}
	u64 itt = 0;
	printf("'");
	for (int x = 0; x < r1; x++) {
		u64 index = matches[x].offset;
		printf("%s", DIMMED);
		for (u64 i = itt; i < index; i++) {
			printf("%c", text[i]);
		}
		printf("%s", RESET);

		for (u64 i = index; i < pattern_len + index; i++) {
			printf("%s", GREEN);
			printf("%c", text[i]);
			printf("%s", RESET);
		}
		itt = pattern_len + index;
	}
	printf("%s", DIMMED);
	for (u64 i = itt; i < text_len; i++)
		printf("%c", text[i]);
	printf("%s", RESET);
	printf("'\n");

	// search for 'sh'
	cr_assert_eq(suffix_tree_search(&t1, "sh", matches, 10), 2);
	// sort results.
	suffix_tree_sort_results(matches, 2);
	// find the first occurance of 'sh' using other methods.
	char *off = strstr(text, "sh");
	// assert it's equal to our first match
	cr_assert_eq((u64)off - (u64)text, matches[0].offset);
	// find the second occurance of 'sh' using other methods.
	char *off2 = rstrstr(text, "sh");
	// assert it's equal to our second match
	cr_assert_eq((u64)off2 - (u64)text, matches[1].offset);

	// search something with no matches
	cr_assert(!suffix_tree_search(&t1, "blah", matches, 10));
}

MyTest(util, test_vec)
{
	Vec v1;
	vec_init(&v1, 5, sizeof(int));
	cr_assert_eq(vec_capacity(&v1), 5);
	cr_assert_eq(vec_size(&v1), 0);
	for (int i = 0; i < 10; i++) {
		int value = i + 10;
		vec_push(&v1, &value);
	}
	for (int i = 0; i < 10; i++) {
		int *value = vec_element_at(&v1, i);
		cr_assert_eq(*value, i + 10);
	}
	cr_assert_eq(vec_capacity(&v1), 15);
	cr_assert_eq(vec_size(&v1), 10);
	int *popped = vec_pop(&v1);
	cr_assert_eq(*popped, 19);
	cr_assert_eq(vec_capacity(&v1), 15);
	cr_assert_eq(vec_size(&v1), 9);

	vec_remove(&v1, 4);
	cr_assert_eq(vec_capacity(&v1), 15);
	cr_assert_eq(vec_size(&v1), 8);
	int j = 0;
	for (int i = 0; i < 8; i++) {
		if (i == 4)
			j++;
		int *value = vec_element_at(&v1, i);
		cr_assert_eq(*value, j + 10);
		j++;
	}
}
