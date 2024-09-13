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

#ifndef _BASE_PATH__
#define _BASE_PATH__

#include <base/chain_allocator.h>

typedef struct PathImpl {
	FatPtr ptr;
} PathImpl;

void path_cleanup(PathImpl *ptr);

#define Path PathImpl __attribute__((warn_unused_result, cleanup(path_cleanup)))

int path_for(Path *dst, const char *path);
int path_canonicalize(Path *p);
int path_push(Path *p, const char *next);
int path_pop(Path *p);
char *path_to_string(Path *p);
bool path_exists(Path *p);
bool path_is_dir(Path *p);
bool path_mkdir(Path *p, mode_t mode);

#endif // _BASE_PATH__
