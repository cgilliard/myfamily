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

#ifndef _BASE_ERR__
#define _BASE_ERR__

#include <base/macro_util.h>
#include <base/types.h>

#define ERR_LEN 64

#define DEFINE_ERR(e)                                               \
	typedef enum FamErr { FOR_EACH(SECOND, none, (, ), e) } FamErr; \
	static const byte *FamErrText[] = {                             \
		FOR_EACH(SECOND_STRINGIFY, none, (, ), e)};

// Define FamErr enum values
// clang-format off
#define ERR_VALUES                                                                              \
	NoErrors, \
	IllegalArgument, \
	Overflow, \
	Underflow, \
	LockInitErr, \
	InitErr, \
	AlreadyInitialized, \
	IndexOutOfBounds, \
	POpenErr, \
	IO, \
	Busy, \
	CapacityExceeded, \
	FileNotFound, \
	NotADirectory, \
	UnsupportedOperation, \
	UnsupportedVersion, \
	FileCorrupted, \
	IllegalState, \
	TooBig, \
	ResourceNotAvailable, \
	NoStream, \
	Permission, \
	BacktraceErr, \
	ExpectedTypeMismatch, \
	OutOfSpace, \
	MaxPathErr, \
	AllocErr, \
	TypeMismatch, \
	ObjectConsumed, \
	DuplicateSizes, \
	NotYetImplemented
// clang-format on

DEFINE_ERR(ERR_VALUES);

extern _Thread_local byte err_last[ERR_LEN + 1];
extern _Thread_local int64 err;

void print_err(const byte *text);
const byte *get_err();

#define SetErr(err_value) ({ err = err_value; })

#endif	// _BASE_ERR__