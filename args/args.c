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

#include <args/args.h>
#include <base/resources.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void args_param_cleanup(ArgsParamImpl *ptr) {
	if (ptr->help) {
		myfree(ptr->help);
		ptr->help = NULL;
	}
	if (ptr->default_value) {
		myfree(ptr->default_value);
		ptr->default_value = NULL;
	}
}

int args_param_build(ArgsParam *ptr, const char *name, const char *help, const char *short_name,
					 const bool takes_value, const bool multiple, const char *default_value) {
	if (ptr == NULL || name == NULL || help == NULL || short_name == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (strlen(name) > ARGS_MAX_ARGUMENT_NAME_LENGTH ||
		strlen(short_name) > ARGS_MAX_ARGUMENT_NAME_LENGTH) {
		errno = EINVAL;
		return -1;
	}

	u64 help_len = strlen(help);
	u64 default_value_len = 0;
	if (default_value)
		default_value_len = strlen(default_value);

	ptr->help = mymalloc(sizeof(char) * (help_len + 1));
	if (default_value)
		ptr->default_value = mymalloc(sizeof(char) * (default_value_len + 1));

	if (ptr->help == NULL || (ptr->default_value == NULL && default_value)) {
		args_param_cleanup(ptr);
		return -1;
	}

	strcpy(ptr->name, name);
	strcpy(ptr->short_name, short_name);
	strcpy(ptr->help, help);
	if (default_value)
		strcpy(ptr->default_value, default_value);
	else
		ptr->default_value = NULL;
	ptr->takes_value = takes_value;
	ptr->multiple = multiple;

	return 0;
}
