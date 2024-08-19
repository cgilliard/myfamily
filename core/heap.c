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

#include <core/heap.h>
#include <string.h>

int heap_init_free_list(HeapDataPtr *sd, size_t heap_count,
			size_t heap_offset) {
	int ret = 0;
	HeapDataParams p = sd->sdp;
	size_t slab_size = p.slab_size;
	for (size_t i = heap_offset; i < heap_offset + heap_count; i++) {
		size_t offset_next = i * (8 + slab_size);
		FatPtr slab;
		slab.len = 8;
		if (heap_data_access(sd, &slab, offset_next)) {
			break;
		}

		size_t next_size_t;
		if (i < (heap_offset + heap_count) - 1)
			next_size_t = i + 1;
		else
			next_size_t = UINT64_MAX;
		memcpy(slab.data, &next_size_t, sizeof(size_t));
	}
	return ret;
}

void heap_data_cleanup(HeapData *ptr) {
	for (size_t i = 0; i < ptr->count; i++) {
		free(ptr->data[i]);
	}
	if (ptr->count)
		free(ptr->data);
	ptr->count = 0;
}

int heap_data_build(HeapData *ptr, HeapDataParams sdp) {
	memcpy(&ptr->sdp, &sdp, sizeof(HeapDataParams));
	ptr->cur_slabs = 0;
	ptr->count = 0;
	if (sdp.initial_chunks) {
		ptr->data = malloc(sdp.initial_chunks * sizeof(void *));
		if (!ptr->data)
			return -1;
		for (size_t i = 0; i < sdp.initial_chunks; i++) {
			ptr->data[i] =
			    malloc((sdp.slab_size + 8) * sdp.slabs_per_resize);
			if (ptr->data[i] == NULL) {
				for (size_t j = i - 1; j >= 0; j--) {
					free(ptr->data[j]);
				}
				free(ptr->data);
				ptr->data = NULL;
				ptr->count = 0;
				return -1;
			}
		}
		ptr->cur_slabs = sdp.initial_chunks * sdp.slabs_per_resize;
		ptr->count = sdp.initial_chunks;
		heap_init_free_list(
		    ptr, sdp.slabs_per_resize * sdp.initial_chunks, 0);
	}

	return 0;
}

int heap_data_access(HeapData *ptr, FatPtr *slab, size_t offset) {
	if (offset >= ((ptr->sdp.slab_size + 8) * ptr->cur_slabs)) {
		return -1;
	}
	size_t heap_index =
	    offset / (ptr->sdp.slabs_per_resize * (ptr->sdp.slab_size + 8));
	size_t offset_mod =
	    offset % (ptr->sdp.slabs_per_resize * (ptr->sdp.slab_size + 8));
	slab->data = ptr->data[heap_index] + offset_mod;
	return 0;
}

int heap_data_resize(HeapData *ptr) {
	if (ptr->cur_slabs >= ptr->sdp.max_slabs)
		return -1;
	void *data =
	    malloc((ptr->sdp.slab_size + 8) * ptr->sdp.slabs_per_resize);
	if (data == NULL)
		return -1;

	void *tmp;
	if (ptr->count)
		tmp = realloc(ptr->data, sizeof(void *) * (ptr->count + 1));
	else
		tmp = malloc(sizeof(void *) * (ptr->count + 1));
	if (tmp == NULL) {
		free(data);
		return -1;
	}
	ptr->data = tmp;
	ptr->data[ptr->count] = data;
	ptr->count++;
	ptr->cur_slabs += ptr->sdp.slabs_per_resize;
	heap_init_free_list(ptr, ptr->sdp.slabs_per_resize,
			    ptr->cur_slabs - ptr->sdp.slabs_per_resize);

	return 0;
}

int heap_data_compare(const void *p1, const void *p2) {
	int ret = 0;

	HeapDataPtr d1 = *(HeapDataPtr *)p1;
	HeapDataPtr d2 = *(HeapDataPtr *)p2;

	if (d1.sdp.slab_size > d2.sdp.slab_size)
		ret = 1;
	else if (d1.sdp.slab_size < d2.sdp.slab_size)
		ret = -1;

	return ret;
}

int heap_allocator_build(HeapAllocator *ptr, bool zeroed, int heap_data_count,
			 ...) {
	int ret = 0;

	ptr->heap_data_arr = malloc(sizeof(HeapDataPtr) * heap_data_count);
	ptr->heap_data_arr_size = heap_data_count;
	ptr->zeroed = zeroed;
	ptr->prev = NULL;
	ptr->initialized = true;
	va_list sdptr;
	va_start(sdptr, heap_data_count);
	for (size_t i = 0; i < heap_data_count; i++) {
		HeapDataParams sdp = va_arg(sdptr, HeapDataParams);
		if (sdp.initial_chunks == 0)
			sdp.free_list_head = UINT64_MAX;
		else
			sdp.free_list_head = 0;
		heap_data_build(&ptr->heap_data_arr[i], sdp);
	}
	va_end(sdptr);

	if (ptr->heap_data_arr_size > 1) {
		qsort(ptr->heap_data_arr, ptr->heap_data_arr_size,
		      sizeof(HeapDataPtr), heap_data_compare);
	}

	return ret;
}

void heap_allocator_cleanup(HeapAllocator *ptr) {
	for (size_t i = 0; i < ptr->heap_data_arr_size; i++) {
		heap_data_cleanup(&ptr->heap_data_arr[i]);
	}
	ptr->heap_data_arr_size = 0;

	if (ptr->heap_data_arr) {
		free(ptr->heap_data_arr);
		ptr->heap_data_arr = NULL;
	}
}

// binary search for the correct slab size
int heap_allocator_index(HeapAllocator *ptr, size_t size) {
	int ret = -1;
	if (ptr->heap_data_arr_size == 0)
		return ret;
	int left = 0;
	int right = ptr->heap_data_arr_size - 1;

	while (left <= right) {
		int mid = left + (right - left) / 2;
		size_t slab_size = ptr->heap_data_arr[mid].sdp.slab_size;
		if (slab_size == size) {
			ret = mid;
			break;
		} else if (slab_size > size)
			right = mid - 1;
		else
			left = mid + 1;
	}

	return ret;
}

size_t heap_allocator_allocate(HeapAllocator *ptr, size_t size) {
	int index = heap_allocator_index(ptr, size);

	// slab size not found return UINT64_MAX.
	if (index < 0)
		return UINT64_MAX;

	size_t ret = ptr->heap_data_arr[index].sdp.free_list_head;
	if (ret == UINT64_MAX) {
		size_t cur_slabs = ptr->heap_data_arr[index].cur_slabs;
		// No more slabs, try to resize, or return UINT64_MAX
		if (heap_data_resize(&ptr->heap_data_arr[index]))
			return UINT64_MAX;
		ptr->heap_data_arr[index].sdp.free_list_head = cur_slabs;
		ret = ptr->heap_data_arr[index].sdp.free_list_head;
	}

	size_t offset_next =
	    ret * (8 + ptr->heap_data_arr[index].sdp.slab_size);
	FatPtr next;
	heap_data_access(&ptr->heap_data_arr[index], &next, offset_next);
	memcpy(&ptr->heap_data_arr[index].sdp.free_list_head, next.data, 8);

	// zero out memory if configured
	if (ptr->zeroed) {
		for (size_t i = 0; i < ptr->heap_data_arr[index].sdp.slab_size;
		     i++) {
			((char *)next.data)[i + 8] = 0;
		}
	}

	return ret | ((size_t)index << 56);
}
int heap_allocator_get(HeapAllocator *ptr, FatPtr *slab, size_t id) {
	size_t index = (id >> 56) & 0xFF;
	size_t rel = id & 0x00FFFFFFFFFFFFFF;

	if (index >= ptr->heap_data_arr_size)
		return -1;
	slab->len = ptr->heap_data_arr[index].sdp.slab_size;
	slab->id = id;
	size_t offset = rel * (8 + ptr->heap_data_arr[index].sdp.slab_size) + 8;
	heap_data_access(&ptr->heap_data_arr[index], slab, offset);

	return 0;
}
int heap_allocator_free(HeapAllocator *ptr, size_t id) {
	size_t index = (id >> 56) & 0xFF;     // Extract the index
	size_t rel = id & 0x00FFFFFFFFFFFFFF; // Extract the relative ID

	if (index >= ptr->heap_data_arr_size) {
		return -1; // Invalid index
	}

	HeapDataPtr *sd = &ptr->heap_data_arr[index];
	FatPtr slab;
	size_t offset = rel * (8 + sd->sdp.slab_size);

	// Access the slab to ensure it's a valid address
	if (heap_data_access(sd, &slab, offset) != 0) {
		return -1; // Failed to access the slab
	}

	// Update the free list
	size_t old_head = sd->sdp.free_list_head;
	memcpy(slab.data, &old_head, sizeof(size_t));
	sd->sdp.free_list_head = rel;

	// zero out memory since it's now freed
	if (ptr->zeroed) {
		size_t start_data = offset + 8;
		/*
		for (size_t i = 0; i < ptr->heap_data_arr[index].sdp.slab_size;
		     i++) {
			((char *)ptr->heap_data_arr[index]
			     .data)[i + start_data] = 0;
		}
		*/
		for (size_t i = 0; i < ptr->heap_data_arr[index].sdp.slab_size;
		     i++) {
			((char *)slab.data)[i + 8] = 0;
		}
	}

	return 0; // Success
}
