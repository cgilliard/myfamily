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

#ifndef _BASE_PANIC__
#define _BASE_PANIC__

#include <base/misc.h>
#include <base/print_util.h>
#include <base/types.h>

#define panic(fmt, ...)                                                                            \
	__do_print_impl_(err_strm, NULL, UINT32_MAX, true, true, -1, "Panic: ", fmt, __VA_ARGS__)

#endif // _BASE_PANIC__
