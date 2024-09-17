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

#include <base/types.h>
#include <stddef.h>
#include <stdio.h>

char *rstrstr(const char *s1, const char *s2);
int copy_file(const char *dst, const char *src);
int remove_directory(const char *path, bool preserve_dir);
size_t read_all(void *buffer, size_t size, size_t count, FILE *stream);
void exit_error(char *format, ...);

#endif // _BASE_MISC__
