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

#ifndef _ERROR_BASE__
#define _ERROR_BASE__

#include <base/backtrace.h>
#include <base/class.h>
#include <base/traits.h>

#define MAX_ERROR_KIND_LEN 128
#define MAX_ERROR_MESSAGE_LEN 512

CLASS(ErrorKind, FIELD(char, kind[MAX_ERROR_KIND_LEN]))
IMPL(ErrorKind, TRAIT_COPY)
IMPL(ErrorKind, TRAIT_SIZE)
IMPL(ErrorKind, TRAIT_EQUAL)
#define ErrorKind DEFINE_CLASS(ErrorKind)
GETTER(ErrorKind, kind)

CLASS(Error, FIELD(ErrorKind, kind) FIELD(char, message[MAX_ERROR_MESSAGE_LEN])
		 FIELD(Backtrace, bt) FIELD(u64, flags))
IMPL(Error, TRAIT_COPY)
IMPL(Error, TRAIT_SIZE)
IMPL(Error, TRAIT_PRINT)
IMPL(Error, TRAIT_EQUAL)
#define Error DEFINE_CLASS(Error)
GETTER(Error, kind)
GETTER(Error, message)
GETTER(Error, bt)

Error error_build(ErrorKind kind, char *msg);
#define ERROR(kind, msg) error_build(kind, msg)

#endif // _ERROR_BASE__

