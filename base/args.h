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
#include <base/option.h>
#include <base/result.h>
#include <base/traits.h>

#define TRAIT_ARGS_PARAM(T)                                                    \
	TRAIT_REQUIRED(T, Result, build, char *name, char *help,               \
		       char *short_name, bool takes_value, bool multiple)

#define TRAIT_SUB_COMMAND(T)                                                   \
	TRAIT_REQUIRED(T, Result, build, char *name, u32 min_args,             \
		       u32 max_args)                                           \
	TRAIT_REQUIRED(T, Result, add_param, T##Ptr *sub, ArgsParam *param)

#define TRAIT_ARGS(T)                                                          \
	TRAIT_REQUIRED(T, Result, add_param, T##Ptr *args, char *name,         \
		       char *help, char *short_name, bool takes_value,         \
		       bool multiple)                                          \
	TRAIT_REQUIRED(T, Result, init, T##Ptr *args, int argc, char **argv,   \
		       u64 flags)                                              \
	TRAIT_REQUIRED(T, Result, value, T##Ptr *args, char *buffer,           \
		       usize len, char *param, char *value)                   \
	TRAIT_REQUIRED(T, Result, build, char *prog, char *version,            \
		       char *author)                                           \
	TRAIT_REQUIRED(T, void, usage, T##Ptr *ptr)                            \
	TRAIT_REQUIRED(T, Result, add_sub, T##Ptr *ptr, SubCommand *sub)       \
	TRAIT_REQUIRED(T, Option, argument, T##Ptr *ptr, u64 index)            \
	TRAIT_REQUIRED(T, Result, print_version, T##Ptr *args)

CLASS(ArgsParamState, FIELD(bool, specified) FIELD(u64, itt))
IMPL(ArgsParamState, TRAIT_COPY)
#define ArgsParamState DEFINE_CLASS(ArgsParamState)

CLASS(ArgsParam,
      FIELD(char *, name) FIELD(char *, help) FIELD(char *, short_name)
	  FIELD(bool, takes_value) FIELD(bool, multiple))
IMPL(ArgsParam, TRAIT_COPY)
IMPL(ArgsParam, TRAIT_EQUAL)
IMPL(ArgsParam, TRAIT_ARGS_PARAM)
#define ArgsParam DEFINE_CLASS(ArgsParam)

CLASS(SubCommand, FIELD(char *, name) FIELD(ArgsParam *, params)
		      FIELD(ArgsParamState *, params_state) FIELD(u64, count)
			  FIELD(u32, min_add_args) FIELD(u32, max_add_args))
IMPL(SubCommand, TRAIT_COPY)
IMPL(SubCommand, TRAIT_SUB_COMMAND)
#define SubCommand DEFINE_CLASS(SubCommand)

CLASS(Args,
      FIELD(char *, prog) FIELD(char *, version) FIELD(char *, author)
	  FIELD(SubCommand *, subs) FIELD(u64, subs_count) FIELD(char **, argv)
	      FIELD(int, argc) FIELD(u64, debug_flags))
IMPL(Args, TRAIT_ARGS);
IMPL(Args, TRAIT_COPY);
#define Args DEFINE_CLASS(Args)

#endif // _BASE_ARGS__
