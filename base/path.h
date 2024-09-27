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

#include <base/types.h>

#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'

typedef struct PathImpl {
	void *ptr;
	u64 len;
} PathImpl;

void path_cleanup(PathImpl *ptr);

#define Path PathImpl __attribute__((warn_unused_result, cleanup(path_cleanup)))

int path_for(Path *dst, const char *path);
int path_canonicalize(Path *p);
int path_push(Path *p, const char *next);
int path_pop(Path *p);
char *path_to_string(const Path *p);
bool path_exists(const Path *p);
bool path_is_dir(const Path *p);
bool path_mkdir(Path *p, u64 mode, bool parent);
const char *path_file_name(const Path *p);
int path_file_stem(const Path *p, char *buf, u64 limit);
int path_copy(Path *dst, const Path *src);
u64 path_file_size(Path *p);

#ifdef TEST
extern bool __is_debug_path_homedir_null;
#endif // TEST

#endif // _BASE_PATH__
