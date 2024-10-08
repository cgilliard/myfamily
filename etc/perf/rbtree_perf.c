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

#include <assert.h>
#include <stdio.h>

#include <base/types.h>
#include <util/rbtree.h>

int u64_compare(const void *v1, const void *v2) {
	const u64 *k1 = v1;
	const u64 *k2 = v2;
	if (*k1 < *k2)
		return -1;
	else if (*k1 > *k2)
		return 1;
	return 0;
}

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

// Your RBTree, rbtree_build, rbtree_insert, rbtree_get functions here...

int main() {
	printf("Testing Red-black tree implementation\n");

	RBTree test1;
	rbtree_build(&test1, sizeof(u64), sizeof(u64), &u64_compare, false);
	u64 count = 100000;

	u64 v;

	// Measure insertion time
	struct timespec start_insert, end_insert;
	clock_gettime(CLOCK_MONOTONIC, &start_insert);
	for (u64 k = 0; k < count; k++) {
		v = k + 3;
		assert(!rbtree_insert(&test1, &k, &v)); // Insert key-value pairs
	}
	clock_gettime(CLOCK_MONOTONIC, &end_insert);
	double insertion_time_ns = (end_insert.tv_sec - start_insert.tv_sec) * 1e9 +
							   (end_insert.tv_nsec - start_insert.tv_nsec);
	printf("Time taken for insertion: %f seconds\n", insertion_time_ns / 1e9);
	printf("Average time per insert: %f nanoseconds\n", insertion_time_ns / count);
	printf("Total inserts: %llu\n", count);

	// Measure retrieval time
	struct timespec start_retrieve, end_retrieve;
	clock_gettime(CLOCK_MONOTONIC, &start_retrieve);
	for (u64 k = 0; k < count; k++) {
		const u64 *value = rbtree_get(&test1, &k);
		assert(*value == k + 3); // Retrieve values and verify
	}
	clock_gettime(CLOCK_MONOTONIC, &end_retrieve);
	double retrieval_time_ns = (end_retrieve.tv_sec - start_retrieve.tv_sec) * 1e9 +
							   (end_retrieve.tv_nsec - start_retrieve.tv_nsec);
	printf("Time taken for retrieval: %f seconds\n", retrieval_time_ns / 1e9);
	printf("Average time per retrieval: %f nanoseconds\n", retrieval_time_ns / count);
	printf("Total retrievals: %llu\n", count);

	// Measure retrieval time
	struct timespec start_delete, end_delete;
	clock_gettime(CLOCK_MONOTONIC, &start_delete);
	for (u64 k = 0; k < count; k++) {
		assert(!rbtree_delete(&test1, &k));
	}
	clock_gettime(CLOCK_MONOTONIC, &end_delete);
	double delete_time_ns = (end_delete.tv_sec - start_delete.tv_sec) * 1e9 +
							(end_delete.tv_nsec - start_delete.tv_nsec);
	printf("Time taken for delete: %f seconds\n", delete_time_ns / 1e9);
	printf("Average time per delete: %f nanoseconds\n", delete_time_ns / count);
	printf("Total deletes: %llu\n", count);

	return 0;
}
