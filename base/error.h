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

#ifndef _BASE_ERROR__
#define _BASE_ERROR__

#include <base/backtrace.h>
#include <base/class.h>
#include <base/traits_base.h>

#define MAX_ERROR_KIND_LEN 128
#define MAX_ERROR_MESSAGE_LEN 512

CLASS(ErrorKind, FIELD(char, kind[MAX_ERROR_KIND_LEN]))
IMPL(ErrorKind, TRAIT_COPY)
IMPL(ErrorKind, TRAIT_EQUAL)
#define ErrorKind DEFINE_CLASS(ErrorKind)
static GETTER(ErrorKind, kind);

CLASS(Error, FIELD(ErrorKind, kind) FIELD(char, message[MAX_ERROR_MESSAGE_LEN])
		 FIELD(Backtrace, bt) FIELD(u64, flags))
IMPL(Error, TRAIT_COPY)
IMPL(Error, TRAIT_PRINT)
IMPL(Error, TRAIT_EQUAL)
#define Error DEFINE_CLASS(Error)

static GETTER(Error, kind);
static GETTER(Error, message);
static GETTER(Error, bt);

Error error_build(ErrorKind kind, char *fmt, ...);
#define ERROR(kind, ...) error_build(kind, ##__VA_ARGS__)
#define EKIND(kind) BUILD(ErrorKind, kind)
#define KIND(e) Error_get_kind(&e)

#endif // _BASE_ERROR__
