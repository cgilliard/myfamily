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
#include <base/colors.h>
#include <base/tlmalloc.h>
#include <stdlib.h>

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

GETTER(Args, sub)
SETTER(Args, sub)
GETTER(Args, sub_count)
SETTER(Args, sub_count)
GETTER(Args, count)
GETTER(Args, params)
SETTER(Args, count)
SETTER(Args, params)
SETTER(Args, argc)
SETTER(Args, argv)
GETTER(Args, argv)
GETTER(Args, argc)
SETTER(Args, prog);
SETTER(Args, author);
SETTER(Args, version);
GETTER(Args, prog);
GETTER(Args, author);
GETTER(Args, version);
GETTER(Args, debug_flags);
SETTER(Args, debug_flags);
GETTER(Args, arg_itt);
SETTER(Args, arg_itt);
GETTER(Args, sub_index);
SETTER(Args, sub_index);

GETTER(SubCommand, name);
SETTER(SubCommand, name);
GETTER(SubCommand, params);
SETTER(SubCommand, params);
GETTER(SubCommand, count);
SETTER(SubCommand, count);
GETTER(SubCommand, min_add_args);
SETTER(SubCommand, min_add_args);
GETTER(SubCommand, max_add_args);
SETTER(SubCommand, max_add_args);

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

void SubCommand_cleanup(SubCommand *ptr) {
	char *name = *SubCommand_get_name(ptr);
	if (name) {
		tlfree(name);
		SubCommand_set_name(ptr, NULL);
	}
}

SubCommand SubCommand_build(char *name, u32 min_args, u32 max_args) {
	char *name_copy = tlmalloc(sizeof(char) * (strlen(name) + 1));
	strcpy(name_copy, name);
	SubCommandPtr ret =
	    BUILD(SubCommand, name_copy, NULL, 0, min_args, max_args);
	return ret;
}

bool SubCommand_copy(SubCommand *dst, SubCommand *src) {
	char *name = *SubCommand_get_name(src);
	u32 min_args = *SubCommand_get_min_add_args(src);
	u32 max_args = *SubCommand_get_max_add_args(src);
	*dst = SubCommand_build(name, min_args, max_args);
	ArgsParamPtr *params = *SubCommand_get_params(src);
	u64 count = *SubCommand_get_count(src);
	if (count) {
		ArgsParamPtr *nparams = tlmalloc(sizeof(ArgsParam) * count);
		for (u64 i = 0; i < count; i++) {
			copy(&nparams[i], &params[i]);
		}
		SubCommand_set_params(dst, nparams);
	} else {
		SubCommand_set_params(dst, NULL);
	}
	SubCommand_set_count(dst, count);
	return true;
}

size_t SubCommand_size(SubCommand *ptr) { return sizeof(SubCommand); }

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
	Args_set_count(ptr, 0);

	char **argv = *(char ***)Args_get_argv(ptr);
	int argc = *Args_get_argc(ptr);
	if (argv != NULL) {
		for (int i = 0; i < argc; i++) {
			tlfree(argv[i]);
		}

		tlfree(argv);
		Args_set_argv(ptr, NULL);
	}

	void *prog = *Args_get_prog(ptr);
	if (prog) {
		tlfree(prog);
		Args_set_prog(ptr, NULL);
	}

	void *version = *Args_get_version(ptr);
	if (version) {
		tlfree(version);
		Args_set_version(ptr, NULL);
	}

	void *author = *Args_get_author(ptr);
	if (author) {
		tlfree(author);
		Args_set_author(ptr, NULL);
	}

	SubCommandPtr *sub = *Args_get_sub(ptr);
	u64 sub_count = *Args_get_sub_count(ptr);
	if (sub) {
		for (u64 i = 0; i < sub_count; i++) {
			cleanup(&sub[i]);
		}
		tlfree(sub);
		Args_set_sub(ptr, NULL);
	}
}

Args Args_build(char *prog, char *version, char *author) {
	ArgsPtr ret =
	    BUILD(Args, NULL, 0, NULL, 0, NULL, NULL, NULL, 0, NULL, 0, 0);
	Args_set_count(&ret, 0);
	Args_set_params(&ret, NULL);
	Args_set_argc(&ret, 0);
	Args_set_argv(&ret, NULL);

	char *prog_copy;
	if (prog == NULL) {
		prog_copy = tlmalloc(sizeof(char));
		prog_copy[0] = 0;
	} else {
		prog_copy = tlmalloc(sizeof(char) * (1 + strlen(prog)));
		strcpy(prog_copy, prog);
	}

	char *version_copy;
	if (version == NULL) {
		version_copy = tlmalloc(sizeof(char));
		version_copy[0] = 0;
	} else {
		version_copy = tlmalloc(sizeof(char) * (1 + strlen(version)));
		strcpy(version_copy, version);
	}

	char *author_copy;
	if (author == NULL) {
		author_copy = tlmalloc(sizeof(char));
		author_copy[0] = 0;
	} else {
		author_copy = tlmalloc(sizeof(char) * (1 + strlen(author)));
		strcpy(author_copy, author);
	}

	Args_set_prog(&ret, prog_copy);
	Args_set_author(&ret, author_copy);
	Args_set_version(&ret, version_copy);

	return ret;
}

bool Args_add_param(ArgsPtr *ptr, const char *name, const char *help,
		    const char *short_name, bool takes_value, bool multiple) {

	if (strlen(short_name) > MAX_SHORT_NAME_LEN) {
		panic("Illegal short name (must be at most %i characters "
		      "long): %s",
		      MAX_SHORT_NAME_LEN, short_name);
	}

	if (!strcmp(short_name, "h")) {
		panic("short_name 'h' is reserved.");
	}
	if (!strcmp(short_name, "V")) {
		panic("short_name 'V' is reserved.");
	}

	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(ptr);
	u64 count = *(u64 *)Args_get_count(ptr);
	for (u64 i = 0; i < count; i++) {
		char *pname = *ArgsParam_get_name(&params[i]);
		char *pshort_name = *ArgsParam_get_short_name(&params[i]);
		if (!strcmp(name, pname)) {
			panic("duplicate name detected: %s", name);
		}
		if (!strcmp(short_name, pshort_name)) {
			panic("duplicate short_name detected: %s\n",
			      short_name);
		}
	}

	bool ret = true;
	void *nptr;
	if (count == 0) {
		nptr = tlmalloc(sizeof(ArgsParam));
		if (nptr == NULL)
			panic("Could not allocate memory to build params");
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

void Args_sub(Args *args, SubCommand *sub) {
	SubCommandPtr *cur = *Args_get_sub(args);
	u64 cur_count = *Args_get_sub_count(args);
	if (!cur) {
		// initial tlmalloc
		cur = tlmalloc(sizeof(SubCommand));
		if (cur == NULL)
			panic("Could not allocate memory for sub commands");
		Args_set_sub(args, cur);
		Args_set_sub_count(args, 1 + cur_count);
	} else {
		// tlrealloc (panic if it fails so we can overwrite the pointer)
		cur = tlrealloc(cur, sizeof(SubCommand) * (1 + cur_count));
		if (cur == NULL)
			panic("Could not allocate memory for sub commands");
		Args_set_sub(args, cur);
		Args_set_sub_count(args, 1 + cur_count);
	}
	copy(&cur[cur_count], sub);
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

void args_error_exit(Args *args, char *format, ...) {
	char *prog = *Args_get_prog(args);
	va_list va_args;
	va_start(va_args, format);
	fprintf(stderr, "%sError%s: ", RED, RESET);
	vfprintf(stderr, format, va_args);
	fprintf(stderr, "\n\n");
	fprintf(stderr,
		"%sUSAGE%s:\n    %s%s%s [%sOPTIONS%s]\n\nFor more information "
		"try %s--help%s\n",
		DIMMED, RESET, BRIGHT_RED, prog, RESET, DIMMED, RESET, GREEN,
		RESET);
	va_end(va_args);
	u64 debug_flags = *Args_get_debug_flags(args);
	if (!(debug_flags & DEBUG_INIT_NO_EXIT))
		exit(-1);
}

void args_print_version(Args *args) {
	char *prog = *Args_get_prog(args);
	char *version = *Args_get_version(args);
	fprintf(stderr, "%s%s%s %s%s%s\n", BRIGHT_RED, prog, RESET, GREEN,
		version, RESET);
}

bool args_check_sub_command(Args *args, char *arg) {

	u64 arg_itt = *Args_get_arg_itt(args);
	u64 sub_count = *Args_get_sub_count(args);
	SubCommandPtr *sub_arr = *Args_get_sub(args);
	if (arg_itt == 0) {
		for (u64 i = 0; i < sub_count; i++) {
			char *name = *SubCommand_get_name(&sub_arr[i]);
			if (!strcmp(name, arg)) {
				Args_set_arg_itt(args, 1);
				Args_set_sub_index(args, i);
				return true;
			}
		}
	} else {
		u64 index = *Args_get_sub_index(args);
		u64 max = *SubCommand_get_max_add_args(&sub_arr[index]);
		arg_itt += 1;
		Args_set_arg_itt(args, arg_itt);

		if (arg_itt > 1 + max) {
			return false;
		} else {
			return true;
		}
	}
	return false;
}

bool Args_init(Args *args, int argc, char **argv, u64 debug_flags) {
	bool ret = true;
	char **argv_copy = tlmalloc(sizeof(char *) * argc);
	u64 i = 0;
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			Args_usage(args);
			exit(0);
		}
		if (!strcmp(argv[i], "-V") || !strcmp(argv[i], "--help")) {
			args_print_version(args);
			exit(0);
		}
	}

	for (i = 0; i < argc; i++) {
		argv_copy[i] = tlmalloc(sizeof(char) * strlen(argv[i]));
		if (!argv_copy[i]) {
			panic("Could not allocate memory to build params");
		}
		strcpy(argv_copy[i], argv[i]);
	}

	Args_set_argv(args, argv_copy);
	Args_set_argc(args, argc);
	Args_set_debug_flags(args, debug_flags);

	// set specified to false for all
	u64 count = *(u64 *)Args_get_count(args);
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(args);
	for (u64 i = 0; i < count; i++) {
		ArgsParam_set_specified(&params[i], false);
	}

	u64 sub_found = 0;
	for (u64 i = 1; i < argc; i++) {
		bool is_takes_value, is_multi, already_specified,
		    is_sub = false;
		if (!find_param(args, argv_copy[i], &is_takes_value, &is_multi,
				&already_specified)) {
			// first check if we have sub commands
			if (!args_check_sub_command(args, argv_copy[i])) {
				args_error_exit(args,
						"Unknown option: '%s%s%s'",
						YELLOW, argv_copy[i], RESET);
				ret = false;
				break;
			} else {
				// found a subcommand
				is_sub = true;
			}
		}

		if (is_sub) {
			sub_found += 1;
		} else if (already_specified && !is_multi) {
			args_error_exit(
			    args, "Parameter specified more than once: %s",
			    argv_copy[i]);
			ret = false;
			break;
		} else if (is_multi && !is_takes_value) {
			args_error_exit(args,
					"Parameter cannot be multi and and not "
					"takes_value: %s",
					argv_copy[i]);
			ret = false;
			break;
		} else if (is_takes_value) {
			i += 1;
			if (i >= argc) {
				args_error_exit(args,
						"Expected a value for: %s",
						argv_copy[i - 1]);
				ret = false;
				break;
			}
			if (argv_copy[i][0] == '-') {
				args_error_exit(args,
						"Expected a value for: %s",
						argv_copy[i - 1]);
				ret = false;
				break;
			}
		}
	}

	if (sub_found) {
		u64 index = *Args_get_sub_index(args);
		SubCommandPtr *sub_arr = *Args_get_sub(args);
		u64 min = *SubCommand_get_min_add_args(&sub_arr[index]);
		char *name = *SubCommand_get_name(&sub_arr[index]);
		if (sub_found < min + 1) {
			args_error_exit(args,
					"Expected a minimum of %llu arguments "
					"for sub command '%s'",
					min, name);
			ret = false;
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
			if (itt == 0) {
				ArgsParam_set_argv_itt(&params[match_index], 1);
				strncpy(buffer, argv[i], len);
			} else {
				u64 first_match = i;
				i += itt;
				while (true) {
					if (i >= argc) {
						ret = false;
						break;
					}

					if (!strcmp(param_buffer, argv[i]) ||
					    !strcmp(param_buffer_short,
						    argv[i])) {
						// next match
						i += 1;
						if (i >= argc) {
							ret = false;
							break;
						}
						strncpy(buffer, argv[i], len);
						ArgsParam_set_argv_itt(
						    &params[match_index],
						    (i - first_match) + 1);
						break;
					}
					i += 1;
				}
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

void Args_usage(Args *args) {
	u64 count = *(u64 *)Args_get_count(args);
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(args);
	u64 max_len = 0;
	for (u64 i = 0; i < count; i++) {
		bool takes_value = *ArgsParam_get_takes_value(&params[i]);
		bool multi = *ArgsParam_get_multiple(&params[i]);
		char *name = *ArgsParam_get_name(&params[i]);
		char *short_name = *ArgsParam_get_short_name(&params[i]);
		u64 len;
		if (!takes_value)
			len = snprintf(NULL, 0, "    -%s, --%s", short_name,
				       name);
		else if (multi)
			len = snprintf(NULL, 0, "    -%s, --%s (<%s>, ...)",
				       short_name, name, name);
		else
			len = snprintf(NULL, 0, "    -%s, --%s <%s>",
				       short_name, name, name);
		if (len > max_len)
			max_len = len;
	}

	max_len += 4;

	if (max_len < 17)
		max_len = 17;

	char *prog = *Args_get_prog(args);
	char *author = *Args_get_author(args);
	char *version = *Args_get_version(args);
	char buffer[1025];
	char buffer2[1025];
	u64 i;
	for (i = 0; i < max_len - 13 && i < 1024; i++)
		buffer[i] = ' ';
	buffer[i] = 0;

	for (i = 0; i < max_len - 16 && i < 1024; i++)
		buffer2[i] = ' ';
	buffer2[i] = 0;

	fprintf(
	    stderr,
	    "%s%s%s %s%s%s\n%s%s%s\n\n%sUSAGE%s:\n    %s%s%s [%sOPTIONS%s]\n\n"
	    "%sFLAGS%s:\n"
	    "    %s-h%s, %s--help%s%sPrints help information\n"
	    "    %s-V%s, %s--version%s%sPrints version information\n",
	    CYAN, prog, RESET, YELLOW, version, RESET, GREEN, author, RESET,
	    DIMMED, RESET, BRIGHT_RED, prog, RESET, DIMMED, RESET, DIMMED,
	    RESET, CYAN, RESET, YELLOW, RESET, buffer, CYAN, RESET, YELLOW,
	    RESET, buffer2);

	for (u64 i = 0; i < count; i++) {
		bool takes_value = *ArgsParam_get_takes_value(&params[i]);
		if (!takes_value) {
			char *name = *ArgsParam_get_name(&params[i]);
			char *short_name =
			    *ArgsParam_get_short_name(&params[i]);
			char *help = *ArgsParam_get_help(&params[i]);
			u64 len = snprintf(NULL, 0, "    -%s, --%s", short_name,
					   name);
			u64 i;
			for (i = 0; i < max_len - len && i < 1024; i++)
				buffer[i] = ' ';
			buffer[i] = 0;
			fprintf(stderr, "    %s-%s%s, %s--%s%s %s%s\n", CYAN,
				short_name, RESET, YELLOW, name, RESET, buffer,
				help);
		}
	}
	if (count) {
		fprintf(stderr, "\n%sOPTIONS%s:\n", DIMMED, RESET);

		for (u64 i = 0; i < count; i++) {
			bool takes_value =
			    *ArgsParam_get_takes_value(&params[i]);
			if (takes_value) {
				char *name = *ArgsParam_get_name(&params[i]);
				char *short_name =
				    *ArgsParam_get_short_name(&params[i]);
				char *help = *ArgsParam_get_help(&params[i]);
				bool multi =
				    *ArgsParam_get_multiple(&params[i]);
				if (multi) {
					u64 len = snprintf(
					    NULL, 0,
					    "    -%s, --%s (<%s>, ...)",
					    short_name, name, name);
					u64 i;
					for (i = 0;
					     i < max_len - len && i < 1024; i++)
						buffer[i] = ' ';
					buffer[i] = 0;
					fprintf(stderr,
						"    %s-%s%s, %s--%s%s (<%s>, "
						"...) %s%s\n",
						CYAN, short_name, RESET, YELLOW,
						name, RESET, name, buffer,
						help);
				} else {

					u64 len = snprintf(
					    NULL, 0, "    -%s, --%s <%s>",
					    short_name, name, name);
					u64 i;
					for (i = 0;
					     i < max_len - len && i < 1024; i++)
						buffer[i] = ' ';
					buffer[i] = 0;

					fprintf(
					    stderr,
					    "    %s-%s%s, %s--%s%s <%s> %s%s\n",
					    CYAN, short_name, RESET, YELLOW,
					    name, RESET, name, buffer, help);
				}
			}
		}
	}
}

char *Args_get_sub_command(Args *args, u64 index) {
	int argc = *Args_get_argc(args);
	char **argv = *Args_get_argv(args);
	int counter = 0;
	ArgsParamPtr *params = *(ArgsParamPtr **)Args_get_params(args);
	u64 count = *(u64 *)Args_get_count(args);

	for (u64 i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			int len = strlen(argv[i]);
			char buf[len + 1];
			int start = 1;
			if (len > 2 && argv[i][1] == '-') {
				start = 2;
			}

			if (len > start) {
				int itt = 0;
				u64 j = 0;
				for (j = start; j < len; j++) {
					buf[itt] = argv[i][j];
					itt++;
				}
				buf[itt] = 0;
			}

			for (u64 j = 0; j < count; j++) {
				char *name = *ArgsParam_get_name(&params[j]);
				char *short_name =
				    *ArgsParam_get_short_name(&params[j]);
				bool takes_value =
				    *ArgsParam_get_takes_value(&params[j]);
				if (!strcmp(name, buf) ||
				    !strcmp(short_name, buf)) {

					// if it takes a value, skip
					if (takes_value) {
						i += 1;
						break;
					}
				}
			}

		} else {
			// this is a sub command arg
			if (counter == index) {
				// printf("ret=%i %s\n", i, argv[i]);
				return argv[i];
			}
			counter += 1;
		}
	}
	return NULL;
}
