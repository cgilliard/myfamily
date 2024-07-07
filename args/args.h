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

#define DEBUG_INIT_NO_EXIT 0x1

#include <core/resources.h>

typedef struct ArgsParamState {
	bool specified;
	u64 itt;
} ArgsParamState;

typedef struct ArgsParam {
	char *name;
	char *help;
	char *short_name;
	bool takes_value;
	bool multiple;
	char *default_value;
} ArgsParam;

typedef struct SubCommand {
	char *name;
	ArgsParam *params;
	ArgsParamState *params_state;
	u64 params_count;
	u32 min_args;
	u32 max_args;
	char *help;
} SubCommand;

typedef struct Args {
	char *prog;
	char *version;
	char *author;
	SubCommand **subs;
	u64 subs_count;
	char **argv;
	int argc;
	u64 debug_flags;
} Args;

int args_param_build(ArgsParam *ap, char *name, char *help, char *short_name,
		     bool takes_value, bool multiple, char *default_value);
void args_param_cleanup(ArgsParam *ap);
int sub_command_build(SubCommand *sc, char *name, char *help, u32 min_args,
		      u32 max_args);
int sub_command_add_param(SubCommand *sc, ArgsParam *ap);
void sub_command_cleanup(SubCommand *sc);
int args_build(Args *args, char *prog, char *version, char *author,
	       u32 min_args, u32 max_args, u64 debug_flags);
void args_cleanup(Args *args);
int args_add_param(Args *args, ArgsParam *ap);
int args_add_sub_command(Args *args, SubCommand *sc);
int args_init(Args *args, int argc, char **argv);
int args_value_of(Args *args, char *param_name, char *value_buf,
		  u64 max_value_len, u64 index);
int args_get_argument(Args *args, u64 index, char *value_buf,
		      u64 max_value_len);
void args_print_version(Args *args);
void args_usage(Args *args, char *sub);

#endif // _ARGS_ARGS__
