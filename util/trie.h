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

#ifndef _UTIL_TRIE__
#define _UTIL_TRIE__

#include <base/types.h>

typedef struct TrieMatch {
	u32 offset;
	u32 len;
	u32 pattern_id;
} TrieMatch;

typedef struct TrieImpl TrieImpl;

typedef struct TrieNc {
	FatPtr impl;
} TrieNc;

void trie_cleanup(TrieNc *ptr);

#define Trie TrieNc __attribute__((warn_unused_result, cleanup(trie_cleanup)))

int trie_build(Trie *ptr, const char *search_strings[], const bool is_case_sensitive[], int count);
int trie_match(Trie *ptr, const char *text, TrieMatch ret[], u64 limit);
void trie_sort(TrieMatch ret[], u64 count);

#endif // _UTIL_TRIE__
