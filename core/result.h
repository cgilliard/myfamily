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
			Rc rc = HEAPIFY(x);                                    \
			ResultPtr r = BUILD_ENUM(Result, Ok, rc);              \
			if (r.slab.data == NULL) {                             \
				r = STATIC_ALLOC_ERROR_RESULT;                 \
			}                                                      \
			r;                                                     \
		});                                                            \
	})

#define Err(e)                                                                 \
	({                                                                     \
		({                                                             \
			ResultPtr r = BUILD_ENUM(Result, Err, e);              \
			if (r.slab.data == NULL) {                             \
				r = STATIC_ALLOC_ERROR_RESULT;                 \
			}                                                      \
			r;                                                     \
		});                                                            \
	})

#define UNWRAP_TYPE(type, x)                                                   \
	({                                                                     \
		({                                                             \
			Rc rc = ENUM_VALUE(rc, Rc, x);                         \
			type ret = *(type *)unwrap(&rc);                       \
			ret;                                                   \
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
				 Rc rc = ENUM_VALUE(rc, Rc, x);                \
				 void *ptr = unwrap(&rc);                      \
				 memcpy(&v, ptr, mysize(ptr));                 \
				 v;                                            \
			 });                                                   \
		 }))

#define UNWRAP_ERR(x)                                                          \
	({                                                                     \
		({                                                             \
			if (IS_OK(x))                                          \
				panic("Attempt to unwrap_err an ok");          \
			Error e = *(Error *)unwrap(x.slab.data);               \
			e;                                                     \
		});                                                            \
	})

#define TRY(x, v)                                                              \
	({                                                                     \
		if (!strcmp(CLASS_NAME(&x), "Option") && IS_NONE(x)) {         \
			Error e = ERR(UNWRAP_NONE, "Unwrap on a none");        \
			return Err(e);                                         \
		}                                                              \
		if ((!strcmp(CLASS_NAME(&x), "Result") && IS_ERR(x))) {        \
			Error e = UNWRAP_ERR(x);                               \
			return Err(e);                                         \
		}                                                              \
		UNWRAP_VALUE(x, v);                                            \
	})

#define TRYU(x)                                                                \
	({                                                                     \
		if (IS_ERR(x)) {                                               \
			Error e = UNWRAP_ERR(x);                               \
			return Err(e);                                         \
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
			Error e = ERR(UNIMPLEMENTED, "Not yet implemented!");  \
			return Err(e);                                         \
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

#endif // _CORE_RESULT__
