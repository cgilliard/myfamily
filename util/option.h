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

#ifndef _UTIL_OPTION__
#define _UTIL_OPTION__

#include <base/cleanup.h>
#include <base/types.h>
#include <stddef.h>

bool option_is_some_false();
bool option_is_some_true();

typedef struct OptionImpl {
	bool (*is_some)();
	void *ref;
} OptionImpl;
void option_free(OptionImpl *ptr);
#define Option OptionImpl CLEANUP(option_free)

Option option_build(Option *opt, void *x, size_t size);
void *option_unwrap(Option x);

#define Some(opt, x) option_build(opt, &x, sizeof(x))
#define Unwrap(x) option_unwrap(x)

#endif	// _UTIL_OPTION__
