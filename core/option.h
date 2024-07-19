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

#ifndef _CORE_OPTION__
#define _CORE_OPTION__

#include <core/enum.h>
#include <core/rc.h>
#include <core/result.h>
#include <core/unit.h>

ENUM(Option, VARIANTS(SOME, NONE), TYPES("Rc", "Unit"))
#define Option DEFINE_ENUM(Option)

#define IS_SOME(x) x.type == SOME
#define IS_NONE(x) x.type == NONE

#define Some(x)                                                                \
	({                                                                     \
		Rc rc = HEAPIFY(x);                                            \
		OptionPtr o = TRY_BUILD_ENUM(Option, SOME, rc);                \
		o;                                                             \
	})

#define SomeP(x)                                                               \
	({                                                                     \
		Rc rc = HEAPIFY(x);                                            \
		OptionPtr o = BUILD_ENUM(Option, SOME, rc);                    \
		o;                                                             \
	})

// special initialization of None to avoid the need to create multiple instances
static Option None = {{&OptionPtr_Vtable__, "Option"}, NONE, &UNIT, true, true};

#endif // _CORE_OPTION__
