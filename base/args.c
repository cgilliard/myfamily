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
#include <base/ekinds.h>
#include <base/string.h>
#include <base/unit.h>
#include <stdlib.h>
#include <unistd.h>

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

GETTER(SubCommand, name)
SETTER(SubCommand, name)
GETTER(SubCommand, params)
SETTER(SubCommand, params)
GETTER(SubCommand, params_state)
SETTER(SubCommand, params_state)
GETTER(SubCommand, count)
SETTER(SubCommand, count)
GETTER(SubCommand, min_add_args)
SETTER(SubCommand, min_add_args)
GETTER(SubCommand, max_add_args)
SETTER(SubCommand, max_add_args)
GETTER(SubCommand, help)
SETTER(SubCommand, help)

GETTER(ArgsParamState, specified)
SETTER(ArgsParamState, specified)
GETTER(ArgsParamState, itt)
SETTER(ArgsParamState, itt)

GETTER(Args, subs)
SETTER(Args, subs)
GETTER(Args, subs_count)
SETTER(Args, subs_count)
GETTER(Args, prog)
SETTER(Args, prog)
GETTER(Args, version)
SETTER(Args, version)
GETTER(Args, author)
SETTER(Args, author)
GETTER(Args, argv)
SETTER(Args, argv)
GETTER(Args, argc)
SETTER(Args, argc)
GETTER(Args, debug_flags)
SETTER(Args, debug_flags)

void ArgsParamState_cleanup(ArgsParamState *ptr) {}
usize ArgsParamState_size(ArgsParamState *ptr) {
	return sizeof(ArgsParamState);
}
bool ArgsParamState_clone(ArgsParamState *dst, ArgsParamState *src) {
	dst->_specified = src->_specified;
	dst->_itt = src->_itt;
	return true;
}

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
bool ArgsParam_clone(ArgsParam *dst, ArgsParam *src) {
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

usize ArgsParam_size(ArgsParam *ptr) { return sizeof(ArgsParam); }

void SubCommand_cleanup(SubCommand *ptr) {
	char *name = *SubCommand_get_name(ptr);
	u64 count = *SubCommand_get_count(ptr);
	if (name) {
		tlfree(name);
		SubCommand_set_name(ptr, NULL);
	}

	char *help = *SubCommand_get_help(ptr);
	if (help) {
		tlfree(help);
		SubCommand_set_help(ptr, NULL);
	}

	ArgsParamPtr *params = *SubCommand_get_params(ptr);
	if (params) {
		for (u64 i = 0; i < count; i++) {
			cleanup(&params[i]);
		}
		tlfree(params);
		SubCommand_set_params(ptr, NULL);
	}

	ArgsParamStatePtr *params_state = *SubCommand_get_params_state(ptr);
	if (params_state) {
		for (u64 i = 0; i < count; i++) {
			cleanup(&params_state[i]);
		}
		tlfree(params_state);
		SubCommand_set_params_state(ptr, NULL);
	}

	SubCommand_set_count(ptr, 0);
}

SubCommand SubCommand_build_impl(char *name, ArgsParam *params,
				 ArgsParamState *state, u64 count, u64 min_args,
				 u64 max_args, char *help) {
	SubCommandPtr ret = BUILD(SubCommand, name, params, state, count,
				  min_args, max_args, help);

	return ret;
}

bool SubCommand_clone(SubCommand *dst, SubCommand *src) {
	bool ret = true;
	char *src_name = *SubCommand_get_name(src);
	char *name = tlmalloc(sizeof(char) * (strlen(src_name) + 1));
	char *src_help = *SubCommand_get_help(src);
	char *help = tlmalloc(sizeof(char) * strlen(src_help) + 1);
	strcpy(help, src_help);
	if (name == NULL)
		return false;
	strcpy(name, src_name);
	u64 count = *SubCommand_get_count(src);
	ArgsParamPtr *params = NULL;
	if (count > 0) {
		params = tlmalloc(sizeof(ArgsParam) * count);
		if (params == NULL) {
			if (name)
				tlfree(name);
			return false;
		}
	} else
		params = NULL;

	ArgsParamStatePtr *params_state;
	if (count > 0) {
		params_state = tlmalloc(sizeof(ArgsParamState) * count);
		if (params_state == NULL) {
			if (params)
				tlfree(params);
			if (name)
				tlfree(name);
			return false;
		}
	} else
		params_state = NULL;
	ArgsParamPtr *params_src = *SubCommand_get_params(src);
	ArgsParamStatePtr *params_state_src = *SubCommand_get_params_state(src);
	for (u64 i = 0; i < count; i++) {
		if (!copy(&params[i], &params_src[i])) {
			ret = false;
			break;
		}
		if (!copy(&params_state[i], &params_state_src[i])) {
			ret = false;
			break;
		}
	}

	if (ret) {
		u64 max_args = *SubCommand_get_max_add_args(src);
		u64 min_args = *SubCommand_get_min_add_args(src);

		*dst = SubCommand_build_impl(name, params, params_state, count,
					     min_args, max_args, help);
	} else {
		if (params)
			tlfree(params);
		if (name)
			tlfree(name);
		if (params_state)
			tlfree(params_state);
	}
	return ret;
}
usize SubCommand_size(SubCommand *ptr) { return sizeof(SubCommand); }
Result SubCommand_build(char *name, u32 min_args, u32 max_args, char *help) {

	SubCommandPtr src =
	    BUILD(SubCommand, name, NULL, NULL, 0, min_args, max_args, help);
	SubCommand ret;
	copy(&ret, &src);
	return Ok(ret);
}

Result SubCommand_add_param(SubCommand *ptr, ArgsParam *param) {
	u64 count = *SubCommand_get_count(ptr);
	if (count == 0) {
		ArgsParamPtr *params_ptr = tlmalloc(sizeof(ArgsParam));
		if (params_ptr == NULL) {
			Error err =
			    ERROR(ALLOC_ERROR, "could not allocate memory");
			return Err(err);
		}

		ArgsParamStatePtr *params_state_ptr =
		    tlmalloc(sizeof(ArgsParamState));
		if (params_state_ptr == NULL) {
			tlfree(params_ptr);
			Error err =
			    ERROR(ALLOC_ERROR, "could not allocate memory");
			return Err(err);
		}

		ArgsParamStatePtr tmp_state = BUILD(ArgsParamState, false, 0);
		memcpy(&params_state_ptr[count], &tmp_state,
		       sizeof(ArgsParamState));

		copy(&params_ptr[count], param);
		SubCommand_set_params(ptr, params_ptr);
		SubCommand_set_params_state(ptr, params_state_ptr);
	} else {
		ArgsParamPtr *params_ptr = *SubCommand_get_params(ptr);
		void *tmp =
		    tlrealloc(params_ptr, sizeof(ArgsParam) * (1 + count));
		if (tmp == NULL) {
			Error err =
			    ERROR(ALLOC_ERROR, "could not allocate memory");
			return Err(err);
		}
		params_ptr = tmp;

		ArgsParamStatePtr *params_state_ptr =
		    *SubCommand_get_params_state(ptr);
		tmp = tlrealloc(params_state_ptr,
				sizeof(ArgsParamState) * (1 + count));
		if (tmp == NULL) {
			Error err =
			    ERROR(ALLOC_ERROR, "could not allocate memory");
			return Err(err);
		}
		params_state_ptr = tmp;

		ArgsParamStatePtr tmp_state = BUILD(ArgsParamState, false, 0);
		memcpy(&params_state_ptr[count], &tmp_state,
		       sizeof(ArgsParamState));

		copy(&params_ptr[count], param);
		SubCommand_set_params(ptr, params_ptr);
		SubCommand_set_params_state(ptr, params_state_ptr);
	}

	SubCommand_set_count(ptr, count + 1);
	return Ok(UNIT);
}

void Args_error_exit(Args *args, char *format, ...) {
	char *prog = *Args_get_prog(args);
	va_list va_args;
	va_start(va_args, format);
	fprintf(stderr, "%sError%s: ", BRIGHT_RED, RESET);
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

void Args_cleanup(Args *ptr) {
	u64 count = *Args_get_subs_count(ptr);
	SubCommandPtr **subs = *Args_get_subs(ptr);
	for (u64 i = 0; i < count; i++) {
		cleanup(subs[i]);
		tlfree(subs[i]);
	}
	if (subs) {
		tlfree(subs);
		Args_set_subs(ptr, NULL);
	}

	char *prog = *Args_get_prog(ptr);
	if (prog) {
		tlfree(prog);
		Args_set_prog(ptr, NULL);
	}

	char *version = *Args_get_version(ptr);
	if (version) {
		tlfree(version);
		Args_set_version(ptr, NULL);
	}

	char *author = *Args_get_author(ptr);
	if (author) {
		tlfree(author);
		Args_set_author(ptr, NULL);
	}

	int argc = *Args_get_argc(ptr);
	char **argv = *Args_get_argv(ptr);

	for (u64 i = 0; i < argc; i++) {
		tlfree(argv[i]);
	}
	if (argc > 0)
		tlfree(argv);
}
Result Args_add_param(Args *ptr, ArgsParam *param) {
	SubCommandPtr **subs = *Args_get_subs(ptr);
	Result res2 = SubCommand_add_param(subs[0], param);
	Try(res2);
	return Ok(UNIT);
}
Result Args_add_sub(Args *ptr, SubCommand *sub) {

	u64 count = *(u64 *)Args_get_subs_count(ptr);
	SubCommandPtr **subs = *Args_get_subs(ptr);
	if (count == 0) {
		void *tmp = tlmalloc(sizeof(SubCommand *));
		if (tmp == NULL) {
			Error err =
			    ERROR(ALLOC_ERROR, "Could not increase the size of "
					       "the sub command list");
			return Err(err);
		}
		subs = tmp;
		Args_set_subs(ptr, tmp);
	} else {
		void *tmp = tlrealloc(subs, sizeof(SubCommand *) * (1 + count));
		if (tmp == NULL) {
			Error err =
			    ERROR(ALLOC_ERROR, "Could not increase the size of "
					       "the sub command list");
			return Err(err);
		}
		subs = tmp;
		Args_set_subs(ptr, tmp);
	}
	subs[count] = tlmalloc(sizeof(SubCommand));
	copy(subs[count], sub);
	Args_set_subs_count(ptr, count + 1);

	return Ok(UNIT);
}

bool Args_clone(Args *dst, Args *src) {
	bool ret = true;
	char *prog_src = *Args_get_prog(src);
	char *version_src = *Args_get_version(src);
	char *author_src = *Args_get_author(src);

	char *prog;
	if (prog_src) {
		prog = tlmalloc(sizeof(char) * (strlen(prog_src) + 1));

		if (prog == NULL)
			ret = false;
		else
			strcpy(prog, prog_src);
	} else
		prog = NULL;

	char *version;
	if (version_src) {
		version = tlmalloc(sizeof(char) * (strlen(version_src) + 1));
		if (version == NULL)
			ret = false;
		else
			strcpy(version, version_src);
	} else
		version = NULL;

	char *author;
	if (author_src) {
		author = tlmalloc(sizeof(char) * (strlen(author_src) + 1));
		if (author == NULL)
			ret = false;
		else
			strcpy(author, author_src);
	}

	else
		author = NULL;

	int argc = *Args_get_argc(src);
	char **argv = *Args_get_argv(src);

	char **argv_copy;
	if (argc > 0)
		argv_copy = tlmalloc(sizeof(char *) * argc);
	else
		argv_copy = NULL;
	for (u64 i = 0; i < argc; i++) {
		argv_copy[i] = tlmalloc(sizeof(char) * (1 + strlen(argv[i])));
		if (!argv_copy[i]) {
			for (u64 j = i - 1; j >= 0; j--)
				tlfree(argv_copy[j]);
			tlfree(argv_copy);
		}
		strcpy(argv_copy[i], argv[i]);
	}

	Args_set_argv(dst, argv_copy);
	Args_set_argc(dst, argc);

	if (ret) {
		Args_set_prog(dst, prog);
		Args_set_version(dst, version);
		Args_set_author(dst, author);
	}

	u64 subs_count = *Args_get_subs_count(src);
	SubCommandPtr **subs_src = *Args_get_subs(src);
	SubCommandPtr **subs;
	if (subs_count > 0) {
		subs = tlmalloc(sizeof(SubCommand *) * subs_count);
		if (subs == NULL)
			ret = false;
		else
			for (u64 i = 0; i < subs_count; i++) {
				subs[i] = tlmalloc(sizeof(SubCommand));
				if (!copy(subs[i], subs_src[i]))
					ret = false;
			}

	} else
		subs = NULL;
	Args_set_subs(dst, subs);
	Args_set_subs_count(dst, subs_count);

	if (!ret) {
		if (prog)
			tlfree(prog);
		if (author)
			tlfree(author);
		if (version)
			tlfree(version);
		if (subs)
			tlfree(subs);
	}

	return ret;
}

usize Args_size(Args *obj) { return sizeof(Args); }

Result Args_build(char *prog, char *version, char *author) {
	ArgsPtr src = BUILD(Args, prog, version, author, NULL, 0, NULL, 0, 0);
	Args ret;
	if (!copy(&ret, &src)) {
		Error err = ERROR(COPY_ERROR, "could not copy sub args");
		return Err(err);
	}
	Result res = SubCommand_build("", 0, 0, "");

	SubCommand sub = *(SubCommand *)Try(res);
	Result res2 = Args_add_sub(&ret, &sub);
	Try(res2);

	return Ok(ret);
}

Result Args_print_version(Args *args) {
	char *prog = *Args_get_prog(args);
	char *version = *Args_get_version(args);
	fprintf(stderr, "%s%s%s %s%s%s\n", BRIGHT_RED, prog, RESET, GREEN,
		version, RESET);
	exit(0);
	return Ok(UNIT);
}

bool find_param(Args *args, char *name, bool *is_takes_value, bool *is_multi,
		bool *already_specified, u64 *sub_index) {
	if (name[0] != '-' || strlen(name) < 2)
		return false;

	bool ret = false;

	SubCommandPtr **subs = *Args_get_subs(args);
	u64 count = *(u64 *)SubCommand_get_count(subs[*sub_index]);
	ArgsParamPtr *params =
	    *(ArgsParamPtr **)SubCommand_get_params(subs[*sub_index]);
	ArgsParamStatePtr *params_state = *(
	    ArgsParamStatePtr **)SubCommand_get_params_state(subs[*sub_index]);

	for (u64 i = 0; i < count; i++) {
		char *pname = *ArgsParam_get_name(&params[i]);
		char *short_name = *ArgsParam_get_short_name(&params[i]);
		bool specified =
		    *ArgsParamState_get_specified(&params_state[i]);

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
				ArgsParamState_set_specified(&params_state[i],
							     true);

				ret = true;
				break;
			}
		}
	}

	return ret;
}

bool args_check_sub_command(Args *args, char *arg, u64 *sub_itt,
			    u64 *sub_index) {
	u64 len = strlen(arg);
	if (len >= 1 && arg[0] == '-') // option
		return false;
	u64 sub_count = *Args_get_subs_count(args);
	SubCommandPtr **sub_arr = *Args_get_subs(args);
	if (*sub_itt == 0) {
		for (u64 i = 0; i < sub_count; i++) {
			char *name = *SubCommand_get_name(sub_arr[i]);
			if (!strcmp(name, arg)) {
				*sub_itt = 1;
				*sub_index = i;
				return true;
			}
		}
	} else {
		u64 max = *SubCommand_get_max_add_args(sub_arr[*sub_index]);
		*sub_itt += 1;

		if (*sub_itt > 1 + max) {
			Args_error_exit(
			    args,
			    "Too many arguments specified for this command\n");
		} else {
			return true;
		}
	}
	return false;
}

Result Args_init(Args *args, int argc, char **argv, u64 flags) {
	bool ret = true;
	char **argv_copy = tlmalloc(sizeof(char *) * argc);
	if (argv_copy == NULL) {
		Error err =
		    ERROR(ALLOC_ERROR, "Could not allocate required memory");
		return Err(err);
	}
	u64 i = 0;
	u64 sub_itt = 0;
	u64 sub_index = 0;
	char *sub_command = NULL;
	for (i = 1; i < argc; i++) {
		args_check_sub_command(args, argv[i], &sub_itt, &sub_index);
		if (sub_command == NULL && sub_itt == 1) {
			sub_command = argv[i];
		}
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			Args_usage(args, sub_command);
			exit(0);
		}
		if (!strcmp(argv[i], "-V") || !strcmp(argv[i], "--help")) {
			Result x = Args_print_version(args);
			Expect(x);
			exit(0);
		}
	}

	for (i = 0; i < argc; i++) {
		argv_copy[i] = tlmalloc(sizeof(char) * (1 + strlen(argv[i])));
		if (!argv_copy[i]) {
			for (u64 j = i - 1; j >= 0; j--)
				tlfree(argv_copy[j]);
			tlfree(argv_copy);
		}
		strcpy(argv_copy[i], argv[i]);
	}

	Args_set_argv(args, argv_copy);
	Args_set_argc(args, argc);

	// set specified to false for all
	SubCommandPtr *subs = *(SubCommand **)Args_get_subs(args);
	u64 count = *(u64 *)SubCommand_get_count(&subs[0]);

	ArgsParamStatePtr *params_state =
	    (ArgsParamStatePtr *)SubCommand_get_params_state(&subs[0]);

	for (u64 i = 0; i < count; i++) {
		ArgsParamState_set_specified(&params_state[i], false);
	}

	sub_itt = 0;
	sub_index = 0;
	bool sub_found = false;
	for (u64 i = 1; i < argc; i++) {
		bool is_takes_value, is_multi, already_specified,
		    is_sub = false;
		if (!find_param(args, argv_copy[i], &is_takes_value, &is_multi,
				&already_specified, &sub_index)) {

			if (!args_check_sub_command(args, argv_copy[i],
						    &sub_itt, &sub_index)) {
				Args_error_exit(args, "unknown param: %s\n",
						argv_copy[i]);
			} else {
				is_sub = true;
			}
		}

		if (is_sub) {
			sub_found = true;
		} else if (!is_multi && already_specified) {
			Args_error_exit(args, "Specified more than once: %s\n",
					argv_copy[i]);
		} else if (is_takes_value) {
			i += 1;
			if (i >= argc) {
				Args_error_exit(args,
						"expected a value for %s\n",
						argv_copy[i - 1]);
			}
			if (argv_copy[i][0] == '-') {
				Args_error_exit(args,
						"Expected a value for: %s\n",
						argv_copy[i - 1]);
				ret = false;
			}
		}
	}

	if (sub_found) {
		SubCommandPtr **sub_arr = *Args_get_subs(args);
		u64 min = *SubCommand_get_min_add_args(sub_arr[sub_index]);
		char *name = *SubCommand_get_name(sub_arr[sub_index]);
		if (min >= sub_itt) {
			Args_error_exit(
			    args,
			    "Sub command %s must have at least %i arguments\n",
			    name, min);
			exit(-1);
		}
	}

	return Ok(UNIT);
}

void Args_usage(Args *args, char *sub_command) {
	u64 subs_count = *Args_get_subs_count(args);
	SubCommandPtr **subs = *Args_get_subs(args);
	ArgsParamPtr *params = *SubCommand_get_params(subs[0]);
	u64 count = *SubCommand_get_count(subs[0]);
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
	u64 sub_command_str_len;
	if (sub_command)
		sub_command_str_len = strlen(sub_command);
	else
		sub_command_str_len = 0;
	char sub_command_str[sub_command_str_len + 30];
	if (sub_command) {
		snprintf(sub_command_str, sub_command_str_len + 30, "%s%s%s",
			 BRIGHT_RED, sub_command, RESET);
	} else {
		snprintf(sub_command_str, sub_command_str_len + 30,
			 "[%sSUB_COMMAND%s]", DIMMED, RESET);
	}
	fprintf(stderr,
		"%s%s%s %s%s%s\n%s%s%s\n\n%sUSAGE%s:\n    %s%s%s "
		"[%sCORE_OPTIONS%s] %s [%sSUB_OPTIONS%s]\n\n"
		"%sCORE_FLAGS%s:\n"
		"    %s-h%s, %s--help%s%sPrints help information\n"
		"    %s-V%s, %s--version%s%sPrints version information\n",
		CYAN, prog, RESET, YELLOW, version, RESET, GREEN, author, RESET,
		DIMMED, RESET, BRIGHT_RED, prog, RESET, DIMMED, RESET,
		sub_command_str, DIMMED, RESET, DIMMED, RESET, CYAN, RESET,
		YELLOW, RESET, buffer, CYAN, RESET, YELLOW, RESET, buffer2);

	for (u64 i = 0; i < count; i++) {
		bool takes_value = *ArgsParam_get_takes_value(&params[i]);
		if (!takes_value) {
			char *name = *ArgsParam_get_name(&params[i]);
			char *short_name =
			    *ArgsParam_get_short_name(&params[i]);
			char *help = *ArgsParam_get_help(&params[i]);
			u64 len = snprintf(NULL, 0, "    -%s, --%s", short_name,
					   name);
			if (len > max_len)
				len = max_len;
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
		fprintf(stderr, "\n%sCORE_OPTIONS%s:\n", DIMMED, RESET);

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
					    NULL, 0, "    -%s, --%s <%s>, ...",
					    short_name, name, name);
					if (len > max_len)
						len = max_len;
					u64 i;
					for (i = 0;
					     i < max_len - len && i < 1024; i++)
						buffer[i] = ' ';
					buffer[i] = 0;
					fprintf(stderr,
						"    %s-%s%s, %s--%s%s <%s>, "
						"... %s%s\n",
						CYAN, short_name, RESET, YELLOW,
						name, RESET, name, buffer,
						help);
				} else {

					u64 len = snprintf(
					    NULL, 0, "    -%s, --%s <%s>",
					    short_name, name, name);
					if (len > max_len)
						len = max_len;
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

	if (subs_count > 1 && sub_command == NULL) {
		fprintf(stderr, "\n%sSUB_COMMANDS%s:\n", DIMMED, RESET);
		for (u64 i = 1; i < subs_count; i++) {

			char *name = *SubCommand_get_name(subs[i]);
			char *help = *SubCommand_get_help(subs[i]);

			u64 len = strlen(name) + 3;
			if (len > max_len)
				len = max_len;
			char buffer[1025];
			u64 j;
			for (j = 0; j < (max_len - len) && j < 1024; j++)
				buffer[j] = ' ';
			buffer[j] = 0;

			fprintf(stderr, "    %s%s%s%s%s\n", CYAN, name, RESET,
				buffer, help);
		}
	}

	if (sub_command) {
		u64 param_index = 0;
		for (u64 i = 1; i < subs_count; i++) {
			char *name = *SubCommand_get_name(subs[i]);
			if (!strcmp(name, sub_command)) {
				param_index = i;
			}
		}

		if (param_index > 0) {
			params = *SubCommand_get_params(subs[param_index]);
			count = *SubCommand_get_count(subs[param_index]);

			fprintf(stderr, "\n%sSUB_FLAGS%s (%s%s%s):\n", DIMMED,
				RESET, BRIGHT_RED, sub_command, RESET);
			for (u64 i = 0; i < count; i++) {
				char *name = *ArgsParam_get_name(&params[i]);
				char *short_name =
				    *ArgsParam_get_short_name(&params[i]);
				bool takes_value =
				    *ArgsParam_get_takes_value(&params[i]);
				char *help = *ArgsParam_get_help(&params[i]);

				if (!takes_value) {
					u64 len = strlen(name) + 10;
					if (len > max_len)
						len = max_len;
					char buffer[1025];
					u64 j;
					for (j = 0;
					     j < (max_len - len) && j < 1024;
					     j++)
						buffer[j] = ' ';
					buffer[j] = 0;
					fprintf(stderr,
						"    %s-%s%s, %s--%s%s%s %s\n",
						CYAN, short_name, RESET, YELLOW,
						name, RESET, buffer, help);
				}
			}

			fprintf(stderr, "\n%sSUB_OPTIONS%s (%s%s%s):\n", DIMMED,
				RESET, BRIGHT_RED, sub_command, RESET);
			for (u64 i = 0; i < count; i++) {
				char *name = *ArgsParam_get_name(&params[i]);
				char *short_name =
				    *ArgsParam_get_short_name(&params[i]);
				bool takes_value =
				    *ArgsParam_get_takes_value(&params[i]);
				char *help = *ArgsParam_get_help(&params[i]);
				bool multiple =
				    *ArgsParam_get_multiple(&params[i]);

				if (takes_value) {
					if (multiple) {
						u64 len = 2 * strlen(name) + 19;
						char buffer[1025];
						u64 j;
						if (len > max_len)
							len = max_len;
						for (j = 0;
						     j < (max_len - len) &&
						     j < 1024;
						     j++)
							buffer[j] = ' ';
						buffer[j] = 0;
						fprintf(stderr,
							"    %s-%s%s, %s--%s%s "
							"<%s>, ...%s %s"
							"%s\n",
							CYAN, short_name, RESET,
							YELLOW, name, RESET,
							name, buffer, help);
					} else {
						u64 len = 2 * strlen(name) + 13;
						char buffer[1025];
						u64 j;
						if (len > max_len)
							len = max_len;
						for (j = 0;
						     j < (max_len - len) &&
						     j < 1024;
						     j++)
							buffer[j] = ' ';
						buffer[j] = 0;
						fprintf(stderr,
							"    %s-%s%s, %s--%s%s "
							"<%s>%s "
							"%s\n",
							CYAN, short_name, RESET,
							YELLOW, name, RESET,
							name, buffer, help);
					}
				}
			}
		}
	}
}

Option Args_argument(Args *args, u64 index) {
	int argc = *Args_get_argc(args);
	char **argv = *Args_get_argv(args);
	SubCommandPtr **sub_arr = *Args_get_subs(args);
	u64 sub_count = *Args_get_subs_count(args);
	ArgsParamPtr *params =
	    *(ArgsParamPtr **)SubCommand_get_params(sub_arr[0]);
	u64 count = *(u64 *)SubCommand_get_count(sub_arr[0]);

	int counter = 0;
	u64 match_index = 0;

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
			if (counter == 0) {
				for (u64 j = 1; j < sub_count; j++) {
					char *name =
					    *SubCommand_get_name(sub_arr[j]);
					if (!strcmp(name, argv[i])) {
						match_index = j;
						params =
						    *(ArgsParamPtr **)
							SubCommand_get_params(
							    sub_arr[j]);
						count =
						    *(u64 *)
							SubCommand_get_count(
							    sub_arr[j]);
						break;
					}
				}
			}
			if (counter == index) {
				StringPtr *s = STRINGPTRP(argv[i]);
				return Some(*s);
			}
			counter += 1;
		}
	}

	return None;
}

Result Args_value(Args *args, char *param) {
	char param_buffer[strlen(param) + 3];
	char param_buffer_short[strlen(param) + 3];
	strcpy(param_buffer, "--");
	strcat(param_buffer, param);
	strcpy(param_buffer_short, "-");
	bool ret = false;
	int argc = *Args_get_argc(args);
	char **argv = *Args_get_argv(args);

	SubCommandPtr **sub_arr = *Args_get_subs(args);
	u64 sub_count = *Args_get_subs_count(args);
	ArgsParamPtr *params =
	    *(ArgsParamPtr **)SubCommand_get_params(sub_arr[0]);
	ArgsParamStatePtr *params_state =
	    *(ArgsParamStatePtr **)SubCommand_get_params_state(sub_arr[0]);
	u64 count = *(u64 *)SubCommand_get_count(sub_arr[0]);

	bool takes_value = false;
	bool multi = false;
	u64 match_index = 0;
	bool found = false;
	for (u64 i = 0; i < count; i++) {
		char *name = *ArgsParam_get_name(&params[i]);
		if (!strcmp(name, param)) {
			char *short_name =
			    *ArgsParam_get_short_name(&params[i]);
			takes_value = *ArgsParam_get_takes_value(&params[i]);
			multi = *ArgsParam_get_multiple(&params[i]);
			strcat(param_buffer_short, short_name);
			match_index = i;
			found = true;
			break;
		}
	}

	u64 i;
	if (!found) {
		u64 sub_itt = 0;
		u64 sub_index = 0;
		u64 match_arg = 0;
		for (i = 1; i < argc; i++) {
			args_check_sub_command(args, argv[i], &sub_itt,
					       &sub_index);
			if (sub_itt != 0) {
				// this is our sub_index
				match_arg = i;
				break;
			}
		}
		params =
		    *(ArgsParamPtr **)SubCommand_get_params(sub_arr[sub_index]);
		count = *(u64 *)SubCommand_get_count(sub_arr[sub_index]);
		params_state =
		    *(ArgsParamStatePtr **)SubCommand_get_params_state(
			sub_arr[sub_index]);
		for (i = 0; i < count; i++) {
			char *name = *ArgsParam_get_name(&params[i]);
			if (!strcmp(name, param)) {
				char *short_name =
				    *ArgsParam_get_short_name(&params[i]);
				takes_value =
				    *ArgsParam_get_takes_value(&params[i]);
				multi = *ArgsParam_get_multiple(&params[i]);
				strcat(param_buffer_short, short_name);
				match_index = i;
				found = true;
				break;
			}
		}
	}

	for (i = 1; i < argc; i++) {
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

	Option ret_val = None;
	if (ret) {
		i++;
		if (multi) {
			u64 itt =
			    *ArgsParamState_get_itt(&params_state[match_index]);
			if (itt == 0) {
				ArgsParamState_set_itt(
				    &params_state[match_index], 1);
				StringPtr *ret_str = STRINGPTRP(argv[i]);
				ret_val = Some(*ret_str);
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
						StringPtr *ret_str =
						    STRINGPTRP(argv[i]);
						ret_val = Some(*ret_str);
						ArgsParamState_set_itt(
						    &params_state[match_index],
						    (i - first_match) + 1);
						break;
					}
					i += 1;
				}
			}
		} else if (takes_value && i < argc) {
			StringPtr *ret_str = STRINGPTRP(argv[i]);
			ret_val = Some(*ret_str);
		}
	}

	return Ok(ret_val);
}
