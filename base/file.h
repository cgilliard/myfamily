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

#ifndef _BASE_FILE__
#define _BASE_FILE__

#include <base/class.h>
#include <base/io.h>

typedef enum OpenOptions {
	OpenRead = 0,
	OpenWrite = 1,
	OpenAppend = 2,
	OpenReadExtended = 3,
	OpenWriteExtended = 4,
	OpenAppendExtended = 5
} OpenOptions;

#define TRAIT_FILE_OPEN(T)                                                     \
	TRAIT_REQUIRED(T, Result, open, char *path, OpenOptions opt)

CLASS(File, FIELD(FILE *, fp))
IMPL(File, TRAIT_READ)
IMPL(File, TRAIT_FILE_OPEN)
IMPL(File, TRAIT_SEEK)
IMPL(File, TRAIT_SIZE)
#define File DEFINE_CLASS(File)

#define FOPEN(path, options) File_open(path, options)

#endif // _BASE_FILE__
