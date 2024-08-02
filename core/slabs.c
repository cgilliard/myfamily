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

#include <core/result.h>
#include <core/slabs.h>
#include <limits.h>
#include <stdlib.h>

GETTER(SlabData, data)
SETTER(SlabData, data)

GETTER(SlabData, sdp)
SETTER(SlabData, sdp)

GETTER(SlabAllocator, slab_data_arr);
SETTER(SlabAllocator, slab_data_arr);
GETTER(SlabAllocator, slab_data_arr_size);
GETTER(SlabAllocator, zeroed);
GETTER(SlabAllocator, slabs_per_resize);

GETTER(SlabSizeCount, slab_size);
GETTER(SlabSizeCount, slab_count);
GETTER(SlabSizeCount, max_slabs);

void SlabSizeCount_cleanup(SlabSizeCount *ptr) {}

bool SlabSizeCount_myclone(SlabSizeCount *dst, SlabSizeCount *src) {
	dst->_slab_size = src->_slab_size;
	dst->_slab_count = src->_slab_count;
	dst->_max_slabs = src->_max_slabs;
	return true;
}

Result slab_init_free_list(SlabData *sd, u64 slab_count, u64 slab_offset) {
	SlabDataParams p = GET(SlabData, sd, sdp);
	u64 slab_size = p.slab_size;
	for (u64 i = slab_offset; i < slab_offset + slab_count; i++) {
		char next_bytes[8];
		if (i < (slab_offset + slab_count) - 1) {
			u64 next_u64 = i + 1;
			memcpy(next_bytes, &next_u64, sizeof(u64));
		} else {
			u64 next_u64 = UINT64_MAX;
			memcpy(next_bytes, &next_u64, sizeof(u64));
		}
		u64 offset_next = i * (8 + slab_size);
		Slice slice = SLICE(next_bytes, 8);
		Result r = SlabData_write(sd, &slice, offset_next);
		TRYU(r);
	}
	return Ok(_());
}

void SlabData_cleanup(SlabData *ptr) {
	void *d = GET(SlabData, ptr, data);
	if (d) {
		myfree(d);
		SET(SlabData, ptr, data, NULL);
	}
}

OrdOptions SlabData_cmp(const void *a, const void *b) {
	SlabDataParams params1 = GET(SlabData, a, sdp);
	SlabDataParams params2 = GET(SlabData, b, sdp);
	if (params1.slab_size < params2.slab_size)
		return LessThan;
	if (params1.slab_size > params2.slab_size)
		return GreaterThan;
	return EqualTo;
}

Result SlabData_init(SlabData *ptr, u64 slab_size, u64 initial_slabs,
		     u64 max_slabs) {
	void *d = mymalloc(slab_size * initial_slabs);
	SET(SlabData, ptr, data, d);

	SlabDataParams p;
	p.slab_size = slab_size;
	p.slab_count = initial_slabs;
	p.max_slabs = max_slabs;
	p.free_list_head = 0;

	SET(SlabData, ptr, sdp, p);

	Result r = slab_init_free_list(ptr, initial_slabs, 0);
	TRYU(r);

	return Ok(_());
}

Result SlabData_read(SlabData *ptr, Slice *slice, u64 offset) {
	SlabDataParams p = GET(SlabData, ptr, sdp);
	u64 slab_size = p.slab_size;
	memcpy(GET(Slice, slice, ref),
	       GET(SlabData, ptr, data) + (slab_size + 8) * offset, len(slice));
	return Ok(_());
}

Result SlabData_write(SlabData *ptr, Slice *slice, u64 offset) {
	SlabDataParams p = GET(SlabData, ptr, sdp);
	u64 slab_size = p.slab_size;
	memcpy(GET(SlabData, ptr, data) + (slab_size + 8) * offset,
	       GET(Slice, slice, ref), len(slice));
	return Ok(_());
}

Result SlabData_reference(SlabData *ptr, Slice *slice, u64 offset) {
	SET(Slice, slice, ref, GET(SlabData, ptr, data) + offset);
	return Ok(_());
}

Result SlabData_resize(SlabData *ptr, u64 slabs) {
	void *d = GET(SlabData, ptr, data);
	SlabDataParams p = GET(SlabData, ptr, sdp);
	u64 cur_slabs = p.slab_count;
	u64 slab_size = p.slab_size;

	void *nd = myrealloc(d, slabs * (slab_size + 8));
	if (nd == NULL) {
		return STATIC_ALLOC_RESULT;
	}
	SET(SlabData, ptr, data, nd);

	if (slabs > cur_slabs) {
		Result r =
		    slab_init_free_list(ptr, slabs - cur_slabs, cur_slabs);
		TRYU(r);
	}

	return Ok(_());
}

Result SlabAllocator_build_impl(int num, va_list ptr) {
	u64 count = 0;
	SlabDataPtr *slab_data_arr = NULL;
	u64 slabs_per_resize = 10;
	bool zeroed = false;
	for (int i = 0; i < num; i++) {
		SlabAllocatorOptionPtr next;
		Rc rc = va_arg(ptr, Rc);
		void *vptr = unwrap(&rc);
		memcpy(&next, vptr, size(vptr));
		// clang-format off
		MATCH(next,
			VARIANT(SLAB_OPTION_SIZE_COUNT, {
				SlabSizeCount sac = TRY_ENUM_VALUE(
					sac,
					SlabSizeCount,
					next
				);

				if(slab_data_arr) {
					void *tmp = myrealloc(slab_data_arr, sizeof(SlabDataPtr) * (count + 1));
					if(!tmp)
						return STATIC_ALLOC_RESULT;
					slab_data_arr = tmp;
				} else {
					slab_data_arr = mymalloc(sizeof(SlabDataPtr));
					if(!slab_data_arr)
						return STATIC_ALLOC_RESULT;
				}

				u64 slab_size = GET(SlabSizeCount, &sac, slab_size);
				u64 slab_count = GET(SlabSizeCount, &sac, slab_count);
				u64 max_slabs = GET(SlabSizeCount, &sac, max_slabs);
				SlabDataPtr *ptr = &slab_data_arr[count];
				BUILDPTR(ptr, SlabData);

				Result r = SlabData_init(
					&slab_data_arr[count],
					slab_size,
					slab_count,
					max_slabs
				);

				TRYU(r);
				count += 1;
			})
			VARIANT(SLAB_OPTION_ZEROED, {
				zeroed = TRY_ENUM_VALUE(zeroed, bool, next);
			})
			VARIANT(SLAB_OPTION_SLABS_PER_RESIZE, {
				slabs_per_resize = TRY_ENUM_VALUE(slabs_per_resize, u64, next);
			})
		);
		// clang-format on
	}

	if (count > 1) {
		myqsort(slab_data_arr, count);
	}

	for (int i = 0; i < count; i++) {
	}

	SlabAllocator ret = BUILD(SlabAllocator, slab_data_arr, count, zeroed,
				  slabs_per_resize);
	return Ok(ret);
}

Result SlabAllocator_build(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return SlabAllocator_build_impl(n, ptr);
}

Result SlabAllocator_allocate(SlabAllocator *ptr, u64 sz) {
	SlabDataPtr *arr = GET(SlabAllocator, ptr, slab_data_arr);
	bool zeroed = GET(SlabAllocator, ptr, zeroed);
	u64 count = GET(SlabAllocator, ptr, slab_data_arr_size);
	SlabData value = BUILD(SlabData, NULL, {sz, 0, 0, 0});
	Result r1 = binsearch(arr, count, (Object *)&value);
	Option opt = TRY(r1, opt);
	if (IS_NONE(opt)) {
		// we don't have this size return None
		return Ok(None);
	} else {
		char next_bytes[8];
		u64 index = UNWRAP_VALUE(opt, index);
		// shift and mask index as the last byte
		u64 ret = arr[index]._sdp.free_list_head | (index << 56);
		if (ret == UINT64_MAX) {
			// TODO: check if we haven't hit our max slabs, if not
			// resize.

			u64 max_slabs = arr[index]._sdp.max_slabs;
			u64 slab_count = arr[index]._sdp.slab_count;
			u64 slabs_per_resize =
			    GET(SlabAllocator, ptr, slabs_per_resize);

			u64 cur_slabs = slab_count;
			if (slab_count < max_slabs) {
				u64 resize_count = slabs_per_resize;
				slab_count += slabs_per_resize;
				if (slab_count > max_slabs) {
					resize_count = max_slabs - slab_count;
					slab_count = max_slabs;
				}

				Result r2 = slab_init_free_list(
				    &arr[index], resize_count, cur_slabs);
				TRYU(r2);
				arr[index]._sdp.slab_count = slab_count;
				arr[index]._sdp.free_list_head = cur_slabs;
				ret = arr[index]._sdp.free_list_head |
				      (index << 56);
			}

			if (ret == UINT64_MAX)
				return Ok(None);
		}

		u64 offset_next = ret * (8 + arr[index]._sdp.slab_size);
		Slice slice_next = SLICE(next_bytes, 8);
		Result r = SlabData_read(&arr[index], &slice_next, offset_next);
		TRYU(r);
		memcpy(&arr[index]._sdp.free_list_head, slice_next._ref, 8);
		Option opt = Some(ret);

		if (zeroed) {
			u64 start_data = offset_next + 8;
			for (u64 i = 0; i < arr[index]._sdp.slab_size; i++) {
				((char *)arr[index]._data)[i + start_data] = 0;
			}
		}

		return Ok(opt);
	}
}

Result SlabAllocator_get(SlabAllocator *ptr, Slice *slice, u64 id) {
	u64 index = (id >> 56) & 0xFF;
	u64 rel = id & 0x00FFFFFFFFFFFFFF;
	u64 count = GET(SlabAllocator, ptr, slab_data_arr_size);
	if (index >= count) {
		Error e =
		    ERR(ILLEGAL_ARGUMENT,
			"index: %llu greater than or equal to count: %llu",
			index, count);
		return Err(e);
	}
	SlabDataPtr *arr = GET(SlabAllocator, ptr, slab_data_arr);
	SlabDataParams sdp = GET(SlabData, &arr[index], sdp);
	SET(Slice, slice, ref,
	    GET(SlabData, &arr[index], data) + rel * (8 + sdp.slab_size) + 8);
	SET(Slice, slice, len, sdp.slab_size);

	return Ok(_());
}

Result SlabAllocator_free(SlabAllocator *ptr, u64 id) {
	u64 index = (id >> 56) & 0xFF;
	u64 rel = id & 0x00FFFFFFFFFFFFFF;
	u64 count = GET(SlabAllocator, ptr, slab_data_arr_size);
	if (index >= count) {
		Error e =
		    ERR(ILLEGAL_ARGUMENT,
			"index: %llu greater than or equal to count: %llu",
			index, count);
		return Err(e);
	}

	SlabDataPtr *arr = GET(SlabAllocator, ptr, slab_data_arr);

	char next_bytes[8];
	u64 offset_next = rel * (8 + arr[index]._sdp.slab_size);
	Slice slice_next = SLICE(next_bytes, 8);
	Result r = SlabData_reference(&arr[index], &slice_next, offset_next);
	TRYU(r);
	memcpy(slice_next._ref, &arr[index]._sdp.free_list_head, sizeof(u64));
	arr[index]._sdp.free_list_head = rel;

	return Ok(_());
}

void SlabAllocator_cleanup(SlabAllocator *ptr) {
	SlabDataPtr *slab_data_arr = GET(SlabAllocator, ptr, slab_data_arr);
	u64 count = GET(SlabAllocator, ptr, slab_data_arr_size);
	if (slab_data_arr) {
		for (u64 i = 0; i < count; i++) {
			cleanup(&slab_data_arr[i]);
		}
		myfree(slab_data_arr);
		SET(SlabAllocator, ptr, slab_data_arr, NULL);
	}
}
