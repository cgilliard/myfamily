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

#ifndef _BASE_RESULT2__
#define _BASE_RESULT2__

#include <base/enum.h>
#include <base/traits_base.h>

CLASS(ResultHolder, FIELD(void *, ref))
IMPL(ResultHolder, TRAIT_COPY)
#define ResultHolder DEFINE_CLASS(ResultHolder)

ENUM(Result2, VARIANTS(Ok, Err), TYPES("ResultHolder", "Error"))
IMPL(Result2, TRAIT_UNWRAP)
#define Result2 DEFINE_ENUM(Result2)

#define Ok2(v)                                                                 \
	({                                                                     \
		ResultHolder rholder = BUILD(ResultHolder);                    \
		rholder._ref = tlmalloc(size(&v));                             \
		printf("1 size = %i\n", size(&v));                             \
		copy(rholder._ref, &v);                                        \
		printf("x\n");                                                 \
		printf("2: %s\n", to_str(rholder._ref));                       \
		BUILD_ENUM(Result2, Ok, rholder);                              \
	})

#endif // _BASE_RESULT2__
