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

#ifndef _BASE_FORMATTER__
#define _BASE_FORMATTER__

#include <base/class.h>
#include <base/traits.h>

CLASS(Formatter, FIELD(char *, buf) FIELD(u64, len) FIELD(u64, pos))
IMPL(Formatter, TRAIT_FORMATTER)
IMPL(Formatter, TRAIT_TO_STR_REF)
#define Formatter DEFINE_CLASS(Formatter)

#define WRITE(f, ...) Formatter_write(f, __VA_ARGS__)

#endif // _BASE_FORMATTER__
