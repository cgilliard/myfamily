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
#include <base/colors.h>
#include <base/resources.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool args_param_copy(ArgsParam *dst, const ArgsParam *src) {
	if (strlen(src->name) > ARGS_MAX_ARGUMENT_NAME_LENGTH)
		return false;
	if (strlen(src->short_name) > ARGS_MAX_ARGUMENT_NAME_LENGTH)
		return false;

	dst->help = mymalloc(strlen(src->help) + 1);
	if (dst->help == NULL)
		return false;

	if (src->default_value) {
		dst->default_value = mymalloc(strlen(src->default_value) + 1);
		if (dst->default_value == NULL) {
			myfree(dst->help);
			return false;
		}
	}

	strcpy(dst->name, src->name);
	strcpy(dst->help, src->help);
	strcpy(dst->short_name, src->short_name);
	dst->takes_value = src->takes_value;
	dst->multiple = src->multiple;
	if (src->default_value)
		strcpy(dst->default_value, src->default_value);
	else
		dst->default_value = NULL;

	return true;
}

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

bool sub_command_copy(SubCommand *dst, const SubCommand *src) {
	if (strlen(src->name) > ARGS_MAX_SUBCOMMAND_LENGTH)
		return false;

	dst->help = mymalloc(strlen(src->help) + 1);
	if (dst->help == NULL)
		return false;

	dst->arg_doc = mymalloc(strlen(src->arg_doc) + 1);
	if (dst->arg_doc == NULL) {
		myfree(dst->help);
		return false;
	}

	if (src->param_count == 0) {
		dst->param_count = 0;
		dst->is_specified = NULL;
		dst->params = NULL;
	} else {
		dst->params = mymalloc(sizeof(ArgsParam) * src->param_count);
		dst->is_specified = mymalloc(sizeof(bool) * src->param_count);

		if (dst->params == NULL || dst->is_specified == NULL) {
			if (dst->params)
				myfree(dst->params);
			if (dst->is_specified)
				myfree(dst->is_specified);
			if (dst->help)
				myfree(dst->help);
			if (dst->arg_doc)
				myfree(dst->arg_doc);
		}

		memcpy(dst->is_specified, src->is_specified, sizeof(bool) * src->param_count);
		for (u32 i = 0; i < src->param_count; i++) {
			args_param_copy(&dst->params[i], &src->params[i]);
		}
		dst->param_count = src->param_count;
	}

	strcpy(dst->name, src->name);
	if (src->help)
		strcpy(dst->help, src->help);
	else
		dst->help = NULL;
	if (src->arg_doc)
		strcpy(dst->arg_doc, src->arg_doc);
	else
		dst->arg_doc = NULL;

	dst->min_args = src->min_args;
	dst->max_args = src->max_args;

	return true;
}

void sub_command_cleanup(SubCommandImpl *sc) {
	for (u32 i = 0; i < sc->param_count; i++) {
		args_param_cleanup(&sc->params[i]);
	}
	if (sc->params) {
		myfree(sc->params);
		sc->params = NULL;
	}
	if (sc->help) {
		myfree(sc->help);
		sc->help = NULL;
	}
	if (sc->arg_doc) {
		myfree(sc->arg_doc);
		sc->arg_doc = NULL;
	}
	if (sc->is_specified) {
		myfree(sc->is_specified);
		sc->is_specified = NULL;
	}
}

int sub_command_build(SubCommand *sc, const char *name, const char *help, const u32 min_args,
					  const u32 max_args, const char *arg_doc) {
	sc->params = NULL;
	sc->help = NULL;
	sc->arg_doc = NULL;
	sc->is_specified = NULL;
	sc->param_count = 0;

	if (strlen(name) > ARGS_MAX_SUBCOMMAND_LENGTH || min_args > max_args) {
		errno = EINVAL;
		return -1;
	}

	if (help) {
		sc->help = mymalloc(sizeof(char) * strlen(help) + 1);
		if (sc->help == NULL)
			return -1;
		strcpy(sc->help, help);
	}

	if (arg_doc) {
		sc->arg_doc = mymalloc(sizeof(char) * strlen(arg_doc) + 1);
		if (sc->arg_doc == NULL) {
			sub_command_cleanup(sc);
			return -1;
		}
		strcpy(sc->arg_doc, arg_doc);
	}

	strcpy(sc->name, name);
	sc->min_args = min_args;
	sc->max_args = max_args;

	return 0;
}

int sub_command_add_param(SubCommand *sc, const ArgsParam *ap) {
	if (sc->params) {
		// already exists so realloc
		void *tmp;
		tmp = myrealloc(sc->params, sizeof(ArgsParam) * (sc->param_count + 1));
		if (tmp == NULL)
			return -1;
		sc->params = tmp;
		tmp = myrealloc(sc->is_specified, sizeof(bool) * (sc->param_count + 1));
		if (tmp == NULL)
			return -1;
		sc->is_specified = tmp;

		args_param_copy(&sc->params[sc->param_count], ap);
		sc->is_specified[sc->param_count] = false;

		sc->param_count += 1;
	} else {
		sc->params = mymalloc(sizeof(ArgsParam));
		sc->is_specified = mymalloc(sizeof(bool));

		if (sc->params == NULL || sc->is_specified == NULL) {
			if (sc->params)
				myfree(sc->params);
			if (sc->is_specified)
				myfree(sc->is_specified);
			return -1;
		}

		args_param_copy(&sc->params[sc->param_count], ap);
		sc->is_specified[sc->param_count] = false;
		sc->param_count += 1;
	}
	return 0;
}

void args_cleanup(ArgsImpl *ptr) {
	for (u32 i = 0; i < ptr->subs_count; i++) {
		printf("cleanup %u\n", i);
		sub_command_cleanup(&ptr->subs[i]);
	}
	if (ptr->subs) {
		myfree(ptr->subs);
		ptr->subs = NULL;
	}
	for (u64 i = 0; i < ptr->argc; i++) {
		myfree(ptr->argv[i]);
	}
	if (ptr->argc) {
		myfree(ptr->argv);
		ptr->argc = 0;
	}
}

int args_build(Args *args, const char *prog, const char *version, const char *author,
			   const u32 min_args, const u32 max_args) {
	if (prog == NULL || version == NULL || author == NULL || min_args > max_args) {
		errno = EINVAL;
		return -1;
	}

	if (strlen(prog) > ARGS_MAX_DETAIL_LENGTH || strlen(version) > ARGS_MAX_DETAIL_LENGTH ||
		strlen(author) > ARGS_MAX_DETAIL_LENGTH) {
		errno = EINVAL;
		return -1;
	}

	// first subcommand is our params (set it up)
	args->subs_count = 0;
	args->subs = NULL;

	SubCommand sc;
	if (sub_command_build(&sc, "", "", min_args, max_args, ""))
		return -1;
	args_add_sub_command(args, &sc);

	strcpy(args->prog, prog);
	strcpy(args->version, version);
	strcpy(args->author, author);

	args->argv = NULL;
	args->argc = 0;

	return 0;
}

void args_exit_error(const Args *args, char *format, ...) {
	va_list va_args;
	va_start(va_args, format);
	fprintf(stderr, "%sError%s: ", BRIGHT_RED, RESET);
	vfprintf(stderr, format, va_args);
	fprintf(stderr, "\n\n");
	fprintf(stderr,
			"%sUSAGE%s:\n    %s%s%s [%sOPTIONS%s]\n\nFor more information "
			"try %s--help%s\n",
			DIMMED, RESET, BRIGHT_RED, args->prog, RESET, DIMMED, RESET, GREEN, RESET);
	va_end(va_args);
	exit(-1);
}

int args_add_param(Args *args, const ArgsParam *ap) {
	return sub_command_add_param(&args->subs[0], ap);
}
int args_add_sub_command(Args *args, SubCommand *sc) {
	if (args->subs_count == 0) {
		args->subs = mymalloc(sizeof(SubCommand));
		if (args->subs == NULL)
			return -1;
	} else {
		void *tmp = myrealloc(args->subs, sizeof(SubCommand) * (args->subs_count + 1));
		if (tmp == NULL)
			return -1;
		args->subs = tmp;
	}

	bool ret = sub_command_copy(&args->subs[args->subs_count], sc);
	printf("add com %i\n", ret);
	if (ret)
		args->subs_count++;

	if (ret)
		return 0;
	else
		return -1;
}

u64 args_subi_for(const Args *args, const char *sub) {
	u64 subi = 0;
	for (u64 i = 1; i < args->subs_count; i++) {
		if (!strcmp(sub, args->subs[i].name)) {
			subi = i;
		}
	}
	return subi;
}

bool args_sub_takes_value(const Args *args, u64 subi, const char *name, bool is_short) {
	for (u64 i = 0; i < args->subs[subi].param_count; i++) {
		if (is_short && !strcmp(name, args->subs[subi].params[i].short_name)) {
			if (args->subs[subi].params[i].takes_value) {
				return true;
			} else
				break;
		} else if (!strcmp(name, args->subs[subi].params[i].name)) {
			if (args->subs[subi].params[i].takes_value) {
				return true;
			} else
				break;
		}
	}
	return false;
}

bool args_check_option(const Args *args, u64 subi, const char *name, bool is_short,
					   const char *argv) {
	bool found = false;
	for (u64 j = 0; j < args->subs[subi].param_count; j++) {
		bool multi = args->subs[subi].params[j].multiple;
		if (is_short && !strcmp(args->subs[subi].params[j].short_name, name)) {
			found = true;
			if (args->subs[subi].is_specified[j] && !multi)
				args_exit_error(args, "Option: %s was spsecified more than once ", argv);
			args->subs[subi].is_specified[j] = true;
			break;
		} else if (!is_short && !strcmp(args->subs[subi].params[j].name, name)) {
			found = true;
			if (args->subs[subi].is_specified[j] && !multi)
				args_exit_error(args, "Option: %s was spsecified more than once ", argv);
			args->subs[subi].is_specified[j] = true;
			break;
		}
	}
	return found;
}

void args_check_validity(const Args *args, int argc, const char **argv) {
	// check if there's a sub command and check arg count
	const char *sub = NULL;
	u32 arg_count = 0;
	u64 subi = 0;
	u64 sub_arg = UINT64_MAX;
	for (u64 i = 1; i < argc; i++) {
		u64 len = strlen(argv[i]);
		if (len > 0 && argv[i][0] == '-') {
			char name[len];
			bool is_short;
			bool found = false;
			if (len > 1 && args->argv[i][1] == '-') {
				strcpy(name, args->argv[i] + 2);
				is_short = false;
			} else {
				strcpy(name, args->argv[i] + 1);
				is_short = true;
			}
			if (args_sub_takes_value(args, subi, name, is_short)) {
				i += 1;
			}
		} else {
			if (!sub && args->subs_count > 1) {
				sub = argv[i];
				sub_arg = i;
				subi = args_subi_for(args, sub);
			} else
				arg_count += 1;
		}
	}

	u64 sub_index = 0;
	if (sub) {
		// there's a sub so validate it
		bool valid = false;
		for (u64 i = 1; i < args->subs_count; i++) {
			if (!strcmp(sub, args->subs[i].name)) {
				valid = true;
				if (arg_count > args->subs[i].max_args || arg_count < args->subs[i].min_args) {
					args_exit_error(args,
									"Incorrect number of "
									"arguments for sub command '%s' "
									"(%i specified). "
									"Number of arguments must be "
									"between %i and %i.",
									args->subs[i].name, arg_count, args->subs[i].min_args,
									args->subs[i].max_args);
				}
				sub_index = i;

				break;
			}
		}
		if (!valid) {
			args_exit_error(args, "Unknown SubCommand \"%s\"", sub);
		}
	} else if (args->subs_count > 1) {
		args_usage(args, NULL);
	} else {
		// check number of args
		if (arg_count > args->subs[0].max_args || arg_count < args->subs[0].min_args) {
			args_exit_error(args,
							"Incorrect number of arguments "
							"(%i specified). "
							"Number of arguments must be "
							"between %i and %i.",
							arg_count, args->subs[0].min_args, args->subs[0].max_args);
		}
	}

	// check options
	for (u64 i = 1; i < argc; i++) {
		u64 subi;
		if (i <= sub_arg) {
			subi = 0;
		} else {
			subi = sub_index;
		}

		u64 len = strlen(argv[i]);
		if (len > 0 && argv[i][0] == '-') {
			// option to check
			char name[len];
			bool is_short;
			bool found = false;
			if (len > 1 && args->argv[i][1] == '-') {
				strcpy(name, args->argv[i] + 2);
				is_short = false;
			} else {
				strcpy(name, args->argv[i] + 1);
				is_short = true;
			}

			found = args_check_option(args, subi, name, is_short, argv[i]);
			if (args_sub_takes_value(args, subi, name, is_short)) {
				i += 1;
			}
			if (!found) {
				if (subi == 0) {
					args_exit_error(args, "Unknown option: %s", argv[i]);
				} else {
					args_exit_error(args,
									"Unknown option: %s. Not valid for "
									"SubCommand \"%s\".",
									argv[i], args->subs[subi].name);
				}
			}
		}
	}
}

int args_init(Args *args, const int argc, const char **argv) {
	args->argc = argc;
	args->argv = mymalloc(sizeof(char *) * argc);
	if (args->argv == NULL)
		return -1;
	for (u64 i = 0; i < argc; i++) {
		args->argv[i] = mymalloc(sizeof(char) * (strlen(argv[i]) + 1));
		if (args->argv[i] == NULL) {
			for (u64 j = 0; j < i; j++) {
				myfree(args->argv[j]);
			}
			myfree(args->argv);
			return -1;
		}
		strcpy(args->argv[i], argv[i]);
	}

	for (u64 i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-V")) {
			args_print_version(args);
		}
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
			const char *sub = NULL;
			for (u64 j = 1; j < i; j++) {
				u64 len = strlen(argv[j]);
				if (len > 0) {
					if (argv[j][0] != '-') {
						sub = argv[j];
						break;
					} else {
						char name[len];
						bool is_short;
						if (len > 1 && args->argv[j][1] == '-') {
							strcpy(name, args->argv[j] + 2);
							is_short = false;
						} else {
							strcpy(name, args->argv[j] + 1);
							is_short = true;
						}

						u64 subi = 0;
						if (sub)
							subi = args_subi_for(args, sub);

						if (args_sub_takes_value(args, subi, name, is_short)) {
							j += 1;
						}
					}
				}
			}
			args_usage(args, sub);
		}
	}

	args_check_validity(args, argc, argv);

	return 0;
}

int args_value_of(const Args *args, const char *param_name, char *value_buf,
				  const u64 max_value_len, const u32 index) {
	return 0;
}
int args_get_argument(const Args *args, const u32 index, char *value_buf, const u64 max_value_len) {
	return 0;
}
void args_print_version(const Args *args) {
	const char *prog = args->prog;
	const char *version = args->version;
	fprintf(stderr, "%s%s%s %s%s%s\n", BRIGHT_RED, prog, RESET, CYAN, version, RESET);
	exit(0);
}

void args_usage(const Args *args, const char *sub) {
	printf("todo: args usage\n");
}
