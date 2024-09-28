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

#ifndef _ARGS_ARGS__
#define _ARGS_ARGS__

#include <base/types.h>

#define ARGS_MAX_ARGUMENT_NAME_LENGTH 128
#define ARGS_MAX_SUBCOMMAND_LENGTH 128
#define ARGS_MAX_DETAIL_LENGTH 256

typedef struct ArgsParamImpl {
	char name[ARGS_MAX_ARGUMENT_NAME_LENGTH + 1];
	char *help;
	char short_name[ARGS_MAX_ARGUMENT_NAME_LENGTH + 1];
	bool takes_value;
	bool multiple;
	char *default_value;
} ArgsParamImpl;

void args_param_cleanup(ArgsParamImpl *ptr);

#define ArgsParam ArgsParamImpl __attribute__((warn_unused_result, cleanup(args_param_cleanup)))

int args_param_build(ArgsParam *ptr, const char *name, const char *help, const char *short_name,
					 const bool takes_value, const bool multiple, const char *default_value);

typedef struct SubCommandImpl {
	char name[ARGS_MAX_SUBCOMMAND_LENGTH + 1];
	ArgsParam *params;
	bool *is_specified;
	u32 param_count;
	u32 min_args;
	u32 max_args;
	char *help;
	char *arg_doc;
} SubCommandImpl;

void sub_command_cleanup(SubCommandImpl *ptr);

#define SubCommand SubCommandImpl __attribute__((warn_unused_result, cleanup(sub_command_cleanup)))

int sub_command_build(SubCommand *sc, const char *name, const char *help, const u32 min_args,
					  const u32 max_args, const char *arg_doc);
int sub_command_add_param(SubCommand *sc, const ArgsParam *ap);

typedef struct ArgsImpl {
	char prog[ARGS_MAX_DETAIL_LENGTH + 1];
	char version[ARGS_MAX_DETAIL_LENGTH + 1];
	char author[ARGS_MAX_DETAIL_LENGTH + 1];
	char arch[ARGS_MAX_DETAIL_LENGTH + 1];
	SubCommand *subs;
	u32 subs_count;
	char **argv;
	int argc;
} ArgsImpl;

void args_cleanup(ArgsImpl *ptr);

#define Args ArgsImpl __attribute__((warn_unused_result, cleanup(args_cleanup)))

int args_build(Args *args, const char *prog, const char *version, const char *author,
			   const u32 min_args, const u32 max_args, const char *arch);

int args_add_param(Args *args, const ArgsParam *ap);
int args_add_sub_command(Args *args, SubCommand *sc);
void args_init(Args *args, const int argc, const char **argv);
int args_value_of(const Args *args, const char *param_name, char *value_buf,
				  const u64 max_value_len, const u32 index);
int args_get_argument(const Args *args, const u32 index, char *value_buf, const u64 max_value_len);
void args_print_version(const Args *args);
void args_usage(const Args *args, const char *sub);

#endif // _ARGS_ARGS__
