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
#include <base/cleanup.h>

struct SlabDataParams {
        u64 slab_size;
        u64 slab_count;
	u64 max_slabs;
        u8 ptr_size;
        u64 free_list_head;
	u64 null_ptr;
};
typedef struct SlabDataParams SlabDataParams;

struct SlabData {
	void *data;
	SlabDataParams sdp;
};
typedef struct SlabData SlabData;

int slab_data_init(SlabData *sd, u64 initial_slabs, u64 slab_size, u64 max_slabs);
void *slab_data_read(SlabData *sd, u64 offset);
int slab_data_write(SlabData *sd, u64 dst_offset, void *value, u64 src_offset, u64 len);
int slab_data_resize(SlabData *sd, u64 slabs);
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

enum SlabAllocatorConfigImplType {
	SlabAllocatorConfigImplTypeZeroed = 0,
	SlabAllocatorConfigImplTypeSlabsPerResize = 1,
	SlabAllocatorConfigImplTypeSlabData = 2,
};
typedef enum SlabAllocatorConfigImplType SlabAllocatorConfigImplType;

struct SlabAllocatorConfigImpl {
	SlabAllocatorConfigImplType type;
	void *value;
};
typedef struct SlabAllocatorConfigImpl SlabAllocatorConfigImpl;
#define SlabAllocatorConfig SlabAllocatorConfigImpl CLEANUP(slab_allocator_config_free)

int slab_allocator_config_zeroed(SlabAllocatorConfigImpl *sc, bool zeroed);
int slab_allocator_config_slabs_per_resize(SlabAllocatorConfigImpl *sc, u64 slabs_per_resize);
int slab_allocator_config_slab_data(SlabAllocatorConfigImpl *sc, u64 max_slabs, u64 slab_size);
void slab_allocator_config_free(SlabAllocatorConfigImpl *sc);

int slab_init(SlabAllocator *sa, int num, ...);
u64 slab_allocate(SlabAllocator *sa, u64 size);
int slab_write(SlabAllocator *sa, u64 id, Slab *slab, u64 offset);
int slab_read(SlabAllocator *sa, u64 id, Slab *slab);
int slab_free(SlabAllocator *sa, u64 id);
void slab_allocator_free(SlabAllocator *sa);
#define SlabAllocatorImpl SlabAllocator CLEANUP(slab_config_free)


#endif /* __UTIL_SLABS_ */
