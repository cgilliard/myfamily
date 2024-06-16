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
#include <base/tlmalloc.h>

SETTER(ArgsParam, name)
SETTER(ArgsParam, help)
SETTER(ArgsParam, short_name)
SETTER(ArgsParam, takes_value)
SETTER(ArgsParam, multiple)
SETTER(ArgsParam, argv_itt);
GETTER(ArgsParam, name)
GETTER(ArgsParam, help)
GETTER(ArgsParam, short_name)
GETTER(ArgsParam, takes_value)
GETTER(ArgsParam, multiple)
GETTER(ArgsParam, argv_itt)
GETTER(ArgsParam, specified)
SETTER(ArgsParam, specified)

GETTER(Args, count)
GETTER(Args, params)
SETTER(Args, count)
SETTER(Args, params)
SETTER(Args, argc)
SETTER(Args, argv)
GETTER(Args, argv)
GETTER(Args, argc)

void ArgsParam_cleanup(ArgsParamPtr *ptr) {
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

ArgsParam ArgsParam_build(const char *name, const char *help,
			  const char *short_name, bool takes_value,
			  bool multiple) {
	ArgsParamPtr ret = BUILD(ArgsParam, NULL, NULL, NULL, false, false);
	char *name_copy = NULL;

	if (name) {
		name_copy = tlmalloc(sizeof(char) * (1 + strlen(name)));
		if (!name_copy)
			panic("Could not allocate memory to build params");
		strcpy(name_copy, name);
	}

	char *help_copy = NULL;

	if (help) {
		help_copy = tlmalloc(sizeof(char) * (1 + strlen(help)));
		if (!help_copy)
			panic("Could not allocate memory to build params");
		strcpy(help_copy, help);
	}

	char *short_name_copy = NULL;

	if (short_name) {
		short_name_copy =
		    tlmalloc(sizeof(char) * (1 + strlen(short_name)));
		if (!short_name_copy)
			panic("Could not allocate memory to build params");
		strcpy(short_name_copy, short_name);
	}

	ArgsParam_set_name(&ret, name_copy);
	ArgsParam_set_help(&ret, help_copy);
	ArgsParam_set_short_name(&ret, short_name_copy);
	ArgsParam_set_takes_value(&ret, takes_value);
	ArgsParam_set_multiple(&ret, multiple);
	ArgsParam_set_argv_itt(&ret, 0);
	ArgsParam_set_specified(&ret, false);
	return ret;
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

bool ArgsParam_copy(ArgsParam *dst, ArgsParam *src) {
	*dst = ArgsParam_build(
	    *ArgsParam_get_name(src), *ArgsParam_get_help(src),
	    *ArgsParam_get_short_name(src), *ArgsParam_get_takes_value(src),
	    *ArgsParam_get_multiple(src));

	if (!equal(src, dst))
		return false;
	return true;
}

size_t ArgsParam_size(ArgsParam *obj) { return sizeof(ArgsParam); }

void Args_cleanup(ArgsPtr *ptr) {
	u64 count = *(u64 *)Args_get_count(ptr);
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(ptr);
	for (u64 i = 0; i < count; i++) {
		char *ptrx = *((char **)ArgsParam_get_name(&params[i]));
		ArgsParam_cleanup(&params[i]);
	}

	if (count) {
		void *p = *Args_get_params(ptr);
		tlfree(p);
	}

	char **argv = *(char ***)Args_get_argv(ptr);
	int argc = *Args_get_argc(ptr);
	if (argv != NULL) {
		for (int i = 0; i < argc; i++) {
			tlfree(argv[i]);
		}

		tlfree(argv);
		Args_set_argv(ptr, NULL);
	}
}

Args Args_build() {
	ArgsPtr ret;
	Args_set_count(&ret, 0);
	Args_set_params(&ret, NULL);
	Args_set_argc(&ret, 0);
	Args_set_argv(&ret, NULL);
	return ret;
}

bool Args_add_param(ArgsPtr *ptr, const char *name, const char *help,
		    const char *short_name, bool takes_value, bool multiple) {

	if (strlen(short_name) > MAX_SHORT_NAME_LEN) {
		printf("Illegal short name (must be at most %i characters "
		       "long): %s",
		       MAX_SHORT_NAME_LEN, short_name);
		return false;
	}
	bool ret = true;
	u64 count = *(u64 *)Args_get_count(ptr);
	void *nptr;
	if (count == 0) {
		nptr = tlmalloc(sizeof(ArgsParam));
		Args_set_params(ptr, nptr);
	} else {
		void *p = *Args_get_params(ptr);
		nptr = tlrealloc(p, sizeof(ArgsParam) * (count + 1));
		if (nptr == NULL) {
			panic("Could not allocate memory to build params");
		} else {
			Args_set_params(ptr, nptr);
		}
	}

	if (ret) {
		((ArgsParam *)nptr)[count] = ArgsParam_build(
		    name, help, short_name, takes_value, multiple);
	}
	if (ret)
		Args_set_count(ptr, count + 1);

	return ret;
}

bool find_param(Args *args, char *name, bool *is_takes_value, bool *is_multi,
		bool *already_specified) {
	if (name[0] != '-' || strlen(name) < 2)
		return false;

	bool ret = false;
	u64 count = *Args_get_count(args);
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(args);

	for (u64 i = 0; i < count; i++) {
		char *pname = *ArgsParam_get_name(&params[i]);
		char *short_name = *ArgsParam_get_short_name(&params[i]);
		bool specified = *ArgsParam_get_specified(&params[i]);

		if (pname && name[1] == '-' || short_name && name[1] != '-') {
			int len;
			if (name[1] == '-')
				len = strlen(pname);
			else
				len = strlen(short_name);
			char pname_updated[len + 3];
			if (name[1] == '-') {
				strcpy(pname_updated, "--");
				strcat(pname_updated, pname);
			} else {
				strcpy(pname_updated, "-");
				strcat(pname_updated, short_name);
			}
			if (!strcmp(name, pname_updated)) {
				*is_takes_value =
				    *ArgsParam_get_takes_value(&params[i]);
				*is_multi = *ArgsParam_get_multiple(&params[i]);
				*already_specified = specified;

				ArgsParam_set_specified(&params[i], true);

				ret = true;
				break;
			}
		}
	}

	return ret;
}

bool Args_init(Args *args, int argc, char **argv) {
	bool ret = true;
	char **argv_copy = tlmalloc(sizeof(char *) * argc);
	u64 i = 0;
	for (i = 0; i < argc; i++) {
		argv_copy[i] = tlmalloc(sizeof(char) * strlen(argv[i]));
		if (!argv_copy[i]) {
			panic("Could not allocate memory to build params");
		}
		strcpy(argv_copy[i], argv[i]);
	}

	Args_set_argv(args, argv_copy);
	Args_set_argc(args, argc);

	// set specified to false for all
	u64 count = *(u64 *)Args_get_count(args);
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(args);
	for (u64 i = 0; i < count; i++) {
		ArgsParam_set_specified(&params[i], false);
	}

	for (u64 i = 0; i < argc; i++) {
		bool is_takes_value, is_multi, already_specified;
		if (!find_param(args, argv_copy[i], &is_takes_value, &is_multi,
				&already_specified)) {
			printf("Error: Unknown parameter: %s\n", argv_copy[i]);
			ret = false;
			break;
		}

		if (already_specified) {
			printf(
			    "Error: Parameter specified more than once: %s\n",
			    argv_copy[i]);
			ret = false;
			break;
		}

		if (is_multi && !is_takes_value) {
			printf("Error: Parameter cannot be multi and and not "
			       "takes_value: %s\n",
			       argv_copy[i]);
			ret = false;
			break;
		}

		if (is_takes_value) {
			i += 1;
			if (i >= argc) {
				printf("Error: Expected a value for: %s\n",
				       argv_copy[i - 1]);
				ret = false;
				break;
			}
			if (argv_copy[i][0] == '-') {
				printf("Error: Expected a value for: %s\n",
				       argv_copy[i - 1]);
				ret = false;
				break;
			}
		}

		if (is_multi) {
			while (true) {
				if (i >= argc)
					break;
				if (strlen(argv_copy[i]) > 1 &&
				    argv_copy[i][0] == '-') {
					i--;
					break;
				}
				i++;
			}
		}
	}

	return ret;
}
bool Args_value(Args *args, char *buffer, size_t len, char *param,
		char *value) {
	char param_buffer[strlen(param) + 3];
	char param_buffer_short[MAX_SHORT_NAME_LEN + 3];
	strcpy(param_buffer, "--");
	strcat(param_buffer, param);
	strcpy(param_buffer_short, "-");
	bool ret = false;
	int argc = *Args_get_argc(args);
	char **argv = *Args_get_argv(args);

	u64 count = *(u64 *)Args_get_count(args);
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(args);
	bool takes_value = false;
	bool multi = false;
	u64 match_index = 0;
	for (u64 i = 0; i < count; i++) {
		char *name = *ArgsParam_get_name(&params[i]);
		if (!strcmp(name, param)) {
			char *short_name =
			    *ArgsParam_get_short_name(&params[i]);
			takes_value = *ArgsParam_get_takes_value(&params[i]);
			multi = *ArgsParam_get_multiple(&params[i]);
			strcat(param_buffer_short, short_name);
			match_index = i;
		}
	}

	u64 i;
	for (i = 0; i < argc; i++) {
		if (!strcmp(param_buffer, argv[i])) {
			// long match
			ret = true;
			break;
		}
		if (!strcmp(param_buffer_short, argv[i])) {
			// short match
			ret = true;
			break;
		}
	}

	if (ret) {
		i++;
		if (multi) {
			u64 itt = *ArgsParam_get_argv_itt(&params[match_index]);
			i += itt;
			ArgsParam_set_argv_itt(&params[match_index], itt + 1);
			if (i < argc && argv[i][0] != '-')
				strncpy(buffer, argv[i], len);
			else {
				ret = false;
			}
		} else if (takes_value && i < argc) {
			strncpy(buffer, argv[i], len);
		}
	}

	if (!ret && value != NULL) {
		strncpy(buffer, value, len);
	}

	return ret;
}
