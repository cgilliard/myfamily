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

#ifndef _UTIL_SUFFIX_TREE__
#define _UTIL_SUFFIX_TREE__

#include <base/types.h>
#include <util/suffix_tree.h>

typedef struct SuffixTreeMatch {
	u64 offset;
} SuffixTreeMatch;

typedef struct SuffixTreeImpl SuffixTreeImpl;

typedef struct SuffixTreeNc {
	FatPtr impl;
} SuffixTreeNc;

void suffix_tree_cleanup(SuffixTreeNc *ptr);
#define SuffixTree SuffixTreeNc __attribute__((warn_unused_result, cleanup(suffix_tree_cleanup)))

int suffix_tree_build(SuffixTree *ptr, const char *text);
int suffix_tree_search(SuffixTree *ptr, const char *pattern, SuffixTreeMatch *ret, u64 limit);
void suffix_tree_sort_results(SuffixTreeMatch *ret, u64 count);
int suffix_tree_longest_repeated_substring(SuffixTree *ptr, SuffixTreeMatch *ret);

#endif // _UTIL_SUFFIX_TREE__
