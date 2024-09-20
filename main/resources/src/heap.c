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
#include <string.h>

// internal representation of the HeapDataParamsConfig with the required
// free_list_head value.
typedef struct HeapDataParams {
	HeapDataParamsConfig config;
	u32 free_list_head; // The next slab that is free
} HeapDataParams;

// The data associated with each slab_size.
typedef struct HeapData {
	void **data; // pointers to each chunk of data
	u32 *free_list; // the pointers for free slabs.
	u32 count; // the number of chunks currently allocated
	u32 cur_slabs; // the number of slabs currently allocated
	u32 cur_slabs_allocated; // the number of slabs currently allocated;
	HeapDataParams hdp; // The heap data params for this HeapData
} HeapData;

// The opaque pointer type which is stored in the HeapAllocator data structure
// which is used to access a HeapAllocator.
typedef struct HeapAllocatorImpl {
	u32 hd_size; // number of sizes available
	HeapAllocatorConfig config; // the configuration
	HeapData *hd_arr; // The array of heap data.
	u32 cur_malloc_slabs_allocated; // the number of slabs currently allocated directly via malloc
} HeapAllocatorImpl;

// debugging options/counters
u64 __malloc_count = 0;
u64 __free_count = 0;
bool __debug_build_allocator_malloc_fail1 = false;
bool __debug_build_allocator_malloc_fail2 = false;
bool __debug_build_allocator_malloc_fail3 = false;
bool __debug_build_allocator_malloc_fail4 = false;
bool __debug_build_allocator_malloc_fail5 = false;
bool __debug_build_allocator_malloc_fail6 = false;
bool __debug_build_allocator_malloc_fail7 = false;
bool __debug_build_allocator_malloc_fail8 = false;

void *do_malloc(size_t size)
{
	__malloc_count += 1;
	void *ret = malloc(size);
	// printf("malloc %zu [%p (%llu)]\n", size, ret, __malloc_count);
	return ret;
}

void do_free(void *ptr)
{
	__free_count += 1;
	// printf("free %p (%llu)\n", ptr, __free_count);
	free(ptr);
}

void *do_realloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);
	// printf("realloc %zu [old=%p,new=%p]\n", size, ptr, ret);
	return ret;
}

void *fat_ptr_data(const FatPtr *ptr)
{
	return ptr->data;
}

u64 fat_ptr_len(const FatPtr *ptr)
{
	return ptr->len;
}

u64 fat_ptr_id(const FatPtr *ptr)
{
	return ptr->id;
}

int heap_allocator_init_free_list(HeapData *hd, u64 index, u32 slabs, bool last_is_uint_max)
{
	hd->data[index] = NULL; // set to NULL for cleanup consistency

	// allocate memory for slabs
	if (!__debug_build_allocator_malloc_fail4)
		hd->data[index] = do_malloc(hd->hdp.config.slab_size * slabs);
	if (hd->data[index] == NULL)
		return -1;

	void *tmp = NULL;

	// allocate / or reallocate the freelist
	if (!__debug_build_allocator_malloc_fail5) {
		if (index == 0)
			tmp = do_malloc(sizeof(u32) * slabs);
		else {
			tmp = do_realloc(hd->free_list, sizeof(u32) * (hd->cur_slabs + slabs));
		}
	}

	if (!tmp) {
		do_free(hd->data[index]);
		return -1;
	}

	hd->free_list = tmp;

	// initialize the values of the free list.
	u32 offset = index * hd->hdp.config.slabs_per_resize;
	for (u64 i = 0; i < slabs; i++) {
		if ((i == (slabs - 1)) && last_is_uint_max)
			hd->free_list[i + offset] = UINT32_MAX;
		else
			hd->free_list[i + offset] = offset + i + 1;
	}

	return 0;
}

// compare function used for sorting.
int heap_data_compare(const void *p1, const void *p2)
{
	int ret = 0;

	HeapData d1 = *(HeapData *)p1;
	HeapData d2 = *(HeapData *)p2;

	if (d1.hdp.config.slab_size > d2.hdp.config.slab_size)
		ret = 1;
	else if (d1.hdp.config.slab_size < d2.hdp.config.slab_size)
		ret = -1;

	return ret;
}

// initialize the heap data params
int heap_allocator_init_hdp(HeapAllocator *ptr, HeapDataParamsConfig *hdp, u64 index)
{

	// set some value in case of a failure below such that cleanup can
	// succeed
	int ret = 0;
	ptr->impl->hd_arr[index].hdp.config = *hdp;
	ptr->impl->hd_arr[index].hdp.free_list_head = 0;
	ptr->impl->hd_arr[index].cur_slabs = 0;
	ptr->impl->hd_arr[index].cur_slabs_allocated = 0;
	ptr->impl->hd_arr[index].count = ptr->impl->hd_arr[index].hdp.config.initial_chunks;

	// if we have initial chunks, initialize them
	if (ptr->impl->hd_arr[index].hdp.config.initial_chunks) {
		ptr->impl->hd_arr[index].data = NULL;

		// try to allocate space for the data for this size
		if (!__debug_build_allocator_malloc_fail8)
			ptr->impl->hd_arr[index].data
				= do_malloc(ptr->impl->hd_arr[index].hdp.config.initial_chunks * sizeof(void *));

		// if NULL return error
		if (ptr->impl->hd_arr[index].data == NULL)
			return -1;

		// set cur_slabs value
		ptr->impl->hd_arr[index].cur_slabs = ptr->impl->hd_arr[index].hdp.config.initial_chunks
			* ptr->impl->hd_arr[index].hdp.config.slabs_per_resize;

		// initialize the initial_chunks of the free list
		bool last_is_uint_max = false;
		ptr->impl->hd_arr[index].free_list = NULL;
		for (u64 i = 0; i < ptr->impl->hd_arr[index].hdp.config.initial_chunks; i++) {

			if (i == ptr->impl->hd_arr[index].hdp.config.initial_chunks - 1)
				last_is_uint_max = true;
			if (heap_allocator_init_free_list(&ptr->impl->hd_arr[index], i,
					ptr->impl->hd_arr[index].hdp.config.slabs_per_resize, last_is_uint_max))
				ret = -1;
		}
	} else
		ptr->impl->hd_arr[index].data = NULL;
	return ret;
}

int heap_allocator_build_arr(HeapAllocator *ptr, HeapAllocatorConfig *config,
	HeapDataParamsConfig arr[], u64 heap_data_params_count)
{
	// check inputs
	if (ptr == NULL || config == NULL || heap_data_params_count >= 256) {
		errno = EINVAL;
		return -1;
	}

	// allocate the HeapAllocatorImpl
	if (!__debug_build_allocator_malloc_fail1)
		ptr->impl = do_malloc(sizeof(HeapAllocatorImpl));
	if (ptr->impl == NULL || __debug_build_allocator_malloc_fail1)
		return -1;

	ptr->impl->cur_malloc_slabs_allocated = 0;
	ptr->impl->hd_size = 0;

	// copy the config
	ptr->impl->config = *config;

	// allocate heap data array
	ptr->impl->hd_arr = NULL;
	if (!__debug_build_allocator_malloc_fail2)
		ptr->impl->hd_arr = do_malloc(sizeof(HeapData) * heap_data_params_count);
	if (ptr->impl->hd_arr == NULL || __debug_build_allocator_malloc_fail2) {
		heap_allocator_cleanup(ptr);
		return -1;
	}
	ptr->impl->hd_size = heap_data_params_count;

	// iterate through specified heap data params
	for (u64 i = 0; i < heap_data_params_count; i++) {
		HeapDataParamsConfig hdp = arr[i];
		ptr->impl->hd_arr[i].count = 0; // init to 0 for safe cleanup
		ptr->impl->hd_arr[i].data = NULL;
		if ((__debug_build_allocator_malloc_fail3 && i > 0)
			|| heap_allocator_init_hdp(ptr, &hdp, i)) {
			ptr->impl->hd_size = i; // update for cleanup, others
									// did not get allocated
			if (ptr->impl->hd_arr[i].data) {
				do_free(ptr->impl->hd_arr[i].data);
			}
			heap_allocator_cleanup(ptr);
			return -1;
		}
	}

	if (heap_data_params_count) {
		qsort(ptr->impl->hd_arr, heap_data_params_count, sizeof(HeapData), heap_data_compare);
	}

	// check invalid configurations
	u64 last_size = 0;
	for (u64 i = 0; i < heap_data_params_count; i++) {
		if (ptr->impl->hd_arr[i].hdp.config.slab_size == 0) {
			// no 0 sized slabs allowed
			errno = EINVAL;
			heap_allocator_cleanup(ptr);
			return -1;
		}
		// note: logic correct because array is sorted.
		if (ptr->impl->hd_arr[i].hdp.config.slab_size == last_size) {
			// Duplicate slab_size
			errno = EEXIST;
			heap_allocator_cleanup(ptr);
			return -1;
		}
		last_size = ptr->impl->hd_arr[i].hdp.config.slab_size;
	}

	return 0;
}

int heap_allocator_build(
	HeapAllocator *ptr, HeapAllocatorConfig *config, int heap_data_params_count, ...)
{
	int arr_size = heap_data_params_count;

	// 0 size is allowed, so we update to address sanitizer warning.
	if (arr_size <= 0) {
		arr_size = 1;
	}
	HeapDataParamsConfig arr[arr_size];
	va_list hdps;
	va_start(hdps, heap_data_params_count);
	// build the array to pass to heap_allocator_build_arr
	for (u64 i = 0; i < heap_data_params_count; i++) {
		HeapDataParamsConfig hdp = va_arg(hdps, HeapDataParamsConfig);
		arr[i] = hdp;
	}

	va_end(hdps);
	// call the array version of the function
	return heap_allocator_build_arr(ptr, config, arr, heap_data_params_count);
}

// binary search for the correct slab size
int heap_allocator_index(HeapAllocator *ptr, u64 size)
{
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

int heap_data_resize(u64 index, HeapData *hd)
{
	// check that we can resize this hdp
	if (hd->cur_slabs < hd->hdp.config.max_slabs) {
		// calculate the new slabs
		u32 nslabs_count = hd->hdp.config.slabs_per_resize + hd->cur_slabs;
		if (nslabs_count > hd->hdp.config.max_slabs)
			nslabs_count = hd->hdp.config.max_slabs;

		// determine slabs to allocate
		u32 slabs_to_alloc = nslabs_count - hd->cur_slabs;

		// attempt to allocate
		void *tmp = NULL;
		if (!__debug_build_allocator_malloc_fail7) {
			if (hd->data)
				tmp = do_realloc(hd->data, (hd->count + 1) * sizeof(void *));
			else
				tmp = do_malloc((hd->count + 1) * sizeof(void **));
		}

		if (tmp == NULL)
			return -1;
		hd->data = tmp;

		// initialize the new part of the free list
		if (heap_allocator_init_free_list(hd, hd->count, slabs_to_alloc, true))
			return -1;
		hd->hdp.free_list_head = hd->cur_slabs;
		hd->cur_slabs = nslabs_count;
		hd->count += 1;
		return 0;
	}
	return -1;
}

int heap_data_allocate(u64 index, HeapData *hd, FatPtr *fptr)
{
	if (hd->cur_slabs == 0) {
		// this hd initially had 0 slabs
		// resize it
		if (heap_data_resize(index, hd))
			return -1;
	}

	// no more slabs, return error
	if (hd->hdp.free_list_head == UINT32_MAX)
		return -1;

	// get next free slab
	u64 id = hd->hdp.free_list_head;

	// update free list head
	hd->hdp.free_list_head = hd->free_list[id];

	// update with the specific index (differing slab sizes)
	fptr->id = id | (index << 56);
	fptr->len = hd->hdp.config.slab_size;

	u64 heap_data_index = id / hd->hdp.config.slabs_per_resize;
	u64 offset_mod = id % hd->hdp.config.slabs_per_resize;

	// set the data of the fptr
	fptr->data = hd->data[heap_data_index] + offset_mod * hd->hdp.config.slab_size;
	hd->cur_slabs_allocated++;

	return 0;
}

// free data in this HeapData
int heap_data_free(u64 index, HeapData *hd, FatPtr *fptr)
{
	u64 rel = fptr->id & 0x00FFFFFFFFFFFFFF; // Extract the relative ID

	// if this is invalid return an error
	if (rel >= hd->cur_slabs)
		return -1;

	// update head and rest of the list
	u64 head = hd->hdp.free_list_head;
	hd->hdp.free_list_head = rel;
	hd->free_list[rel] = head;

	hd->cur_slabs_allocated--;

	return 0;
}

u64 heap_allocator_cur_slabs_allocated(HeapAllocator *ptr)
{
	u64 ret = ptr->impl->cur_malloc_slabs_allocated;
	for (int i = 0; i < ptr->impl->hd_size; i++) {
		ret += ptr->impl->hd_arr[i].cur_slabs_allocated;
	}
	return ret;
}

// main allocation function
int heap_allocator_allocate(HeapAllocator *ptr, u64 size, FatPtr *fptr)
{
	int ret = -1;
	// determine the index via binary search
	int index = heap_allocator_index(ptr, size);

	// if this index is not found (too big) fall back to malloc
	if (index < 0) {
		if (!ptr->impl->config.no_malloc) {
			fptr->data = NULL;
			if (!__debug_build_allocator_malloc_fail6) {
				ptr->impl->cur_malloc_slabs_allocated++;
				fptr->data = do_malloc(size);
			}
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
				fptr->data = NULL;
				if (!__debug_build_allocator_malloc_fail6)
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

	// if zeroed is configured set all memory of allocated slabs to 0.
	if (!ret && ptr->impl->config.zeroed) {
		memset(fptr->data, 0, fptr->len);
	}

	return ret;
}

int heap_allocator_free(HeapAllocator *ptr, FatPtr *fptr)
{
	int ret = 0;
	if (fptr->id == UINT64_MAX) {
		// malloc allocated

		if (fptr->data) {
			ptr->impl->cur_malloc_slabs_allocated--;
			do_free(fptr->data);
			fptr->data = NULL;
		}

		ret = 0;
	} else {

		u64 index = (fptr->id >> 56) & 0xFF; // Extract the index

		if (index >= ptr->impl->hd_size) {
			errno = EINVAL;
			ret = -1; // invalid index
		} else {
			HeapData *hd = &ptr->impl->hd_arr[index];
			ret = heap_data_free(index, hd, fptr);
		}
	}

	if (!ret && ptr->impl->config.zeroed) {
		memset(fptr->data, 0, fptr->len);
	}

	return ret;
}

int heap_allocator_cleanup(HeapAllocator *ptr)
{
	// check for impl and deallocate
	if (ptr->impl) {
		for (u32 i = 0; i < ptr->impl->hd_size; i++) {
			if (ptr->impl->hd_arr[i].count) {
				// check that it's not an unallocated heap data
				if (ptr->impl->hd_arr[i].free_list) {
					do_free(ptr->impl->hd_arr[i].free_list);
					ptr->impl->hd_arr[i].free_list = NULL;
				}
				for (u64 j = 0; j < ptr->impl->hd_arr[i].count; j++) {
					do_free(ptr->impl->hd_arr[i].data[j]);
				}
				if (ptr->impl->hd_arr[i].data) {
					do_free(ptr->impl->hd_arr[i].data);
					ptr->impl->hd_arr[i].data = NULL;
				}
			}
		}

		if (ptr->impl->hd_arr) {
			do_free(ptr->impl->hd_arr);
			ptr->impl->hd_arr = NULL;
		}
		do_free(ptr->impl);
		ptr->impl = NULL;
	}
	return 0;
}
