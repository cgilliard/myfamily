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

#include <core/slabs.h>
#include <string.h>

int slab_init_free_list(SlabDataPtr *sd, u64 slab_count, u64 slab_offset) {
	int ret = 0;
	SlabDataParams p = sd->sdp;
	u64 slab_size = p.slab_size;
	for (u64 i = slab_offset; i < slab_offset + slab_count; i++) {
		u64 offset_next = i * (8 + slab_size);
		Slab slab;
		slab.len = 8;
		if (slab_data_access(sd, &slab, offset_next)) {
			break;
		}

		u64 next_u64;
		if (i < (slab_offset + slab_count) - 1)
			next_u64 = i + 1;
		else
			next_u64 = UINT64_MAX;
		memcpy(slab.data, &next_u64, sizeof(u64));
	}
	return ret;
}

void slab_data_cleanup(SlabData *ptr) {
	for (u64 i = 0; i < ptr->count; i++) {
		free(ptr->data[i]);
	}
	if (ptr->count)
		free(ptr->data);
	ptr->count = 0;
}

int slab_data_build(SlabData *ptr, SlabDataParams sdp) {
	memcpy(&ptr->sdp, &sdp, sizeof(SlabDataParams));
	ptr->cur_slabs = 0;
	ptr->count = 0;
	if (sdp.initial_chunks) {
		ptr->data = malloc(sdp.initial_chunks * sizeof(void *));
		if (!ptr->data)
			return -1;
		for (u64 i = 0; i < sdp.initial_chunks; i++) {
			ptr->data[i] =
			    malloc((sdp.slab_size + 8) * sdp.slabs_per_resize);
			if (ptr->data[i] == NULL) {
				for (u64 j = i - 1; j >= 0; j--) {
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
		slab_init_free_list(
		    ptr, sdp.slabs_per_resize * sdp.initial_chunks, 0);
	}

	return 0;
}

int slab_data_access(SlabData *ptr, Slab *slab, u64 offset) {
	if (offset >= ((ptr->sdp.slab_size + 8) * ptr->cur_slabs)) {
		return -1;
	}
	u64 slab_index =
	    offset / (ptr->sdp.slabs_per_resize * (ptr->sdp.slab_size + 8));
	u64 offset_mod =
	    offset % (ptr->sdp.slabs_per_resize * (ptr->sdp.slab_size + 8));
	slab->data = ptr->data[slab_index] + offset_mod;
	return 0;
}

int slab_data_resize(SlabData *ptr) {
	if (ptr->cur_slabs >= ptr->sdp.max_slabs)
		return -1;
	void *data =
	    malloc((ptr->sdp.slab_size + 8) * ptr->sdp.slabs_per_resize);
	if (data == NULL)
		return -1;

	void *tmp = realloc(ptr->data, sizeof(void *) * (ptr->count + 1));
	if (tmp == NULL) {
		free(data);
		return -1;
	}
	ptr->data = tmp;
	ptr->data[ptr->count] = data;
	ptr->count++;
	ptr->cur_slabs += ptr->sdp.slabs_per_resize;
	slab_init_free_list(ptr, ptr->sdp.slabs_per_resize,
			    ptr->cur_slabs - ptr->sdp.slabs_per_resize);

	return 0;
}

int slab_data_compare(const void *p1, const void *p2) {
	int ret = 0;

	SlabDataPtr d1 = *(SlabDataPtr *)p1;
	SlabDataPtr d2 = *(SlabDataPtr *)p2;

	if (d1.sdp.slab_size > d2.sdp.slab_size)
		ret = 1;
	else if (d1.sdp.slab_size < d2.sdp.slab_size)
		ret = -1;

	return ret;
}

int slab_allocator_build(SlabAllocator *ptr, bool zeroed, int slab_data_count,
			 ...) {
	int ret = 0;

	ptr->slab_data_arr = malloc(sizeof(SlabDataPtr) * slab_data_count);
	ptr->slab_data_arr_size = slab_data_count;
	ptr->zeroed = zeroed;
	ptr->prev = NULL;
	ptr->initialized = true;
	va_list sdptr;
	va_start(sdptr, slab_data_count);
	for (u64 i = 0; i < slab_data_count; i++) {
		SlabDataParams sdp = va_arg(sdptr, SlabDataParams);
		if (sdp.initial_chunks == 0)
			sdp.free_list_head = UINT64_MAX;
		else
			sdp.free_list_head = 0;
		slab_data_build(&ptr->slab_data_arr[i], sdp);
	}
	va_end(sdptr);

	if (ptr->slab_data_arr_size > 1) {
		qsort(ptr->slab_data_arr, ptr->slab_data_arr_size,
		      sizeof(SlabDataPtr), slab_data_compare);
	}

	return ret;
}

void slab_allocator_cleanup(SlabAllocator *ptr) {
	for (u64 i = 0; i < ptr->slab_data_arr_size; i++) {
		slab_data_cleanup(&ptr->slab_data_arr[i]);
	}
	ptr->slab_data_arr_size = 0;

	if (ptr->slab_data_arr) {
		free(ptr->slab_data_arr);
		ptr->slab_data_arr = NULL;
	}
}

// binary search for the correct slab size
int slab_allocator_index(SlabAllocator *ptr, u64 size) {
	int ret = -1;
	if (ptr->slab_data_arr_size == 0)
		return ret;
	int left = 0;
	int right = ptr->slab_data_arr_size - 1;

	while (left <= right) {
		int mid = left + (right - left) / 2;
		u64 slab_size = ptr->slab_data_arr[mid].sdp.slab_size;
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

u64 slab_allocator_allocate(SlabAllocator *ptr, u64 size) {
	int index = slab_allocator_index(ptr, size);

	// slab size not found return UINT64_MAX.
	if (index < 0)
		return UINT64_MAX;

	u64 ret = ptr->slab_data_arr[index].sdp.free_list_head;
	if (ret == UINT64_MAX) {
		u64 cur_slabs = ptr->slab_data_arr[index].cur_slabs;
		// No more slabs, try to resize, or return UINT64_MAX
		if (slab_data_resize(&ptr->slab_data_arr[index]))
			return UINT64_MAX;
		ptr->slab_data_arr[index].sdp.free_list_head = cur_slabs;
		ret = ptr->slab_data_arr[index].sdp.free_list_head;
	}

	u64 offset_next = ret * (8 + ptr->slab_data_arr[index].sdp.slab_size);
	Slab next;
	slab_data_access(&ptr->slab_data_arr[index], &next, offset_next);
	memcpy(&ptr->slab_data_arr[index].sdp.free_list_head, next.data, 8);

	// zero out memory if configured
	if (ptr->zeroed) {
		for (u64 i = 0; i < ptr->slab_data_arr[index].sdp.slab_size;
		     i++) {
			((char *)next.data)[i + 8] = 0;
		}
	}

	return ret | ((u64)index << 56);
}
int slab_allocator_get(SlabAllocator *ptr, Slab *slab, u64 id) {
	u64 index = (id >> 56) & 0xFF;
	u64 rel = id & 0x00FFFFFFFFFFFFFF;

	if (index >= ptr->slab_data_arr_size)
		return -1;
	slab->len = ptr->slab_data_arr[index].sdp.slab_size;
	slab->id = id;
	u64 offset = rel * (8 + ptr->slab_data_arr[index].sdp.slab_size) + 8;
	slab_data_access(&ptr->slab_data_arr[index], slab, offset);

	return 0;
}
int slab_allocator_free(SlabAllocator *ptr, u64 id) {
	u64 index = (id >> 56) & 0xFF;	   // Extract the index
	u64 rel = id & 0x00FFFFFFFFFFFFFF; // Extract the relative ID

	if (index >= ptr->slab_data_arr_size) {
		return -1; // Invalid index
	}

	SlabDataPtr *sd = &ptr->slab_data_arr[index];
	Slab slab;
	u64 offset = rel * (8 + sd->sdp.slab_size);

	// Access the slab to ensure it's a valid address
	if (slab_data_access(sd, &slab, offset) != 0) {
		return -1; // Failed to access the slab
	}

	// Update the free list
	u64 old_head = sd->sdp.free_list_head;
	memcpy(slab.data, &old_head, sizeof(u64));
	sd->sdp.free_list_head = rel;

	// zero out memory since it's now freed
	if (ptr->zeroed) {
		u64 start_data = offset + 8;
		/*
		for (u64 i = 0; i < ptr->slab_data_arr[index].sdp.slab_size;
		     i++) {
			((char *)ptr->slab_data_arr[index]
			     .data)[i + start_data] = 0;
		}
		*/
		for (u64 i = 0; i < ptr->slab_data_arr[index].sdp.slab_size;
		     i++) {
			((char *)slab.data)[i + 8] = 0;
		}
	}

	return 0; // Success
}
