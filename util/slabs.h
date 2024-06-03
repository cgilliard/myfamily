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

#ifndef __UTIL_SLABS_
#define __UTIL_SLABS_

#include <base/types.h>

struct SlabData {
	void *data;
	u64 len;
};
typedef struct SlabData SlabData;

struct SlabDataParams {
	u64 slab_size;
	u64 slab_count;
	u8 ptr_size;
	u64 free_list_head;
	u64 max_value;
	u8 unallocated[8];
	u8 end[8];
};
typedef struct SlabDataParams SlabDataParams;

struct SlabDataHolder {
	SlabDataParams sdp;
	SlabData sd;
};
typedef struct SlabDataHolder SlabDataHolder;

int slab_data_init(SlabData *sd, u64 len);
void *slab_data_read(SlabData *sd, u64 offset);
int slab_data_write(SlabData *sd, u64 dst_offset, void *value, u64 src_offset, u64 len);
int slab_data_resize(SlabData *sd, u64 len);
void slab_data_free(SlabData *sd);

struct Slab {
	void *data;
	u64 len;
};
typedef struct Slab Slab;

struct SlabAllocator {
	SlabData *slab_data_array;
	u64 *sizes;
	u64 *max_slabs;
	u64 slab_data_array_len;
	u64 slabs_per_resize;
	bool zeroed;
};
typedef struct SlabAllocator SlabAllocator;

int slab_init(SlabAllocator *sa, int num, u64 *sizes, u64 *max_slabs, u64 slabs_per_resize, bool zeroed);
u64 slab_allocate(SlabAllocator *sa, u64 size);
int slab_write(SlabAllocator *sa, u64 id, void *data, u64 offset, u64 len);
int slab_read(SlabAllocator *sa, u64 id, Slab *slab);
int slab_free(SlabAllocator *sa, u64 id);
void slab_allocator_free(SlabAllocator *sa);

#endif /* __UTIL_SLABS_ */
