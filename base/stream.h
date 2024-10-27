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

#ifndef _BASE_STREAM__
#define _BASE_STREAM__

#include <base/types.h>

typedef struct Stream {
	int handle;
	void *strm;
} Stream;

const static Stream in_strm_impl = {.handle = 0, .strm = NULL};
const static Stream out_strm_impl = {.handle = 1, .strm = NULL};
const static Stream err_strm_impl = {.handle = 2, .strm = NULL};
const static Stream *out_strm = &out_strm_impl;
const static Stream *in_strm = &in_strm_impl;
const static Stream *err_strm = &err_strm_impl;

ch *sgets(ch *buf, num limit, Stream *strm);

#endif // _BASE_STREAM__
