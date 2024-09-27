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

#ifndef _BASE_MISC__
#define _BASE_MISC__

#include <base/path.h>
#include <base/resources.h>
#include <base/types.h>

char *rstrstr(const char *s1, const char *s2);
int copy_file(const Path *dst, const Path *src);
int remove_directory(const Path *path, bool preserve_dir);
u64 read_all(void *buffer, u64 size, u64 count, MYFILE *stream);
void exit_error(char *format, ...);

#endif // _BASE_MISC__
