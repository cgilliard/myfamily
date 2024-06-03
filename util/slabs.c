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

#include <string.h>
#include <stdlib.h>
#include <util/slabs.h>
#include <log/log.h>

#define LOG_LEVEL Debug

int slab_data_init(SlabData *sd, u64 len) {
	sd->data = NULL;
	return slab_data_resize(sd, len);
}

void *slab_data_read(SlabData *sd, u64 offset) {
	if(offset > sd->len)
		return NULL;
	return sd->data + offset;
}

int slab_data_write(SlabData *sd, u64 dst_offset, void *value, u64 src_offset, u64 len) {
        int ret = 0;
	memcpy(sd->data + dst_offset, value + src_offset, len);
        return ret;
}

int slab_data_resize(SlabData *sd, u64 len) {
	int ret = 0;

	if (sd->data == NULL) {
		sd->data = malloc(len);
		if(sd->data == NULL) {
			error("Could not allocate the required memory.");
			ret = -1;
		}
	} else {
		void *tmp = realloc(sd->data, len);
		if(tmp == NULL) {
			error("Could not allocate the required memory.");
			ret = -1;
		} else {
			sd->data = tmp;
		}
	}

	return ret;
}

void slab_data_free(SlabData *sd) {
	if(sd->data) {
		free(sd->data);
		sd->data = NULL;
	}
}

int init_free_list(
		SlabData *slab_data,
		u64 size,
		u64 offset
		) {
	return 0;
}



int slab_init(SlabAllocator *sa, int num, u64 *sizes, u64 *max_slabs, u64 slabs_per_resize, bool zeroed) {
	// check that ordering is sorted and other configs are ok
	u64 last_size = 0;
	for(int i=0; i<num; i++) {
		if(sizes[i] <= last_size) {
			error("slab_allocator sizes must be sorted in ascending order.");
			return -1;
		}
		last_size = sizes[i];
		if(max_slabs[i] == 0) {
			error("max_slabs must be greater than 0");
			return -1;
		}
	}

	if(slabs_per_resize == 0) {
		error("slabs_per_resize must be greater than 0");
		return -1;
	}

	int ret = 0;
	sa->slab_data_array = NULL;
	sa->sizes = NULL;
	sa->max_slabs = NULL;
	sa->zeroed = zeroed;
	sa->slabs_per_resize = slabs_per_resize;
	sa->slab_data_array = malloc(sizeof(SlabData) * num);
	if(sa->slab_data_array == NULL) {
		ret = -1;
		error("Could not allocate the required memory.");
	}
	
	if(ret == 0) {
		sa->sizes = malloc(sizeof(u64) * num);

		if(sa->sizes == NULL) {
			free(sa->slab_data_array);
			sa->slab_data_array = NULL;
			ret = -1;
			error("Could not allocate the required memory.");
		} else {
			sa->slab_data_array_len = num;
			memcpy(sa->sizes, sizes, num);

			for(int i=0; i<num; i++) {
				if(slab_data_init(
					&sa->slab_data_array[i],
					slabs_per_resize * sizes[i]
				)) {
					ret = -1;
					break;
				}
			}
		}

	}

	if(ret == 0) {
		sa->max_slabs = malloc(sizeof(u64) * num);
		if(sa->max_slabs == NULL) {
			free(sa->slab_data_array);
                        sa->slab_data_array = NULL;
			free(sa->sizes);
			sa->sizes = NULL;
			error("Could not allocate required memory.");
			ret = -1;
		} else {
			memcpy(sa->max_slabs, max_slabs, num);
		}
	}

	return ret;
}

