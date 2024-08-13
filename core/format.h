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

#ifndef _CORE_FORMAT__
#define _CORE_FORMAT__

#include <core/formatter.h>
#include <core/result.h>
#include <core/string.h>

Result format(FormatterPtr *formatter, char *fmt, ...);

#define PROC_TYPE(sign_upper, sign_lower, bits, value)                         \
	({                                                                     \
		mymalloc(                                                      \
		    &((Slab *)_slabs__format_impl__.data)[_itt_format_impl__], \
		    sizeof(sign_upper##bits));                                 \
		sign_upper##bits##Ptr *_ptr_proc_type__ =                      \
		    ((Slab *)_slabs__format_impl__.data)[_itt_format_impl__]   \
			.data;                                                 \
		BUILDPTR(_ptr_proc_type__, sign_upper##bits);                  \
		memcpy(&_ptr_proc_type__->_value, &value,                      \
		       sizeof(sign_lower##bits));                              \
		_itt_format_impl__++;                                          \
		_ptr_proc_type__;                                              \
	})

#define PROC_ARG(value)                                                        \
	_Generic((value),                                                      \
	    i8: ({ PROC_TYPE(I, i, 8, value); }),                              \
	    i16: ({ PROC_TYPE(I, i, 16, value); }),                            \
	    i32: ({ PROC_TYPE(I, i, 32, value); }),                            \
	    i64: ({ PROC_TYPE(I, i, 64, value); }),                            \
	    i128: ({ PROC_TYPE(I, i, 128, value); }),                          \
	    u8: ({ PROC_TYPE(U, u, 8, value); }),                              \
	    u16: ({ PROC_TYPE(U, u, 16, value); }),                            \
	    u32: ({ PROC_TYPE(U, u, 32, value); }),                            \
	    u64: ({ PROC_TYPE(U, u, 64, value); }),                            \
	    u128: ({ PROC_TYPE(U, u, 128, value); }),                          \
	    f32: ({ PROC_TYPE(F, f, 32, value); }),                            \
	    f64: ({ PROC_TYPE(F, f, 64, value); }),                            \
	    bool: ({                                                           \
			 mymalloc(&((Slab *)_slabs__format_impl__              \
					.data)[_itt_format_impl__],            \
				  sizeof(Bool));                               \
			 BoolPtr *_ptr_proc_arg_impl__ =                       \
			     ((Slab *)_slabs__format_impl__                    \
				  .data)[_itt_format_impl__]                   \
				 .data;                                        \
			 BUILDPTR(_ptr_proc_arg_impl__, Bool);                 \
			 memcpy(&_ptr_proc_arg_impl__->_value, &value,         \
				sizeof(bool));                                 \
			 _itt_format_impl__++;                                 \
			 _ptr_proc_arg_impl__;                                 \
		 }),                                                           \
	    default: ({                                                        \
			 ((Slab *)                                             \
			      _slabs__format_impl__.data)[_itt_format_impl__]  \
			     .data = NULL;                                     \
			 _itt_format_impl__++;                                 \
			 &value;                                               \
		 }))

#define COUNT_FORMAT(value) ({ __counter___ += 1; })

#define FORMAT(f, fmt, ...)                                                    \
	({                                                                     \
		({                                                             \
			u64 __counter___ = 0;                                  \
			EXPAND(FOR_EACH(COUNT_FORMAT, __VA_ARGS__));           \
			Slab _slabs__format_impl__;                            \
                                                                               \
			mymalloc(&_slabs__format_impl__,                       \
				 sizeof(Slab) * __counter___);                 \
			u64 _itt_format_impl__ = 0;                            \
			ResultPtr _ret__format_impl__ =                        \
			    format(f, fmt __VA_OPT__(, ) EXPAND(               \
					  FOR_EACH(PROC_ARG, __VA_ARGS__)));   \
			for (u64 _i__ = 0; _i__ < __counter___; _i__++) {      \
				if (((Slab *)_slabs__format_impl__.data)[_i__] \
					.data != NULL) {                       \
					myfree(&((Slab *)_slabs__format_impl__ \
						     .data)[_i__]);            \
				}                                              \
			}                                                      \
			myfree(&_slabs__format_impl__);                        \
			_ret__format_impl__;                                   \
		});                                                            \
	})

#endif // _CORE_FORMAT__
