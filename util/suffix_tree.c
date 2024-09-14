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
#include <errno.h>
#include <string.h>
#include <util/suffix_tree.h>

typedef struct SuffixPair {
	u64 index;
	const char *suffix;
} SuffixPair;

typedef struct SuffixTreeImpl {
	FatPtr suffix_array;
	char *text;
	u64 suffix_array_len;
} SuffixTreeImpl;

void suffix_tree_cleanup(SuffixTree *ptr)
{
	if (ptr->impl.data) {
		SuffixTreeImpl *si = ptr->impl.data;
		if (si->suffix_array.data) {
			chain_free(&si->suffix_array);
		}
		if (si->text) {
			myfree(si->text);
			si->text = NULL;
		}
		chain_free(&ptr->impl);
	}
}

int suffix_pair_compare(const void *p1, const void *p2)
{
	const SuffixPair *sp1 = p1;
	const SuffixPair *sp2 = p2;
	return strcmp(sp1->suffix, sp2->suffix);
}

int suffix_match_compare(const void *s1, const void *s2)
{
	const SuffixTreeMatch *sm1 = s1;
	const SuffixTreeMatch *sm2 = s2;
	if (sm1->offset < sm2->offset)
		return -1;
	else if (sm1->offset > sm2->offset)
		return 1;
	return 0;
}

int suffix_tree_build(SuffixTree *ptr, const char *text)
{
	if (text == NULL) {
		errno = EINVAL;
		return -1;
	}
	int text_len = strlen(text);
	if (text_len == 0) {
		errno = EINVAL;
		return -1;
	}

	if (chain_malloc(&ptr->impl, sizeof(SuffixTreeImpl))) {
		return -1;
	}

	SuffixTreeImpl *si = ptr->impl.data;

	if (chain_malloc(&si->suffix_array, sizeof(u64) * text_len)) {
		chain_free(&ptr->impl);
		return -1;
	}

	si->text = mymalloc(sizeof(char) * text_len + 1);
	if (si->text == NULL) {
		chain_free(&si->suffix_array);
		chain_free(&ptr->impl);
		return -1;
	}

	SuffixPair arr[text_len];

	for (u64 i = 0; i < text_len; i++) {
		arr[i].index = i;
		arr[i].suffix = (char *)(text + i);
	}

	qsort(arr, text_len, sizeof(SuffixPair), suffix_pair_compare);

	u64 *suffix_arr = si->suffix_array.data;
	for (u64 i = 0; i < text_len; i++) {
		suffix_arr[i] = arr[i].index;
	}

	strcpy(si->text, text);
	si->suffix_array_len = text_len;

	return 0;
}
int suffix_tree_search(SuffixTree *ptr, const char *pattern, SuffixTreeMatch *ret, u64 limit)
{
	if (pattern == NULL) {
		errno = EINVAL;
		return -1;
	}

	SuffixTreeImpl *si = ptr->impl.data;
	if (si->suffix_array_len == 0) {
		errno = EINVAL;
		return -1;
	}

	u64 pattern_len = strlen(pattern);
	char buf[pattern_len + 1];
	u64 *suffix_arr = si->suffix_array.data;
	i32 min = 0;
	i32 max = si->suffix_array_len;

	u64 match_offset = UINT64_MAX;
	u64 match_index = UINT64_MAX;
	loop
	{
		i64 mid = min + ((max - min) / 2);
		int cmp = strncmp(pattern, si->text + suffix_arr[mid], pattern_len);
		if (cmp < 0) {
			max = mid - 1;
		} else if (cmp > 0) {
			min = mid + 1;
		} else {
			// match found
			match_offset = suffix_arr[mid];
			match_index = mid;
			break;
		}
		if (max < min)
			break;
	}

	// u64 first_match = match_index;
	u64 last_match = match_index;

	if (match_offset != UINT64_MAX) {
		// TODO: binary search back to last match
		loop
		{
			if (last_match >= si->suffix_array_len)
				break;
			if (strncmp(pattern, si->text + suffix_arr[last_match + 1], pattern_len))
				break;
			last_match++;
		}
	}

	if (match_offset != UINT64_MAX) {
		int count = 0;
		loop
		{
			if (count > last_match || count >= limit)
				break;
			if (strncmp(pattern, si->text + suffix_arr[last_match - count], pattern_len))
				break;
			ret[count].offset = suffix_arr[last_match - count];
			count++;
		}
		return count;
	} else {
		return 0;
	}
}

void suffix_tree_sort_results(SuffixTreeMatch *ret, u64 count)
{
	qsort(ret, count, sizeof(SuffixTreeMatch), suffix_match_compare);
}

int suffix_tree_longest_repeated_substring(SuffixTree *ptr, SuffixTreeMatch *ret)
{
	return 0;
}
