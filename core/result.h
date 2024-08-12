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

#ifndef _CORE_RESULT__
#define _CORE_RESULT__

#include <core/ekinds.h>
#include <core/enum.h>
#include <core/error.h>
#include <core/prim.h>
#include <core/rc.h>
#include <core/traits_base.h>
#include <core/unit.h>

ENUM(Result, VARIANTS(Ok, Err), TYPES("Rc", "Error"))
#define Result DEFINE_ENUM(Result)

ENUM(Option, VARIANTS(SOME, NONE), TYPES("Rc", "Unit"))
IMPL(Option, TRAIT_CLONE)
#define Option DEFINE_ENUM(Option)

#define IS_ERR(x) (x).type == Err
#define IS_OK(x) (x).type == Ok

#define IS_SOME(x) (x).type == SOME
#define IS_NONE(x) (x).type == NONE

// static errors for allocation issues so we don't need to call malloc to create
// them.
static Error STATIC_ALLOC_ERROR = {
    {&ErrorPtr_Vtable__, "Error", NEXT_ID},
    false,
    {{&ErrorKindPtr_Vtable__, "ErrorKind", NEXT_ID}, false, "AllocError"},
    "Could not allocate sufficient memory",
    {{&BacktracePtr_Vtable__, "Backtrace", NEXT_ID}, false, NULL, 0},
    ERROR_NO_CLEANUP};

static Rc STATIC_ALLOC_ERROR_RC = {
    {&RcPtr_Vtable__, "Rc", NEXT_ID},
    false,
    {UINT64_MAX, &STATIC_ALLOC_ERROR, sizeof(Error)},
    {},
    RC_FLAGS_NO_COUNTER};

static Result STATIC_ALLOC_ERROR_RESULT = {
    {&ResultPtr_Vtable__, "Result"},
    Err,
    ENUM_FLAG_NO_CLEANUP,
    {UINT64_MAX, &STATIC_ALLOC_ERROR_RC, sizeof(Rc)}};

// special initialization of None to avoid the need to create multiple instances
static Option None = {{&OptionPtr_Vtable__, "Option"},
		      NONE,
		      ENUM_FLAG_NO_CLEANUP,
		      {UINT64_MAX, &UNIT, sizeof(Unit)}};

#define Ok(x)                                                                  \
	({                                                                     \
		({                                                             \
			Rc _rc_ok_impl__ = HEAPIFY(x);                         \
			ResultPtr _r_ok_impl__ =                               \
			    BUILD_ENUM(Result, Ok, _rc_ok_impl__);             \
			if (_r_ok_impl__.slab.data == NULL) {                  \
				_r_ok_impl__ = STATIC_ALLOC_ERROR_RESULT;      \
			}                                                      \
			_r_ok_impl__;                                          \
		});                                                            \
	})

#define Err(e)                                                                 \
	({                                                                     \
		({                                                             \
			ResultPtr _r_err_impl__ = BUILD_ENUM(Result, Err, e);  \
			if (_r_err_impl__.slab.data == NULL) {                 \
				_r_err_impl__ = STATIC_ALLOC_ERROR_RESULT;     \
			}                                                      \
			_r_err_impl__;                                         \
		});                                                            \
	})

#define UNWRAP_TYPE(type, x)                                                   \
	({                                                                     \
		({                                                             \
			Rc _rc_unwrap_type_impl__ =                            \
			    ENUM_VALUE(_rc_unwrap_type_impl__, Rc, x);         \
			type _ret_unwrap_type_impl__ =                         \
			    *(type *)unwrap(&_rc_unwrap_type_impl__);          \
			_ret_unwrap_type_impl__;                               \
		});                                                            \
	})

#define UNWRAP_VALUE(x, v)                                                     \
	_Generic((v),                                                          \
	    u8: UNWRAP_TYPE(u8, x),                                            \
	    u16: UNWRAP_TYPE(u16, x),                                          \
	    u32: UNWRAP_TYPE(u32, x),                                          \
	    u64: UNWRAP_TYPE(u64, x),                                          \
	    u128: UNWRAP_TYPE(u128, x),                                        \
	    i8: UNWRAP_TYPE(i8, x),                                            \
	    i16: UNWRAP_TYPE(i16, x),                                          \
	    i32: UNWRAP_TYPE(i32, x),                                          \
	    i64: UNWRAP_TYPE(i64, x),                                          \
	    i128: UNWRAP_TYPE(i128, x),                                        \
	    f32: UNWRAP_TYPE(f32, x),                                          \
	    f64: UNWRAP_TYPE(f64, x),                                          \
	    bool: UNWRAP_TYPE(bool, x),                                        \
	    default: ({                                                        \
			 ({                                                    \
				 Rc _rc_unwrap_value__ =                       \
				     ENUM_VALUE(_rc_unwrap_value__, Rc, x);    \
				 void *__ptr_unwrap_value__ =                  \
				     unwrap(&_rc_unwrap_value__);              \
				 memcpy(&v, __ptr_unwrap_value__,              \
					mysize(__ptr_unwrap_value__));         \
				 v;                                            \
			 });                                                   \
		 }))

#define UNWRAP_ERR(x)                                                          \
	({                                                                     \
		({                                                             \
			if (IS_OK(x))                                          \
				panic("Attempt to unwrap_err an ok");          \
			Error _e_unwrap_err_impl__ =                           \
			    *(Error *)unwrap(x.slab.data);                     \
			_e_unwrap_err_impl__;                                  \
		});                                                            \
	})

#define TRY(x, v)                                                              \
	({                                                                     \
		if (!strcmp(CLASS_NAME(&x), "Option") && IS_NONE(x)) {         \
			Error _e_try_impl__ =                                  \
			    ERR(UNWRAP_NONE, "Unwrap on a none");              \
			return Err(_e_try_impl__);                             \
		}                                                              \
		if ((!strcmp(CLASS_NAME(&x), "Result") && IS_ERR(x))) {        \
			Error _e_try_impl__ = UNWRAP_ERR(x);                   \
			return Err(_e_try_impl__);                             \
		}                                                              \
		UNWRAP_VALUE(x, v);                                            \
	})

#define TRYU(x)                                                                \
	({                                                                     \
		if (IS_ERR(x)) {                                               \
			Error _e_tryu_impl__ = UNWRAP_ERR(x);                  \
			return Err(_e_tryu_impl__);                            \
		}                                                              \
	})

#define EXPECT(x, v)                                                           \
	({                                                                     \
		if (!strcmp(CLASS_NAME(&x), "Option") && IS_NONE(x)) {         \
			panic("Expect on none");                               \
		}                                                              \
		if ((!strcmp(CLASS_NAME(&x), "Result") && IS_ERR(x))) {        \
			Error _e_expect_impl__ = UNWRAP_ERR(x);                \
			print(&_e_expect_impl__);                              \
			panic("Expect on an error/none");                      \
		}                                                              \
		UNWRAP_VALUE(x, v);                                            \
	})

#define todo()                                                                 \
	({                                                                     \
		({                                                             \
			Error _e__ =                                           \
			    ERR(UNIMPLEMENTED, "Not yet implemented!");        \
			return Err(_e__);                                      \
		});                                                            \
	});

#define Some(x)                                                                \
	({                                                                     \
		({                                                             \
			Rc _rc_some_impl_ = HEAPIFY(x);                        \
			OptionPtr _res_some_impl_ =                            \
			    BUILD_ENUM(Option, SOME, _rc_some_impl_);          \
			_res_some_impl_;                                       \
		});                                                            \
	})

#endif // _CORE_RESULT__
