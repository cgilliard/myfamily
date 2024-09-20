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

#ifndef _BUILD_PARSER__
#define _BUILD_PARSER__

#include <base/path.h>
#include <build/build.h>
#include <limits.h>
#include <util/vec.h>

void parse_header(const char *config_dir, const char *base_dir, Vec *modules, Vec *types,
	const ModuleInfo *self_info);

#endif // _BUILD_PARSER__
