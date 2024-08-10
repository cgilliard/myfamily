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
#define Option DEFINE_ENUM(Option)

#define IS_ERR(x) x.type == Err
#define IS_OK(x) x.type == Ok

#define IS_SOME(x) x.type == SOME
#define IS_NONE(x) x.type == NONE

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

#define Ok(x)                                                                  \
	({                                                                     \
		({                                                             \
			Rc ___rc__##x = HEAPIFY(x);                            \
			ResultPtr _r__##x =                                    \
			    BUILD_ENUM(Result, Ok, ___rc__##x);                \
			if (_r__##x.slab.data == NULL) {                       \
				_r__##x = STATIC_ALLOC_RESULT;                 \
			}                                                      \
			_r__##x;                                               \
		});                                                            \
	})

#define Err(e)                                                                 \
	({                                                                     \
		({                                                             \
			ResultPtr _r__##x = BUILD_ENUM(Result, Err, e);        \
			if (_r__##x.slab.data == NULL) {                       \
				_r__##x = STATIC_ALLOC_RESULT;                 \
			}                                                      \
			_r__##x;                                               \
		});                                                            \
	})

static Result STATIC_ALLOC_RESULT = {
    {&ResultPtr_Vtable__, "Result"},
    Err,
    ENUM_FLAG_NO_CLEANUP,
    {UINT64_MAX, &STATIC_ALLOC_ERROR_RC, sizeof(Rc)}};

#define UNWRAP_TYPE(type, x)                                                   \
	({                                                                     \
		({                                                             \
			Rc _out__##x = ENUM_VALUE(_out__##x, Rc, x);           \
			*(type *)unwrap(&_out__##x);                           \
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
				 Rc _unwrap_value__rc_##v =                    \
				     ENUM_VALUE(_unwrap_value__rc_##v, Rc, x); \
				 void *_ptr__##v =                             \
				     unwrap(&_unwrap_value__rc_##v);           \
				 memcpy(&v, _ptr__##v, mysize(_ptr__##v));     \
				 v;                                            \
			 });                                                   \
		 }))

#define UNWRAP_ERR(x)                                                          \
	({                                                                     \
		if (IS_OK(x))                                                  \
			panic("Attempt to unwrap_err an ok");                  \
		*(Error *)unwrap(x.slab.data);                                 \
	})

#define TRY(x, v)                                                              \
	({                                                                     \
		if (!strcmp(CLASS_NAME(&x), "Option") && IS_NONE(x)) {         \
			Error _e__##x = ERR(UNWRAP_NONE, "Unwrap on a none");  \
			return Err(_e__##x);                                   \
		}                                                              \
		if ((!strcmp(CLASS_NAME(&x), "Result") && IS_ERR(x))) {        \
			Error _e__##x = UNWRAP_ERR(x);                         \
			return Err(_e__##x);                                   \
		}                                                              \
		UNWRAP_VALUE(x, v);                                            \
	})

#define TRYU(x)                                                                \
	({                                                                     \
		if (IS_ERR(x)) {                                               \
			Error _e2__##x = UNWRAP_ERR(x);                        \
			return Err(_e2__##x);                                  \
		}                                                              \
	})

#define EXPECT(x, v)                                                           \
	({                                                                     \
		if (!strcmp(CLASS_NAME(&x), "Option") && IS_NONE(x)) {         \
			panic("Expect on none");                               \
		}                                                              \
		if ((!strcmp(CLASS_NAME(&x), "Result") && IS_ERR(x))) {        \
			Error e = UNWRAP_ERR(x);                               \
			print(&e);                                             \
			panic("Expect on an error/none");                      \
		}                                                              \
		UNWRAP_VALUE(x, v);                                            \
	})

#define todo()                                                                 \
	({                                                                     \
		({                                                             \
			Error _e__ =                                           \
			    ERR(UNIMPLEMENTED, "Not yet implemented");         \
			return Err(_e__);                                      \
		});                                                            \
	});

#define Some(x)                                                                \
	({                                                                     \
		({                                                             \
			Rc rc = HEAPIFY(x);                                    \
			OptionPtr res = BUILD_ENUM(Option, SOME, rc);          \
			res;                                                   \
		});                                                            \
	})

// special initialization of None to avoid the need to create multiple instances
static Option None = {{&OptionPtr_Vtable__, "Option"},
		      NONE,
		      ENUM_FLAG_NO_CLEANUP,
		      {UINT64_MAX, &UNIT, sizeof(Unit)}};
#endif // _CORE_RESULT__
