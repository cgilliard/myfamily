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

#ifndef _UTIL_REPLACE__
#define _UTIL_REPLACE__

#include <base/path.h>

int replace_file(Path *in_path, Path *out_path, const char *patterns_in[],
	const bool is_case_sensisitive[], const char *replace[], int count);

#endif // _UTIL_REPLACE__
