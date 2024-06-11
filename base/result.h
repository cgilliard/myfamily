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

#ifndef _RESULT_BASE__
#define _RESULT_BASE__

#include <base/error.h>
#include <base/vtable.h>

// Result

// typedef
typedef struct {
	Vtable *vtable;
	bool (*is_ok)();
	Error *err;
	void *ref;
} ResultPtr;

// cleanup
void result_free(ResultPtr *ptr);
#define Result ResultPtr CLEANUP(result_free)

void *result_unwrap(Result *result);
Error *result_unwrap_err(Result *result);

// vtable
static Vtable ResultVtable = {
    2, 0, {{"unwrap", result_unwrap}, {"unwrap_err", result_unwrap_err}}};

// builders
Result result_build_ok(void *ref);
Result result_build_err(Error err);

// macros
#define Ok(x) result_build_ok(&x)
#define Err(x) result_build_err(x)

#endif // _RESULT_BASE__
