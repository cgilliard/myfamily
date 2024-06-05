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
#include <base/types.h>
#include <limits.h>
#include <stdarg.h>
#include <log/log.h>

#define LOG_LEVEL Debug

int slab_data_init(SlabData *sd, u64 slab_count, u64 slab_size, u64 max_slabs) {
	sd->data = NULL;
	sd->sdp.slab_size = slab_size;
	sd->sdp.slab_count = slab_count;
	sd->sdp.max_slabs = max_slabs;
	sd->sdp.free_list_head = 0;

	sd->sdp.ptr_size = 0;
        // add 2 (1 termination pointer and one for free status)
        u64 x = saddu64(max_slabs, 2);
        while(true) {
        	if(x == 0) {
                	break;
                }
                x >>= 8;
                sd->sdp.ptr_size += 1;
        }

	sd->sdp.null_ptr = 0;
	int shft = 0;
	for(int i=0; i<sd->sdp.ptr_size; i++) {
		sd->sdp.null_ptr |= 0xFF << shft;
		shft += 8;
	}

	debug(
		"sd->sdp.ptr_size=%d,max_slabs=%llu,null_ptr=%llu",
		sd->sdp.ptr_size,
		max_slabs,
		sd->sdp.null_ptr
	);


	int ret = slab_data_resize(sd, slab_count);

	return ret;
}

void *slab_data_read(SlabData *sd, u64 offset) {
	if(offset > sd->sdp.slab_size * sd->sdp.slab_count)
		return NULL;
	return sd->data + offset;
}

int slab_data_write(SlabData *sd, u64 dst_offset, void *value, u64 src_offset, u64 len) {
        int ret = 0;
	debug("=====sdw dst_offset=%llu,src_offset=%llu,len=%llu", dst_offset, src_offset, len);
	memcpy(sd->data + dst_offset, value + src_offset, len);
        return ret;
}

int slab_data_resize(SlabData *sd, u64 slabs) {
	debug("slab_data_resize %llu", slabs);
	u64 len = slabs * sd->sdp.slab_size;
	int ret = 0;

	if (sd->data == NULL) {
		debug("slab_data_resize malloc %llu", len);
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

void slab_write_ptr(char *bytes, u64 n, u64 ptr_size) {
	for(int j=ptr_size-1; j>=0; j--) {
                bytes[j] = n & 0xFF;
        	n = n >> 8;
        }
}

void slab_set_max(char *bytes, u64 ptr_size) {
	for(int j=0; j<ptr_size; j++)
		bytes[j] = 0xFF;
}

u64 slab_read_ptr(unsigned char *bytes, u64 ptr_size) {
	u64 ret = 0;
	debug("slab_read_ptr with ptr_size = %llu", ptr_size);

	int shft = 0;
	for(int j=ptr_size-1; j>=0; j--) {
		ret += bytes[j] << shft;
		shft += 8;
	}

	debug("ret=%llu", ret);

	return ret;
}

int slab_init_free_list(SlabData *sd, u64 size, u64 offset) {
	debug("init free list %llu off=%llu, size=%llu", sd->sdp.slab_size, offset, size);
	int ret = 0;
	u64 ptr_size = sd->sdp.ptr_size;
	u64 slab_size = sd->sdp.slab_size;

	for(u64 i=offset; i<offset + size; i++) {
		char next_bytes[ptr_size];
		if(i < (offset + size) - 1) {
			u64 n = i + 1;
			slab_write_ptr(next_bytes, n, ptr_size);
		} else {
			slab_set_max(next_bytes, ptr_size);
		}
		u64 offset_next = i * (ptr_size + slab_size);
		if(i < 1)
			debug(
				"writing first block at %llu,ptr_size=%llu, [0]=%d,[1]=%d",
				offset_next,
				ptr_size,
				next_bytes[0],
				next_bytes[1]
			);
		slab_data_write(sd, offset_next, next_bytes, 0, ptr_size);
	}


	return ret;
}

int do_slabs(SlabAllocator *sa, int num, va_list valist) {
	va_list args_copy;
        va_copy(args_copy, valist);
	int ret = 0;
	int sd_count = 0;
	u64 slabs_per_resize = 100;
	bool zeroed = true;

	for(int i=0; i<num; i++) {
		SlabAllocatorConfigImpl next = va_arg(valist, SlabAllocatorConfigImpl);
		if(next.type == SlabAllocatorConfigImplTypeSlabData)
			sd_count += 1;
		else if(next.type == SlabAllocatorConfigImplTypeSlabsPerResize) {
			slabs_per_resize = *((u64 *)next.value);
		} else if(next.type == SlabAllocatorConfigImplTypeZeroed) {
			zeroed = *((bool *)next.value);
		}
	}

	if(sd_count > 254) {
		error("slab_allocator may not have more than 254 slab sizes.");
		return -1;
	}

	u64 sizes[sd_count];
	u64 max_slabs[sd_count];
	int counter = 0;
	for(int i=0; i<num; i++) {
                SlabAllocatorConfigImpl next = va_arg(args_copy, SlabAllocatorConfigImpl);
                if(next.type == SlabAllocatorConfigImplTypeSlabData) {
			u64 slab_size = ((u64 *)next.value)[1];
			u64 max_slabs_value = ((u64 *)next.value)[0];
			printf("ss=%llu,max=%llu\n", slab_size, max_slabs_value);
			sizes[counter] = slab_size;
			max_slabs[counter] = max_slabs_value;
			counter += 1;
		}
        }
	printf("counter=%d\n", counter);

	// check that ordering is sorted and other configs are ok
        u64 last_size = 0;
        for(int i=0; i<counter; i++) {
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

        sa->slab_data_array = NULL;
        sa->sizes = NULL;
        sa->max_slabs = NULL;
        sa->zeroed = zeroed;
        sa->slabs_per_resize = slabs_per_resize;
        sa->slab_data_array = malloc(sizeof(SlabData) * counter);
        if(sa->slab_data_array == NULL) {
                ret = -1;
                error("Could not allocate the required memory.");
        }

	        if(ret == 0) {
                sa->sizes = malloc(sizeof(u64) * counter);

                if(sa->sizes == NULL) {
                        free(sa->slab_data_array);
                        sa->slab_data_array = NULL;
                        ret = -1;
                        error("Could not allocate the required memory.");
                } else {
                        sa->slab_data_array_len = counter;
                        memcpy(sa->sizes, sizes, counter * sizeof(u64));
                }

        }

        if(ret == 0) {
                sa->max_slabs = malloc(sizeof(u64) * counter);
                if(sa->max_slabs == NULL) {
                        free(sa->slab_data_array);
                        sa->slab_data_array = NULL;
                        free(sa->sizes);
                        sa->sizes = NULL;
                        error("Could not allocate required memory.");
                        ret = -1;
                } else {
                        memcpy(sa->max_slabs, max_slabs, counter * sizeof(u64));
                }
        }

	if(ret == 0) {
 		for(int i=0; i<counter; i++) {
			u64 initial_slabs = slabs_per_resize;
			if(slabs_per_resize > sa->max_slabs[i]) {
				initial_slabs = sa->max_slabs[i];
			}
			debug(
				"initial_slabs=%llu,max=%llu,spr=%llu",
				initial_slabs,
				sa->max_slabs[i],
				slabs_per_resize
			);
                	if(slab_data_init(
                        	&sa->slab_data_array[i],
                                initial_slabs,
                                sa->sizes[i],
                                sa->max_slabs[i]
                        )) {
                        	ret = -1; 
                                break;
                        }
                }
	}

	if(ret == 0) {
		for(int i=0; i<sa->slab_data_array_len; i++) {
			u64 size = slabs_per_resize;
			if(sa->slab_data_array[i].sdp.max_slabs < slabs_per_resize)
				size = sa->slab_data_array[i].sdp.max_slabs;
                	slab_init_free_list(&sa->slab_data_array[i], size, 0);
                }
	}

        return ret;	
}

int slab_init(SlabAllocator *sa, int num, ...) {
    va_list valist;
    va_start(valist, num);
    int ret = do_slabs(sa, num, valist);
    va_end(valist);
    return ret;
}

int slab_index_for_size(SlabAllocator *sa, u64 size) {
	// slab sizes are sorted so we can do binary search
	int slab_data_array_len = sa->slab_data_array_len;
	int mid = slab_data_array_len / 2;
	int min = 0;
	int max = slab_data_array_len - 1;

	while(true) {
		if(sa->sizes[mid] == size)
			return mid;
		else if(sa->sizes[mid] > size) {
			if(max <= min)
				break;
			max = mid - 1;
		} else {
			if(max <= min)
				break;
			min = mid + 1;
		}
		mid = (min + (max - min) / 2);
	}

	return -1;
}

u64 slab_allocate(SlabAllocator *sa, u64 size) {
	int idx = slab_index_for_size(sa, size);
	if(idx < 0)
		return ULONG_MAX;

	u64 slab_size = sa->slab_data_array[idx].sdp.slab_size;
	u64 ptr_size = sa->slab_data_array[idx].sdp.ptr_size;
	u64 cur_slabs = sa->slab_data_array[idx].sdp.slab_count;
	u64 max_slabs = sa->slab_data_array[idx].sdp.max_slabs;
	u64 head = sa->slab_data_array[idx].sdp.free_list_head;

	if(head == sa->slab_data_array[idx].sdp.null_ptr) {
		if(max_slabs > cur_slabs) {
			debug("resize possible for %d,max=%llu,cur=%llu", idx, max_slabs, cur_slabs);
			u64 nslabs = cur_slabs + sa->slabs_per_resize;
			if(nslabs > max_slabs) {
				nslabs = max_slabs;
			}
			slab_data_resize(&sa->slab_data_array[idx], nslabs);
			slab_init_free_list(&sa->slab_data_array[idx], nslabs - cur_slabs, cur_slabs);
			sa->slab_data_array[idx].sdp.free_list_head = cur_slabs;
			sa->slab_data_array[idx].sdp.slab_count = nslabs;
			head = cur_slabs;
		} else {
			return ULONG_MAX;
		}
	}

	sa->slab_data_array[idx].sdp.free_list_head = slab_read_ptr(
		sa->slab_data_array[idx].data + head * (ptr_size + slab_size),
		ptr_size
	);
	debug("reading next free list at relative id = %llu", head);

	if(sa->zeroed) {
		for(int i=0; i<slab_size; i++) {
			u64 offset = head * (ptr_size + slab_size) + ptr_size + i;
			(((char*)sa->slab_data_array[idx].data)[offset]) = 0;
		}
	}

	debug(
		"head=%llu,next=%llu, ULONG_MAX=%llu",
		head,
		sa->slab_data_array[idx].sdp.free_list_head,
		ULONG_MAX
	);
	u64 idx_u64 = idx;
	return head | (idx_u64 << 56);
}

int slab_free(SlabAllocator *sa, u64 id) {
	int ret = 0;
	int idx = (id >> 56) & 0xFF;
	u64 id_relative = id & 0x00FFFFFFFFFFFFFF;

	debug("id=%llu,idx=%d,id_relative=%llu", id, idx, id_relative);
	u64 head = sa->slab_data_array[idx].sdp.free_list_head;
	u64 slab_size = sa->slab_data_array[idx].sdp.slab_size;
	u64 ptr_size = sa->slab_data_array[idx].sdp.ptr_size;

	sa->slab_data_array[idx].sdp.free_list_head = id_relative;
	u64 offset = id_relative * (ptr_size + slab_size);
	slab_write_ptr(sa->slab_data_array[idx].data + offset, head, ptr_size);

	return ret;
}

int slab_write(SlabAllocator *sa, u64 id, Slab *slab, u64 offset_slab) {
	int ret = 0;

 	int idx = (id >> 56) & 0xFF;
        u64 id_relative = id & 0x00FFFFFFFFFFFFFF;
        
        u64 slab_size = sa->slab_data_array[idx].sdp.slab_size;
        u64 ptr_size = sa->slab_data_array[idx].sdp.ptr_size;

	if(offset_slab >= slab_size) {
		error(
			"offset (%llu) is greater than or equal to slab size (%llu)",
			offset_slab,
			slab_size
		);
		return -1;
	}

	if(slab->len != slab_size) {
		error("slab->len (%llu) not equal to slab_size (%llu)", slab->len, slab_size);
		return -1;
	}
                
        u64 offset = id_relative * (ptr_size + slab_size) + offset_slab + ptr_size;

	debug("write offset = %llu", offset);
	u64 len = slab_size - offset_slab;

	slab_data_write(&sa->slab_data_array[idx], offset, slab->data, 0, len);


	return ret;
}

int slab_read(SlabAllocator *sa, u64 id, Slab *slab) {
	int ret = 0;
	int idx = (id >> 56) & 0xFF;
        u64 id_relative = id & 0x00FFFFFFFFFFFFFF;

	u64 slab_size = sa->slab_data_array[idx].sdp.slab_size;
        u64 ptr_size = sa->slab_data_array[idx].sdp.ptr_size;

	u64 offset = id_relative * (ptr_size + slab_size) + ptr_size;
	debug("read offset = %llu", offset);
	slab->data = sa->slab_data_array[idx].data + offset;
	slab->len = slab_size;

	return ret;
}

void slab_allocator_free(SlabAllocator *sa) {
	for(int i=0; i<sa->slab_data_array_len; i++) {
		slab_data_free(&sa->slab_data_array[i]);
	}
	sa->slab_data_array_len = 0;

	if(sa->slab_data_array) {
		free(sa->slab_data_array);
		sa->slab_data_array = NULL;
	}

	if(sa->sizes) {
		free(sa->sizes);
		sa->sizes = NULL;
	}

	if(sa->max_slabs) {
		free(sa->max_slabs);
		sa->max_slabs = NULL;
	}
}

int slab_allocator_config_zeroed(SlabAllocatorConfigImpl *sc, bool zeroed) {
	int ret = 0;
	sc->type = SlabAllocatorConfigImplTypeZeroed;
	sc->value = malloc(sizeof(bool));
        if(sc->value == NULL) {
                ret = -1;
        } else {
                *((bool *)sc->value) = zeroed;
        }
	return ret;
}

int slab_allocator_config_slabs_per_resize(SlabAllocatorConfigImpl *sc, u64 slabs_per_resize) {
	int ret = 0;
	sc->type = SlabAllocatorConfigImplTypeSlabsPerResize;
	sc->value = malloc(sizeof(u64));
	if(sc->value == NULL) {
                ret = -1;
        } else {
		*((u64 *)sc->value) = slabs_per_resize;
	}
	return ret;
}

int slab_allocator_config_slab_data(SlabAllocatorConfigImpl *sc, u64 max_slabs, u64 slab_size) {
	int ret = 0;
	sc->type = SlabAllocatorConfigImplTypeSlabData;
	printf("config slab data\n");
	sc->value = malloc(sizeof(u64) * 2);
	if(sc->value == NULL) {
		ret = -1;
	} else {
		(((u64 *)sc->value)[0]) = max_slabs;
		(((u64 *)sc->value)[1]) = slab_size;
	}
	return ret;
}

void slab_allocator_config_free(SlabAllocatorConfigImpl *sc) {
	if(sc->value != NULL) {
		free(sc->value);
		sc->value = NULL;
	}
}
