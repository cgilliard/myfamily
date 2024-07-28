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

CLASS(BufReader, FIELD(Rc, f))
IMPL(BufReader, TRAIT_READER)
IMPL(BufReader, TRAIT_BUF_READER)
#define BufReader DEFINE_CLASS(BufReader)

#define BufReaderFile(__fp__)                                                  \
	({                                                                     \
		Rc __rcfp__ = HEAPIFY(__fp__);                                 \
		BufReaderOption __optfp__ =                                    \
		    BUILD_ENUM(BufReaderOption, BUF_READER_FILE, __rcfp__);    \
		RcPtr __rcfp2__ = HEAPIFY(__optfp__);                          \
		__rcfp2__;                                                     \
	})

#define BufReaderInitialSize(x)                                                \
	({                                                                     \
		u64 __vb11_ = x;                                               \
		BufReaderOption __optb11_ = BUILD_ENUM(                        \
		    BufReaderOption, BUF_READER_INITIAL_BUF_SIZE, __vb11_);    \
		RcPtr __rcb11_ = HEAPIFY(__optb11_);                           \
		__rcb11_;                                                      \
	})

#define BufReaderMaximumSize(x)                                                \
	({                                                                     \
		u64 __vb12_ = x;                                               \
		BufReaderOption __optb12_ = BUILD_ENUM(                        \
		    BufReaderOption, BUF_READER_MAXIMUM_BUF_SIZE, __vb12_);    \
		RcPtr __rcb12_ = HEAPIFY(__optb12_);                           \
		__rcb12_;                                                      \
	})

#define PROC_BUF_READER_CONFIG(value) ({ value; })

// TODO: This should not only check counter is 0, but also check for an actual
// BufReaderFile argument
#define BUF_READER(...)                                                        \
	({                                                                     \
		int __counter___ = 0;                                          \
		EXPAND(FOR_EACH(COUNT_ARGS, __VA_ARGS__));                     \
		if (__counter___ == 0) {                                       \
			Error e = ERR(ILLEGAL_ARGUMENT,                        \
				      "BufReaderFile must be specified");      \
			return Err(e);                                         \
		}                                                              \
		Result __rr___ = BufReader_open_rc(                            \
		    __counter___ __VA_OPT__(, ) __VA_OPT__(EXPAND(             \
			FOR_EACH(PROC_BUF_READER_CONFIG, __VA_ARGS__))));      \
		BufReaderPtr __bptr__ = TRY(__rr___, __bptr__);                \
		__bptr__;                                                      \
	})

#endif // _CORE_BUF_READER__
