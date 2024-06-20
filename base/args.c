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

#include <base/args.h>

void ArgsParamState_cleanup(ArgsParamState *ptr) {}

Result ArgsParam_build(const char *name, const char *help,
		       const char *short_name, bool takes_value,
		       bool multiple) {
	todo()
}
void ArgsParam_cleanup(ArgsParam *ptr) {}
bool ArgsParam_copy(ArgsParam *dst, ArgsParam *src) { return false; }
bool ArgsParam_equal(ArgsParam *obj1, ArgsParam *obj2) { return false; }
size_t ArgsParam_size(ArgsParam *ptr) { return sizeof(ArgsParam); }

void SubCommand_cleanup(SubCommand *ptr) {}
bool SubCommand_copy(SubCommand *dst, SubCommand *src) { return false; }
size_t SubCommand_size(SubCommand *ptr) { return sizeof(SubCommand); }
Result SubCommand_build(char *name, u32 min_args, u32 max_args) { todo() }

void Args_cleanup(Args *ptr) {}
Result Args_add_param(Args *ptr, const char *name, const char *help,
		      const char *short_name, bool takes_value, bool multiple) {
	todo()
}
void Args_add_sub(Args *ptr, SubCommand *sub) {}

Option Args_argument(Args *ptr, u64 index) { return None; }

Result Args_build(char *prog, char *version, char *author){todo()}

Result Args_init(Args *ptr, int argc, char **argv, u64 flags) {
	todo()
}

void Args_usage(Args *ptr) {}

Result Args_value(Args *ptr, char *buffer, size_t len, char *param,
		  char *value) {
	todo()
}

