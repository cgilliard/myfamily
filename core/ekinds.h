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

#ifndef _CORE_EKINDS__
#define _CORE_EKINDS__

#include <core/error.h>

static ErrorKind ILLEGAL_ARGUMENT = EKIND("IllegalArgument");
static ErrorKind ILLEGAL_STATE = EKIND("IllegalState");
static ErrorKind COPY_NOT_IMPLEMENTED_ERROR = EKIND("CopyNotImplemented");
static ErrorKind UNWRAP_NONE = EKIND("UnwrapNone");
static ErrorKind FILE_NOT_FOUND_ERROR = EKIND("FileNotFound");
static ErrorKind IO_ERROR = EKIND("IOError");
static ErrorKind PARSE_ERROR = EKIND("ParseError");
static ErrorKind STRING_INDEX_OUT_OF_BOUNDS = EKIND("StringIndexOutOfBounds");
static ErrorKind WRITE_ERROR = EKIND("WriteError");
static ErrorKind OVERFLOW = EKIND("Overflow");
static ErrorKind UNEXPECTED_EOF = EKIND("UnexpecteEOF");
static ErrorKind FILE_OPEN_ERROR = EKIND("FileOpen");
static ErrorKind RAND_ERROR = EKIND("RandError");
static ErrorKind FILE_SEEK_ERROR = EKIND("FileSeek");
static ErrorKind PTHREAD_ERROR = EKIND("PthreadError");
static ErrorKind UNEXPECTED_CLASS_ERROR = EKIND("UnexpectedClass");
static ErrorKind UNIMPLEMENTED = EKIND("UNImplemented");

#endif // _CORE_EKINDS__
