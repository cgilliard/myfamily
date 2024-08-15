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

#include <core/class2.h>
#include <string.h>

Slab init_generic_slab(char **bindings[]) {
	printf("init slabs\n");
	Slab ret = {0, NULL, 0};
	return ret;
}

int compare_trait_lookup_entry(const void *ent1, const void *ent2) {
	const TraitEntry *vtent1 = ent1;
	const TraitEntry *vtent2 = ent2;
	return strcmp(vtent1->name, vtent2->name);
}

void sort_global_trait_lookup_table() {
	qsort(__global_trait_lookup_table.entries,
	      __global_trait_lookup_table.len, sizeof(TraitEntry),
	      compare_trait_lookup_entry);
}

void add_trait_entry(TraitEntry *ent) { printf("add te %s\n", ent->name); }
