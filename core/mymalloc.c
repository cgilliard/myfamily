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

#include <core/mymalloc.h>
#include <string.h>

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0, 0};

_Thread_local SlabAllocatorPtr TL_SLAB_ALLOCATOR = SLAB_ALLOCATOR_UNINIT;
_Thread_local SlabAllocatorPtr *local_slab_allocator;

u64 next_greater_slab_size(u64 size) {
	u64 right = sizeof(slab_sizes) / sizeof(slab_sizes[0]) - 1;
	if (size > slab_sizes[right])
		return size;
	if (size <= 64)
		return size;
	u64 left = 0;
	while (left <= right) {
		u64 mid = left + (right - left) / 2;
		if (slab_sizes[mid] >= size && slab_sizes[mid - 1] < size) {
			return slab_sizes[mid];
		}
		if (slab_sizes[mid] > size)
			right = mid - 1;
		else
			left = mid + 1;
	}
	return slab_sizes[right];
}

int mymalloc(Slab *slab, u64 size_in) {
	u64 size = next_greater_slab_size(size_in);

	// get correct slab allocator
	SlabAllocatorPtr *sa = local_slab_allocator;
	if (!sa) {
		if (!TL_SLAB_ALLOCATOR.initialized)
			init_tl_slab_allocator();
		sa = &TL_SLAB_ALLOCATOR;
	}

	int ret = 0;

	// try to allocate a slab
	u64 alloc = slab_allocator_allocate(sa, size);

	if (alloc == UINT64_MAX) {
		THREAD_LOCAL_RESOURCE_STATS.slab_misses++;
		if (sa->no_malloc) {
			ret = -1;
		} else {
			// could not allocate the slab. Try malloc.
			slab->data = malloc(size);
			if (slab->data == NULL)
				ret = -1;
			else {
				slab->id = UINT64_MAX;
				slab->len = size;
			}
		}
	} else {
		// we found a slab. Access it.
		if (slab_allocator_get(sa, slab, alloc))
			ret = -1;
	}

	if (!ret) {
		THREAD_LOCAL_RESOURCE_STATS.malloc_sum += 1;
	}

	return ret;
}

int myfree(Slab *slab) {
	// get correct slab allocator
	SlabAllocatorPtr *sa = local_slab_allocator;
	if (!sa) {
		if (!TL_SLAB_ALLOCATOR.initialized)
			init_tl_slab_allocator();
		sa = &TL_SLAB_ALLOCATOR;
	}

	int ret = 0;
	if (slab->id == UINT64_MAX) {
		// This is a malloc allocated slab so free it directly
		if (slab->data) {
			free(slab->data);
			slab->data = NULL;
		}
	} else {
		// slab allocated so try to free it with the slab allocator
		if (slab_allocator_free(sa, slab->id))
			ret = -1;
	}

	if (!ret) {
		THREAD_LOCAL_RESOURCE_STATS.malloc_sum -= 1;
	}

	return ret;
}

int myrealloc(Slab *slab, u64 size) {
	size = next_greater_slab_size(size);
	// get correct slab allocator
	SlabAllocatorPtr *sa = local_slab_allocator;
	if (!sa) {
		if (!TL_SLAB_ALLOCATOR.initialized)
			init_tl_slab_allocator();
		sa = &TL_SLAB_ALLOCATOR;
	}

	int ret = 0;

	if (slab->id == UINT64_MAX) {
		void *tmp = realloc(slab->data, size);
		if (tmp == NULL)
			ret = -1;
		else {
			slab->data = tmp;
			slab->len = size;
			THREAD_LOCAL_RESOURCE_STATS.realloc_sum += 1;
		}
	} else {
		Slab nslab;
		if (mymalloc(&nslab, size)) {
			ret = -1;
		} else {
			// allocation success, copy and dealloc original slab
			memcpy(nslab.data, slab->data, slab->len);

			myfree(slab);
			slab->len = nslab.len;
			slab->id = nslab.id;
			slab->data = nslab.data;
			THREAD_LOCAL_RESOURCE_STATS.realloc_sum += 1;
		}
	}
	return ret;
}

FILE *myfopen(const char *path, const char *mode) { return NULL; }
void myfclose(FILE *ptr) {}

ResourceStats get_resource_stats() {
	ResourceStats ret = THREAD_LOCAL_RESOURCE_STATS;
	return ret;
}
