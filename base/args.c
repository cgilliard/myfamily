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
#include <base/ekinds.h>

SETTER(ArgsParam, name)
SETTER(ArgsParam, help)
SETTER(ArgsParam, short_name)
SETTER(ArgsParam, takes_value)
SETTER(ArgsParam, multiple)
GETTER(ArgsParam, name)
GETTER(ArgsParam, help)
GETTER(ArgsParam, short_name)
GETTER(ArgsParam, takes_value)
GETTER(ArgsParam, multiple)

void ArgsParamState_cleanup(ArgsParamState *ptr) {}

ArgsParam ArgsParam_build_impl(char *name_copy, char *help_copy,
			       char *short_name_copy, bool takes_value,
			       bool multiple) {
	ArgsParamPtr ret = BUILD(ArgsParam, name_copy, help_copy,
				 short_name_copy, takes_value, multiple);
	return ret;
}

Result ArgsParam_build(char *name, char *help, char *short_name,
		       bool takes_value, bool multiple) {

	ArgsParamPtr src =
	    BUILD(ArgsParam, name, help, short_name, takes_value, multiple);
	ArgsParam ret;
	copy(&ret, &src);
	return Ok(ret);
}

void ArgsParam_cleanup(ArgsParam *ptr) {
	void *name = *ArgsParam_get_name(ptr);
	if (name) {
		tlfree(name);
		ArgsParam_set_name(ptr, NULL);
	}

	void *help = *ArgsParam_get_help(ptr);
	if (help) {
		tlfree(help);
		ArgsParam_set_help(ptr, NULL);
	}

	void *short_name = *ArgsParam_get_short_name(ptr);
	if (short_name) {
		tlfree(short_name);
		ArgsParam_set_short_name(ptr, NULL);
	}
}
bool ArgsParam_copy(ArgsParam *dst, ArgsParam *src) {
	bool ret = true;
	char *name_copy = NULL, *help_copy = NULL, *short_name_copy = NULL;

	char *name = *(char **)ArgsParam_get_name(src);
	char *help = *(char **)ArgsParam_get_help(src);
	char *short_name = *(char **)ArgsParam_get_short_name(src);
	bool takes_value = *(bool *)ArgsParam_get_takes_value(src);
	bool multiple = *(bool *)ArgsParam_get_multiple(src);
	if (name) {
		name_copy = tlmalloc(sizeof(char) * (1 + strlen(name)));
		if (!name_copy) {
			ret = false;
		} else {
			strcpy(name_copy, name);
		}
	}

	if (ret && help) {
		help_copy = tlmalloc(sizeof(char) * (1 + strlen(help)));
		if (!help_copy) {
			ret = false;
		} else {
			strcpy(help_copy, help);
		}
	}

	if (short_name) {
		short_name_copy =
		    tlmalloc(sizeof(char) * (1 + strlen(short_name)));
		if (!short_name_copy) {
			ret = false;
		} else {
			strcpy(short_name_copy, short_name);
		}
	}

	if (ret) {
		*dst =
		    ArgsParam_build_impl(name_copy, help_copy, short_name_copy,
					 takes_value, multiple);
		return true;
	} else {
		return false;
	}
}
bool ArgsParam_equal(ArgsParam *obj1, ArgsParam *obj2) {
	bool ret = true;

	char *name1 = *ArgsParam_get_name(obj1);
	char *name2 = *ArgsParam_get_name(obj2);

	if (name1 == NULL && name2 != NULL || name2 == NULL && name1 != NULL)
		ret = false;

	if (ret && name1 != NULL)
		ret = !strcmp(name1, name2);
	if (ret) {
		char *help1 = *ArgsParam_get_help(obj1);
		char *help2 = *ArgsParam_get_help(obj2);
		if (help1 == NULL && help2 != NULL ||
		    help2 == NULL && help1 != NULL)
			ret = false;
		if (ret && help1 != NULL)
			ret = !strcmp(help1, help2);
	}

	if (ret) {
		char *short_name1 = *ArgsParam_get_short_name(obj1);
		char *short_name2 = *ArgsParam_get_short_name(obj2);
		if (short_name1 == NULL && short_name2 != NULL ||
		    short_name2 == NULL && short_name1 != NULL)
			ret = false;
		if (ret && short_name1 != NULL)
			ret = !strcmp(short_name1, short_name2);
	}

	if (ret)
		ret = *ArgsParam_get_takes_value(obj1) ==
		      *ArgsParam_get_takes_value(obj2);
	if (ret)
		ret = *ArgsParam_get_multiple(obj1) ==
		      *ArgsParam_get_multiple(obj2);

	return ret;
}

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
