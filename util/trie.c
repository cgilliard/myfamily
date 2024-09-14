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

#include <base/chain_allocator.h>
#include <string.h>
#include <util/trie.h>

typedef struct TrieNode {
	u32 children[256];
	u32 pattern_id;
} TrieNode;

typedef struct TrieImpl {
	FatPtr memory_block;
	TrieNode *root;
	u32 cur;
} TrieImpl;

void trie_cleanup(TrieNc *ptr)
{
	if (ptr->impl.data) {
		TrieImpl *ti = ptr->impl.data;
		if (ti)
			chain_free(&ti->memory_block);
		chain_free(&ptr->impl);
	}
}

int trie_init_node(TrieNode *ptr)
{
	ptr->pattern_id = UINT32_MAX;
	for (int i = 0; i < 256; i++)
		ptr->children[i] = UINT32_MAX;
	return 0;
}

int trie_insert(Trie *ptr, const char *str, bool is_case_sensitive, int pattern_id)
{
	TrieImpl *ti = ptr->impl.data;
	int slen = strlen(str);
	TrieNode *itt = ti->root;
	for (int i = 0; i < slen; i++) {
		char next = str[i];
		if (!is_case_sensitive) {
			if (next >= 'A' && next <= 'Z')
				next += 32;
		}
		u32 offset = itt->children[next];
		if (offset == UINT32_MAX) {
			// new node needed
			itt->children[next] = ti->cur;
			trie_init_node((TrieNode *)(ti->memory_block.data + ti->cur * sizeof(TrieNode)));
			offset = ti->cur;
			ti->cur++;

		} else {
			// node exists follow it
		}
		itt = (TrieNode *)(ti->memory_block.data + offset * sizeof(TrieNode));
	}

	itt->pattern_id = pattern_id;

	return 0;
}

int trie_build(Trie *ptr, const char *search_strings[], const bool is_case_sensitive[], int count)
{
	int m = 1; // 1 for root
	for (int i = 0; i < count; i++) {
		m += strlen(search_strings[i]);
	}
	if (chain_malloc(&ptr->impl, sizeof(TrieImpl)))
		return -1;

	TrieImpl *ti = ptr->impl.data;
	ti->cur = 1;

	if (chain_malloc(&ti->memory_block, m * sizeof(TrieNode))) {
		chain_free(&ptr->impl);
		return -1;
	}
	ti->root = ti->memory_block.data;
	trie_init_node(ti->root);

	for (int i = 0; i < count; i++) {
		if (trie_insert(ptr, search_strings[i], is_case_sensitive[i], i)) {
			trie_cleanup(ptr);
			return -1;
		}
	}

	return 0;
}
int trie_match(Trie *ptr, const char *text, TrieMatch ret[], u64 limit)
{
	TrieMatch ci_matches[limit];
	int ci_match_count = 0;
	int match_count = 0;
	int tlen = strlen(text);
	TrieImpl *ti = ptr->impl.data;
	TrieNode *itt = ti->root;
	int match_len = 0;
	for (int i = 0; i < tlen; i++) {
		char next = text[i];
		u32 offset = itt->children[next];
		if (offset != UINT32_MAX) {
			// found the node
			itt = (TrieNode *)(ti->memory_block.data + offset * sizeof(TrieNode));
			if (itt->pattern_id != UINT32_MAX) {
				ret[match_count].pattern_id = itt->pattern_id;
				ret[match_count].offset = i - match_len;
				ret[match_count].len = match_len + 1;
				ci_matches[ci_match_count].pattern_id = itt->pattern_id;
				ci_matches[ci_match_count].offset = i - match_len;
				ci_match_count++;
				match_count++;
			}
			match_len++;
			if (match_count == limit)
				break;
		} else {
			// not found, return to root
			itt = ti->root;
			match_len = 0;
		}
	}

	if (match_count < limit) {
		for (int i = 0; i < tlen; i++) {
			char next = text[i];
			if (next >= 'A' && next <= 'Z')
				next += 32;
			u32 offset = itt->children[next];
			if (offset != UINT32_MAX) {
				// found the node
				itt = (TrieNode *)(ti->memory_block.data + offset * sizeof(TrieNode));
				if (itt->pattern_id != UINT32_MAX) {

					bool duplicate = false;
					for (int j = 0; j < ci_match_count; j++) {
						if (ci_matches[j].pattern_id == itt->pattern_id
							&& ci_matches[j].offset == i - match_len) {
							duplicate = true;
						}
					}

					if (!duplicate) {
						ret[match_count].pattern_id = itt->pattern_id;
						ret[match_count].offset = i - match_len;
						ret[match_count].len = match_len + 1;
						match_count++;
					}
				}
				match_len++;
				if (match_count == limit)
					break;
			} else {
				// not found, return to root
				itt = ti->root;
				match_len = 0;
			}
		}
	}
	return match_count;
}
