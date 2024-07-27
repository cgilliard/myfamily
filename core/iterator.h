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

#ifndef _CORE_ITERATOR__
#define _CORE_ITERATOR__

#include <core/class.h>
#include <core/result.h>

Result next(void *obj);

#define TRAIT_ITERATOR(T) TRAIT_REQUIRED(T, Result, next, T##Ptr *itt)

#define FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, init)             \
	({                                                                     \
		if (!init)                                                     \
			cleanup(&_ret_##counter##__);                          \
		Result _r__ = next(&iterator);                                 \
		if (IS_OK(_r__)) {                                             \
			Option _o__ = TRY(_r__, _o__);                         \
			if (IS_SOME(_o__)) {                                   \
				_ret_##counter##__ = UNWRAP_PRIM(_o__, item);  \
			} else {                                               \
				_has_next_##counter##__ = false;               \
			}                                                      \
		} else {                                                       \
			_has_next_##counter##__ = false;                       \
		}                                                              \
		_ret_##counter##__;                                            \
	})

#define FOR_EACH_IMPL(type, item, iterator, counter, init)                     \
	_Generic((item),                                                       \
	    i8: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),    \
	    i16: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    i32: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    i64: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    i128: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),  \
	    u8: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),    \
	    u16: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    u32: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    u64: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    u128: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),  \
	    f32: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    f64: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),   \
	    bool: FOR_EACH_IMPL_DETAILS(type, item, iterator, counter, true),  \
	    default: ({                                                        \
			 ((Object *)&_ret_##counter##__)->vdata.no_cleanup =   \
			     true;                                             \
			 FOR_EACH_IMPL_DETAILS(type, item, iterator, counter,  \
					       init);                          \
		 }))

#define foreach_impl_expand(type, item, iterator, counter)                     \
	bool _has_next_##counter##__ = true;                                   \
	type _ret_##counter##__;                                               \
	for (type item = FOR_EACH_IMPL(type, item, iterator, counter, true);   \
	     _has_next_##counter##__;                                          \
	     item = FOR_EACH_IMPL(type, item, iterator, counter, false))

#define foreach_impl(type, item, iterator, counter)                            \
	foreach_impl_expand(type, item, iterator, counter)

#define foreach(type, item, iterator)                                          \
	foreach_impl(type, item, iterator, UNIQUE_ID)

#endif // _CORE_ITERATOR__
