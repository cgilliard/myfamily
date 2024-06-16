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

#ifndef _BASE_ARGS__
#define _BASE_ARGS__

#include <base/class.h>
#include <base/traits.h>

#define MAX_SHORT_NAME_LEN 10

#define TRAIT_ARGS(T)                                                          \
	TRAIT_REQUIRED(T, bool, add_param, T##Ptr *args, const char *name,     \
		       const char *help, const char *short_name,               \
		       bool takes_value, bool multiple)                        \
	TRAIT_REQUIRED(T, bool, init, T##Ptr *args, int argc, char **argv)     \
	TRAIT_REQUIRED(T, bool, value, T##Ptr *args, char *buffer, size_t len, \
		       char *param, char *value)                               \
	TRAIT_REQUIRED(T, T##Ptr, build)

#define TRAIT_ARGS_PARAM(T)                                                    \
	TRAIT_REQUIRED(T, T##Ptr, build, const char *name, const char *help,   \
		       const char *short_name, bool takes_value,               \
		       bool multiple)

CLASS(ArgsParam,
      FIELD(char *, name) FIELD(char *, help) FIELD(char *, short_name)
	  FIELD(bool, takes_value) FIELD(bool, multiple) FIELD(u64, argv_itt)
	      FIELD(bool, specified));
IMPL(ArgsParam, TRAIT_COPY)
IMPL(ArgsParam, TRAIT_EQUAL)
IMPL(ArgsParam, TRAIT_ARGS_PARAM)
#define ArgsParam DEFINE_CLASS(ArgsParam)

CLASS(Args, FIELD(char **, argv) FIELD(int, argc) FIELD(ArgsParamPtr *, params)
		FIELD(u64, count))
IMPL(Args, TRAIT_ARGS)
#define Args DEFINE_CLASS(Args)

#endif // _BASE_ARGS__
