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

#include <core/traits.h>

Result to_string(void *obj);
Result to_string_buf(void *obj, usize buf_size);
Result to_debug(void *obj);
Result to_debug_buf(void *obj, usize buf_size);

CLASS(Formatter,
      FIELD(char *, buf) FIELD(u64, len) FIELD(u64, pos) FIELD(bool, cleanup))
IMPL(Formatter, TRAIT_FORMATTER)
IMPL(Formatter, TRAIT_TO_STRING)
#define Formatter DEFINE_CLASS(Formatter)

#define WRITE(f, ...) Formatter_write(f, __VA_ARGS__)
#define TO_STRING(x)                                                           \
	({                                                                     \
		Result _rr11__ = to_string(x);                                 \
		StringPtr _rr22__ = TRY(_rr11__, _rr22__);                     \
		_rr22__;                                                       \
	})

#define TO_DEBUG(x)                                                            \
	({                                                                     \
		Result _rr11__ = to_debug(x);                                  \
		StringPtr _rr22__ = TRY(_rr11__, _rr22__);                     \
		_rr22__;                                                       \
	})

#define FORMATTER(size)                                                        \
	({                                                                     \
		char *_buf__ = mymalloc(size * sizeof(char));                  \
		FormatterPtr _rr55__ =                                         \
		    BUILD(Formatter, _buf__, size, 0, true);                   \
		_rr55__;                                                       \
	})

#endif // _CORE_FORMATTER__
