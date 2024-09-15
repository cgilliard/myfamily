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

// The suffix pair represents the index and it's corresponding suffix. This is used to calcualte the
// suffix tree efficiently.
typedef struct SuffixPair {
	u64 index; // index (where does this suffix start within the text)
	const char *suffix; // the actual suffix
} SuffixPair;

// The opaque SuffixTreeImpl
typedef struct SuffixTreeImpl {
	FatPtr suffix_array; // The array of u64 offsets
	FatPtr lcp_array; // The array of least common prefixes
	char *text; // The text for this suffix tree
	u64 suffix_array_len; // The length of the suffix array
} SuffixTreeImpl;

// Deallocate all memory associated with the suffix tree
void suffix_tree_cleanup(SuffixTree *ptr)
{
	if (ptr->impl.data) {
		SuffixTreeImpl *si = ptr->impl.data;
		if (si->suffix_array.data) {
			chain_free(&si->suffix_array);
		}
		if (si->lcp_array.data) {
			chain_free(&si->lcp_array);
		}
		if (si->text) {
			myfree(si->text);
			si->text = NULL;
		}
		chain_free(&ptr->impl);
	}
}

// Compare suffix pairs (for qsort)
int suffix_pair_compare(const void *p1, const void *p2)
{
	const SuffixPair *sp1 = p1;
	const SuffixPair *sp2 = p2;
	return strcmp(sp1->suffix, sp2->suffix);
}

// Compare suffix matches (for qsort)
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

// build a suffix tree
int suffix_tree_build(SuffixTree *ptr, const char *text)
{
	// validate input
	if (text == NULL) {
		errno = EINVAL;
		return -1;
	}
	int text_len = strlen(text);
	if (text_len == 0) {
		errno = EINVAL;
		return -1;
	}

	// allocate the needed memory for the SuffixTreeImpl
	if (chain_malloc(&ptr->impl, sizeof(SuffixTreeImpl))) {
		return -1;
	}

	// Pointer to our suffix tree impl
	SuffixTreeImpl *si = ptr->impl.data;

	// allocate the suffix array
	if (chain_malloc(&si->suffix_array, sizeof(u64) * text_len)) {
		chain_free(&ptr->impl);
		return -1;
	}

	// allocate the lcp array
	if (chain_malloc(&si->lcp_array, sizeof(u64) * text_len)) {
		chain_free(&si->suffix_array);
		chain_free(&ptr->impl);
		return -1;
	}

	// allocate the text
	si->text = mymalloc(sizeof(char) * text_len + 1);
	if (si->text == NULL) {
		chain_free(&si->suffix_array);
		chain_free(&si->lcp_array);
		chain_free(&ptr->impl);
		return -1;
	}

	// create a suffix array to build our suffixes.
	SuffixPair arr[text_len];
	u64 rank[text_len];
	u64 *lcp_arr = si->lcp_array.data;

	// initialize the values of the suffix array.
	for (u64 i = 0; i < text_len; i++) {
		arr[i].index = i;
		arr[i].suffix = (char *)(text + i);
	}

	// sort them
	qsort(arr, text_len, sizeof(SuffixPair), suffix_pair_compare);

	// initialize rank and lcp to 0
	memset(rank, 0, sizeof(u64) * text_len);
	memset(lcp_arr, 0, sizeof(u64) * text_len);

	// update the data in our array and initialize 'rank' for LCP calc.
	u64 *suffix_arr = si->suffix_array.data;
	for (u64 i = 0; i < text_len; i++) {
		suffix_arr[i] = arr[i].index;
		rank[arr[i].index] = i;
	}

	// calculate the LCP
	u64 h = 0; // Length of the common prefix
	for (u64 i = 0; i < text_len; i++) {
		u64 r = rank[i];
		if (r > 0) {
			// Previous suffix in sorted order
			u64 j = suffix_arr[r - 1];

			// Compare characters
			while (i + h < text_len && j + h < text_len && text[i + h] == text[j + h]) {
				h++;
			}
			lcp_arr[r] = h; // Set LCP value
			if (h > 0) {
				h--; // Decrease h for the next suffix
			}
		}
	}

	// copy the text and set the suffix_array_len
	strcpy(si->text, text);
	si->suffix_array_len = text_len;

	return 0;
}

// search our suffix tree for the specified pattern. Return up to limit entries in the
// SuffixTreeMatch.
int suffix_tree_search(SuffixTree *ptr, const char *pattern, SuffixTreeMatch *ret, u64 limit)
{
	// validate input
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
	if (pattern_len == 0) {
		errno = EINVAL;
		return -1;
	}

	// init other variables
	u64 *suffix_arr = si->suffix_array.data;
	u64 *lcp_arr = si->lcp_array.data;
	i32 min = 0;
	i32 max = si->suffix_array_len - 1;
	u64 match_index = UINT64_MAX;

	// do a binary search to find a match
	loop
	{
		// take midpoint
		i64 mid = min + ((max - min) / 2);
		// compare
		int cmp = strncmp(pattern, si->text + suffix_arr[mid], pattern_len);
		if (cmp < 0) {
			// it's less so max must be lower
			max = mid - 1;
		} else if (cmp > 0) {
			// it's greater so min must be higher
			min = mid + 1;
		} else {
			// match found
			match_index = mid;
			break;
		}
		// can't go any further
		if (max < min)
			break;
	}

	// We know the last_match is at least (match_index). Start there.
	u64 last_match = match_index;

	// do something like a binary search to find the last index.
	if (match_index != UINT64_MAX) {
		// max is the last entry.
		i64 max = si->suffix_array_len - 1;
		// check that there are common prefixes, if not we have a single match and we need not find
		// a range.
		if (lcp_arr[match_index] >= pattern_len) {
			// there are common prefixes that are at least the length of our pattern. Use binary
			// search to find the last one.
			loop
			{
				i64 test = last_match + ((max - last_match) / 2);
				if (strncmp(pattern, si->text + suffix_arr[test], pattern_len)) {
					// not a match, test is too high
					max = test;
				} else {
					// they match, last_match could be higher
					last_match = test;
				}
				if (last_match >= max - 1) {
					// as high as it could be, break.
					break;
				}
			}
		}
	}

	// If we have a match, iterate through up to limit times and set the ret[i].offset
	// appropriately.
	if (match_index != UINT64_MAX) {
		int count = 0;
		loop
		{
			// if count is greater than last match or count is greater than equal to limit break
			if (count > last_match || count >= limit)
				break;
			// If the pattern no longer matches, break
			if (strncmp(pattern, si->text + suffix_arr[last_match - count], pattern_len))
				break;
			// set the offset and increment count
			ret[count].offset = suffix_arr[last_match - count];
			count++;
		}
		// return count
		return count;
	} else {
		// no matches, return 0.
		return 0;
	}
}

// sort the suffix tree using qsort
void suffix_tree_sort_results(SuffixTreeMatch *ret, u64 count)
{
	qsort(ret, count, sizeof(SuffixTreeMatch), suffix_match_compare);
}
