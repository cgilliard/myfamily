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

#ifndef _CORE_BUF_READER__
#define _CORE_BUF_READER__

#include <core/enum.h>
#include <core/file.h>
#include <core/io.h>

ENUM(BufReaderOption,
     VARIANTS(BUF_READER_FILE, BUF_READER_INITIAL_BUF_SIZE,
	      BUF_READER_MAXIMUM_BUF_SIZE),
     TYPES("Rc", "u64", "u64"))
#define BufReaderOption DEFINE_ENUM(BufReaderOption)

#define TRAIT_BUF_READER(T)                                                    \
	TRAIT_REQUIRED(T, Result, open, int n, ...)                            \
	TRAIT_REQUIRED(T, Result, open_rc, int n, ...)

CLASS(BufReader, FIELD(File, f))
IMPL(BufReader, TRAIT_READER)
IMPL(BufReader, TRAIT_BUF_READER)
#define BufReader DEFINE_CLASS(BufReader)

#define BufReaderFile(x)                                                       \
	({                                                                     \
		FilePtr __fp__ = FOPEN(x, OpenRead);                           \
		RcPtr __rcfp__ = HEAPIFY(__fp__);                              \
		BufReaderOptionPtr __optfp__ =                                 \
		    BUILD_ENUM(BufReaderOption, BUF_READER_FILE, __rcfp__);    \
		RcPtr __rcfp2__ = HEAPIFY(__optfp__);                          \
		printf("class=%s\n", CLASS_NAME(&__rcfp2__));                  \
		__rcfp2__;                                                     \
	})

#endif // _CORE_BUF_READER__
