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
#include <errno.h>
#include <stdlib.h>

typedef struct HeapDataParams {
	HeapDataParamsConfig config;
	u32 free_list_head;
} HeapDataParams;

typedef struct HeapData {
	void **data;
	u32 *free_list;
	u32 count;
	u32 cur_slabs;
	HeapDataParams hdp;
} HeapData;

typedef struct HeapAllocatorImpl {
	u32 hd_size;
	HeapAllocatorConfig config;
	HeapData *hd_arr;
} HeapAllocatorImpl;

static u64 __malloc_count = 0;
static u64 __free_count = 0;
static bool __ret_null_alloc = false;

static inline void *do_malloc(size_t size) {
	if (__ret_null_alloc)
		return NULL;
	void *ret = malloc(size);
	// printf("malloc %zu [%p (%llu)]\n", size, ret, ++__malloc_count);
	return ret;
}

static inline void do_free(void *ptr) {
	// printf("free %p (%llu)\n", ptr, ++__free_count);
	free(ptr);
}

static inline void *do_realloc(void *ptr, size_t size) {
	if (__ret_null_alloc)
		return NULL;
	void *ret = realloc(ptr, size);
	// printf("realloc %zu [old=%p,new=%p]\n", size, ptr, ret);
	return ret;
}

void *fat_ptr_data(FatPtr *ptr) { return ptr->data; }

u64 fat_ptr_len(FatPtr *ptr) { return ptr->len; }

int heap_allocator_init_free_list(HeapData *hd, u64 index, u32 slabs,
				  bool last_is_uint_max) {
	hd->data[index] = do_malloc(hd->hdp.config.slab_size * slabs);
	if (hd->data[index] == NULL)
		return -1;
	if (index == 0)
		hd->free_list = do_malloc(sizeof(u32) * slabs);
	else {
		void *tmp = do_realloc(hd->free_list,
				       sizeof(u32) * (hd->cur_slabs + slabs));
		if (!tmp) {
			do_free(hd->data[index]);
			return -1;
		}
		hd->free_list = tmp;
	}
	u32 offset = index * hd->hdp.config.slabs_per_resize;
	for (u64 i = 0; i < slabs; i++) {
		if ((i == (slabs - 1)) && last_is_uint_max)
			hd->free_list[i + offset] = UINT32_MAX;
		else
			hd->free_list[i + offset] = offset + i + 1;
	}

	return 0;
}

int heap_data_compare(const void *p1, const void *p2) {
	int ret = 0;

	HeapData d1 = *(HeapData *)p1;
	HeapData d2 = *(HeapData *)p2;

	if (d1.hdp.config.slab_size > d2.hdp.config.slab_size)
		ret = 1;
	else if (d1.hdp.config.slab_size < d2.hdp.config.slab_size)
		ret = -1;

	return ret;
}

int heap_allocator_init_hdp(HeapAllocator *ptr, HeapDataParamsConfig *hdp,
			    u64 index) {

	int ret = 0;
	ptr->impl->hd_arr[index].hdp.config = *hdp;
	ptr->impl->hd_arr[index].hdp.free_list_head = 0;
	ptr->impl->hd_arr[index].cur_slabs = 0;
	ptr->impl->hd_arr[index].count =
	    ptr->impl->hd_arr[index].hdp.config.initial_chunks;
	if (ptr->impl->hd_arr[index].hdp.config.initial_chunks) {
		ptr->impl->hd_arr[index].data = do_malloc(
		    ptr->impl->hd_arr[index].hdp.config.initial_chunks *
		    sizeof(void *));
		if (ptr->impl->hd_arr[index].data == NULL)
			return -1;
		ptr->impl->hd_arr[index].cur_slabs =
		    ptr->impl->hd_arr[index].hdp.config.initial_chunks *
		    ptr->impl->hd_arr[index].hdp.config.slabs_per_resize;
		bool last_is_uint_max = false;
		for (u64 i = 0;
		     i < ptr->impl->hd_arr[index].hdp.config.initial_chunks;
		     i++) {

			if (i ==
			    ptr->impl->hd_arr[index].hdp.config.initial_chunks -
				1)
				last_is_uint_max = true;
			if (heap_allocator_init_free_list(
				&ptr->impl->hd_arr[index], i,
				ptr->impl->hd_arr[index]
				    .hdp.config.slabs_per_resize,
				last_is_uint_max))
				ret = -1;
		}
	} else
		ptr->impl->hd_arr[index].data = NULL;
	return ret;
}

int heap_allocator_build(HeapAllocator *ptr, HeapAllocatorConfig *config,
			 int heap_data_params_count, ...) {
	// check inputs
	if (ptr == NULL || config == NULL) {
		errno = EINVAL;
		return -1;
	}

	// allocate the HeapAllocatorImpl
	ptr->impl = do_malloc(sizeof(HeapAllocatorImpl));
	if (ptr->impl == NULL)
		return -1;

	// copy the config
	ptr->impl->config = *config;

	// allocate heap data array
	ptr->impl->hd_arr =
	    do_malloc(sizeof(HeapData) * heap_data_params_count);
	if (ptr->impl->hd_arr == NULL) {
		heap_allocator_cleanup(ptr);
		return -1;
	}

	// iterate through specified heap data params
	va_list hdps;
	va_start(hdps, heap_data_params_count);
	for (u64 i = 0; i < heap_data_params_count; i++) {
		HeapDataParamsConfig hdp = va_arg(hdps, HeapDataParamsConfig);
		if (heap_allocator_init_hdp(ptr, &hdp, i)) {
			heap_allocator_cleanup(ptr);
			return -1;
		}
	}

	if (heap_data_params_count) {
		qsort(ptr->impl->hd_arr, heap_data_params_count,
		      sizeof(HeapData), heap_data_compare);
	}

	ptr->impl->hd_size = heap_data_params_count;

	va_end(hdps);

	return 0;
}

// binary search for the correct slab size
int heap_allocator_index(HeapAllocator *ptr, u64 size) {
	int ret = -1;
	if (ptr->impl->hd_size == 0)
		return ret;

	int left = 0;
	int right = ptr->impl->hd_size - 1;

	while (left <= right) {
		int mid = left + (right - left) / 2;
		u64 slab_size = ptr->impl->hd_arr[mid].hdp.config.slab_size;
		if (slab_size == size) {
			ret = mid;
			break;
		} else if (slab_size > size)
			right = mid - 1;
		else
			left = mid + 1;
	}

	if (ret == -1 && right + 1 <= ptr->impl->hd_size - 1) {
		return right + 1;
	}

	return ret;
}

int heap_data_resize(u64 index, HeapData *hd) {
	if (hd->cur_slabs < hd->hdp.config.max_slabs) {
		u32 nslabs_count =
		    hd->hdp.config.slabs_per_resize + hd->cur_slabs;
		if (nslabs_count > hd->hdp.config.max_slabs)
			nslabs_count = hd->hdp.config.max_slabs;

		u32 slabs_to_alloc = nslabs_count - hd->cur_slabs;
		void *tmp;
		if (hd->data)
			tmp = do_realloc(hd->data,
					 (hd->count + 1) * sizeof(void *));
		else
			tmp = do_malloc((hd->count + 1) * sizeof(void **));

		if (tmp == NULL)
			return -1;
		hd->data = tmp;

		if (heap_allocator_init_free_list(hd, hd->count, slabs_to_alloc,
						  true))
			return -1;
		hd->hdp.free_list_head = hd->cur_slabs;
		hd->cur_slabs = nslabs_count;
		hd->count += 1;
		return 0;
	}
	return -1;
}

int heap_data_allocate(u64 index, HeapData *hd, FatPtr *fptr) {
	if (hd->cur_slabs == 0) {
		// this hd initially had 0 slabs
		// resize it
		if (heap_data_resize(index, hd))
			return -1;
	}

	if (hd->hdp.free_list_head == UINT32_MAX)
		return -1;
	u64 id = hd->hdp.free_list_head;
	hd->hdp.free_list_head = hd->free_list[id];
	fptr->id = id | (index << 56);
	fptr->len = hd->hdp.config.slab_size;

	u64 heap_data_index = id / hd->hdp.config.slabs_per_resize;
	u64 offset_mod = id % hd->hdp.config.slabs_per_resize;

	fptr->data =
	    hd->data[heap_data_index] + offset_mod * hd->hdp.config.slab_size;

	return 0;
}

int heap_data_free(u64 index, HeapData *hd, FatPtr *fptr) {
	u64 rel = fptr->id & 0x00FFFFFFFFFFFFFF; // Extract the relative ID

	if (rel >= hd->cur_slabs)
		return -1;

	u64 head = hd->hdp.free_list_head;
	hd->hdp.free_list_head = rel;
	hd->free_list[rel] = head;

	return 0;
}

int heap_allocator_allocate(HeapAllocator *ptr, u64 size, FatPtr *fptr) {
	int ret = -1;
	int index = heap_allocator_index(ptr, size);

	if (index < 0) {
		if (!ptr->impl->config.no_malloc) {
			fptr->data = do_malloc(size);
			if (fptr->data == NULL) {
				fptr->len = 0;
				ret = -1;
			} else {
				fptr->len = size;
				fptr->id = UINT64_MAX;
				ret = 0;
			}
		}
	} else {
		HeapData *hd = &ptr->impl->hd_arr[index];
		ret = heap_data_allocate(index, hd, fptr);
		if (ret) {
			// there are no more slabs. Try to resize
			if (!heap_data_resize(index, hd)) {
				// successful resize, allocate should always
				// succeed here
				ret = heap_data_allocate(index, hd, fptr);
			} else if (!ptr->impl->config.no_malloc) {
				// could not allocate, so we fall back to malloc
				// if configured
				fptr->data = do_malloc(size);
				if (fptr->data == NULL) {
					fptr->len = 0;
					ret = -1;
				} else {
					fptr->len = size;
					fptr->id = UINT64_MAX;
					ret = 0;
				}
			}
		}
	}

	return ret;
}

int heap_allocator_free(HeapAllocator *ptr, FatPtr *fptr) {
	if (fptr->id == UINT64_MAX) {
		// malloc allocated

		if (fptr->data) {
			do_free(fptr->data);
			fptr->data = NULL;
		}

		return 0;
	}

	u64 index = (fptr->id >> 56) & 0xFF; // Extract the index

	if (index >= ptr->impl->hd_size) {
		errno = EINVAL;
		return -1; // Invalid index
	}

	HeapData *hd = &ptr->impl->hd_arr[index];
	return heap_data_free(index, hd, fptr);
}

int heap_allocator_cleanup(HeapAllocator *ptr) {
	// check for impl and deallocate
	if (ptr->impl) {
		for (u32 i = 0; i < ptr->impl->hd_size; i++) {
			if (ptr->impl->hd_arr[i].count) {
				// check that it's not an unallocated heap data
				do_free(ptr->impl->hd_arr[i].free_list);
				for (u64 j = 0; j < ptr->impl->hd_arr[i].count;
				     j++) {
					do_free(ptr->impl->hd_arr[i].data[j]);
				}
				do_free(ptr->impl->hd_arr[i].data);
			}
		}

		do_free(ptr->impl->hd_arr);
		do_free(ptr->impl);
		ptr->impl = NULL;
	}
	return 0;
}
