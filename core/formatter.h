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

#ifndef _CORE_FORMATTER__
#define _CORE_FORMATTER__

#include <core/prim.h>
#include <core/traits.h>
#include <core/unit.h>

Result to_string(void *obj);
Result to_string_buf(void *obj, u64 buf_size);
Result to_debug(void *obj);
Result to_debug_buf(void *obj, u64 buf_size);

CLASS(Formatter, FIELD(Slab, buf) FIELD(u64, pos) FIELD(bool, cleanup))
IMPL(Formatter, TRAIT_FORMATTER)
IMPL(Formatter, TRAIT_TO_STRING)
#define Formatter DEFINE_CLASS(Formatter)

void Formatter_reset(Formatter *fmt);

#undef I8
IMPL(I8, TRAIT_DISPLAY)
#define I8 DEFINE_CLASS(I8)

#undef I16
IMPL(I16, TRAIT_DISPLAY)
#define I16 DEFINE_CLASS(I16)

#undef I32
IMPL(I32, TRAIT_DISPLAY)
#define I32 DEFINE_CLASS(I32)

#undef I64
IMPL(I64, TRAIT_DISPLAY)
#define I64 DEFINE_CLASS(I64)

#undef I128
IMPL(I128, TRAIT_DISPLAY)
#define I128 DEFINE_CLASS(I128)

#undef U8
IMPL(U8, TRAIT_DISPLAY)
#define U8 DEFINE_CLASS(U8)

#undef U16
IMPL(U16, TRAIT_DISPLAY)
#define U16 DEFINE_CLASS(U16)

#undef U32
IMPL(U32, TRAIT_DISPLAY)
#define U32 DEFINE_CLASS(U32)

#undef U64
IMPL(U64, TRAIT_DISPLAY)
#define U64 DEFINE_CLASS(U64)

#undef U128
IMPL(U128, TRAIT_DISPLAY)
#define U128 DEFINE_CLASS(U128)

#undef F32
IMPL(F32, TRAIT_DISPLAY)
#define F32 DEFINE_CLASS(F32)

#undef F64
IMPL(F64, TRAIT_DISPLAY)
#define F64 DEFINE_CLASS(F64)

#undef Bool
IMPL(Bool, TRAIT_DISPLAY)
#define Bool DEFINE_CLASS(Bool)

#define WRITE(f, ...) Formatter_write(f, __VA_ARGS__)
#define TO_STRING(x)                                                           \
	({                                                                     \
		({                                                             \
			Result r = to_string(x);                               \
			StringPtr sptr = TRY(r, sptr);                         \
			sptr;                                                  \
		});                                                            \
	})

#define TO_DEBUG(x)                                                            \
	({                                                                     \
		({                                                             \
			Result r = to_debug(x);                                \
			StringPtr sptr = TRY(r, sptr);                         \
			sptr;                                                  \
		});                                                            \
	})

#define FORMATTER(size)                                                        \
	({                                                                     \
		({                                                             \
			Slab buf;                                              \
			if (mymalloc(buf, size * sizeof(char)))                \
				return STATIC_ALLOC_RESULT;                    \
			FormatterPtr f = BUILD(Formatter, buf, 0, true);       \
			f;                                                     \
		});                                                            \
	})

#define FORMATTERP(size)                                                       \
	({                                                                     \
		({                                                             \
			Slab buf;                                              \
			if (mymalloc(buf, size * sizeof(char)))                \
				panic("Could not allocate sufficient memory"); \
			FormatterPtr f = BUILD(Formatter, buf, 0, true);       \
			f;                                                     \
		});                                                            \
	})

#endif // _CORE_FORMATTER__