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
#include <base/misc.h>
#include <base/resources.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool args_param_copy(ArgsParam *dst, const ArgsParam *src) {
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
		print_error("Input may not be NULL");
		errno = EINVAL;
		return -1;
	}

	if (strlen(name) > ARGS_MAX_ARGUMENT_NAME_LENGTH ||
		strlen(short_name) > ARGS_MAX_ARGUMENT_NAME_LENGTH) {
		print_error("Input too long");
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
		print_error("Input may not be NULL");
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
	if (src->help == NULL)
		return false;
	dst->help = mymalloc(strlen(src->help) + 1);
	if (dst->help == NULL)
		return false;

	if (src->arg_doc) {
		dst->arg_doc = mymalloc(strlen(src->arg_doc) + 1);
		if (dst->arg_doc == NULL) {
			myfree(dst->help);
			return false;
		}
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
			return false;
		}

		memcpy(dst->is_specified, src->is_specified, sizeof(bool) * src->param_count);
		for (u32 i = 0; i < src->param_count; i++) {
			args_param_copy(&dst->params[i], &src->params[i]);
		}
		dst->param_count = src->param_count;
	}

	strcpy(dst->name, src->name);
	strcpy(dst->help, src->help);
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
	if (sc == NULL || name == NULL || help == NULL || min_args > max_args) {
		errno = EINVAL;
		print_error("Invalid input");
		return -1;
	}
	sc->params = NULL;
	sc->help = NULL;
	sc->arg_doc = NULL;
	sc->is_specified = NULL;
	sc->param_count = 0;

	if (strlen(name) > ARGS_MAX_SUBCOMMAND_LENGTH || min_args > max_args) {
		print_error("Invalid input");
		errno = EINVAL;
		return -1;
	}

	if (help) {
		sc->help = mymalloc(sizeof(char) * strlen(help) + 1);
		if (sc->help == NULL) {
			print_error("Could not allocate sufficient memory");
			return -1;
		}
		strcpy(sc->help, help);
	}

	if (arg_doc) {
		sc->arg_doc = mymalloc(sizeof(char) * strlen(arg_doc) + 1);
		if (sc->arg_doc == NULL) {
			print_error("Could not allocate sufficient memory");
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
		if (tmp == NULL) {
			print_error("Could not allocate sufficient memory");
			return -1;
		}
		sc->params = tmp;
		tmp = myrealloc(sc->is_specified, sizeof(bool) * (sc->param_count + 1));
		if (tmp == NULL) {
			print_error("Could not allocate sufficient memory");
			return -1;
		}
		sc->is_specified = tmp;

		args_param_copy(&sc->params[sc->param_count], ap);
		sc->is_specified[sc->param_count] = false;

		sc->param_count += 1;
	} else {
		sc->params = mymalloc(sizeof(ArgsParam));
		sc->is_specified = mymalloc(sizeof(bool));

		if (sc->params == NULL || sc->is_specified == NULL) {
			if (sc->params) {
				myfree(sc->params);
				sc->params = NULL;
			}
			if (sc->is_specified) {
				myfree(sc->is_specified);
				sc->is_specified = NULL;
			}
			print_error("Could not allocate sufficient memory");
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
		sub_command_cleanup(&ptr->subs[i]);
	}
	ptr->subs_count = 0;
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
			   const u32 min_args, const u32 max_args, const char *arch) {
	if (prog == NULL || version == NULL || author == NULL) {
		print_error("Input may not be NULL");
		errno = EINVAL;
		return -1;
	}

	if (strlen(prog) > ARGS_MAX_DETAIL_LENGTH || strlen(version) > ARGS_MAX_DETAIL_LENGTH ||
		strlen(author) > ARGS_MAX_DETAIL_LENGTH) {
		print_error("Input too long");
		errno = EINVAL;
		return -1;
	}

	// first subcommand is our params (set it up)
	args->subs_count = 0;
	args->subs = NULL;

	SubCommandImpl sc;
	if (sub_command_build(&sc, "", "", min_args, max_args, "")) {
		args->subs_count = 0;
		args->subs = NULL;
		return -1;
	}
	args_add_sub_command(args, &sc);
	sub_command_cleanup(&sc);

	strcpy(args->prog, prog);
	strcpy(args->version, version);
	strcpy(args->author, author);

	if (arch != NULL)
		strcpy(args->arch, arch);
	else
		strcpy(args->arch, "");

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
	EXIT_ERR_IF_NO_DEBUG(-1);
}

int args_add_param(Args *args, const ArgsParam *ap) {
	return sub_command_add_param(&args->subs[0], ap);
}
int args_add_sub_command(Args *args, SubCommand *sc) {
	if (args->subs_count == 0) {
		args->subs = mymalloc(sizeof(SubCommand));
		if (args->subs == NULL) {
			print_error("Could not allocate sufficient memory");
			return -1;
		}
	} else {
		void *tmp = myrealloc(args->subs, sizeof(SubCommand) * (args->subs_count + 1));
		if (tmp == NULL) {
			print_error("Could not allocate sufficient memory");
			return -1;
		}
		args->subs = tmp;
	}

	bool ret = sub_command_copy(&args->subs[args->subs_count], sc);
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
		// no sub commands so we check the base min/max args.
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

void process_lines(Args *args, const char *arg1, char config_file[], size_t fsize) {

	args->argv = mymalloc(sizeof(char *));
	if (args->argv == NULL) {
		args_exit_error(args, "Could not allocate sufficient memory");
		return;
	}

	args->argv[0] = mymalloc(sizeof(char) * (strlen(arg1) + 1));
	if (args->argv[0] == NULL) {
		args_exit_error(args, "Could not allocate sufficient memory");
		return;
	}
	strcpy(args->argv[0], arg1);

	args->argc = 1;

	// Use strtok to split the string into lines
	char *line = strtok(config_file, "\n");

	while (line != NULL) {
		// allow for comments using '#'.
		for (int i = 0; i < strlen(line); i++) {
			if (line[i] == '#') {
				line[i] = 0;
				break;
			}
		}
		// trim
		char *trimmed = trim_whitespace(line);

		if (*trimmed == '\0') {
			line = strtok(NULL, "\n"); // Skip empty lines
			continue;
		}

		void *tmp = myrealloc(args->argv, sizeof(char *) * (args->argc + 1));
		if (tmp == NULL) {
			args_exit_error(args, "Could not allocate sufficient memory");
			return;
		}
		args->argv = tmp;
		args->argv[args->argc] = mymalloc(sizeof(char) * (strlen(trimmed) + 1));
		if (args->argv[args->argc] == NULL) {
			args_exit_error(args, "Could not allocate sufficient memory");
			return;
		}
		strcpy(args->argv[args->argc], trimmed);
		args->argc++;

		// Get the next line
		line = strtok(NULL, "\n");
	}
}

// Return 1 on successful file config created
// Return 0 if no file was specified
// Return -1 if an error occurs
int check_file_config(Args *args, const int argc, const char **argv) {
	// if file config is used there must be exactly two arguments fam @config.txt for example
	if (argc != 2)
		return 0;

	// If the second argument does not start with an '@' symbol it is not a file.
	u64 arg_len = strlen(argv[1]);
	if (arg_len > 0 && argv[1][0] != '@') {
		return 0;
	}

	// We have a file config process it.
	// get file size
	const char *file_name = argv[1] + 1;

	Path path;
	path_for(&path, file_name);
	if (!path_exists(&path) || path_is_dir(&path)) {
		args_exit_error(args, "File not found at '%s'", path_to_string(&path));
		return -1;
	}
	u64 fsize = path_file_size(&path);

	char config_file[fsize + 1];
	MYFILE *fp = myfopen(&path, "r");
	read_all(config_file, 1, fsize, fp);
	config_file[fsize] = 0;
	myfclose(fp);

	process_lines(args, argv[0], config_file, fsize);

	return 1;
}

void args_init(Args *args, const int argc, const char **argv) {
	// check for NULLs (not allowed)
	for (int i = 0; i < argc; i++) {
		if (argv[i] == NULL) {
			args_exit_error(args, "argv may not have NULL value where i < argc");
			return;
		}
	}
	int r;
	if (check_file_config(args, argc, argv)) {
		// if true the file config as been loaded
		return;
	}

	// scan args for an arg starting with '@'. Not valid here. It must be the first and only param.
	// Report error.

	// If any args start with '@' it's an error because we already checked for file config
	for (u64 i = 1; i < argc; i++) {
		u64 arg_len = strlen(argv[i]);
		if (arg_len > 0 && argv[i][0] == '@') {
			args_exit_error(args,
							"File speicified with the '@' symbol must be the first argument.");
			return;
		}
	}

	args->argc = argc;
	args->argv = mymalloc(sizeof(char *) * argc);
	if (args->argv == NULL) {
		args->argc = 0;
		args_exit_error(args, "Could not allocate sufficient memory");
		return;
	}
	for (u64 i = 0; i < argc; i++) {
		args->argv[i] = mymalloc(sizeof(char) * (strlen(argv[i]) + 1));
		if (args->argv[i] == NULL) {
			args->argc = 0;
			myfree(args->argv);
			args_exit_error(args, "Could not allocate sufficient memory");
			return;
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
						char name[len + 1];
						bool is_short;
						if (len > 1 && args->argv[j][1] == '-') {
							strcpy(name, args->argv[j] + 2);
							is_short = false;
						} else {
							strcpy(name, args->argv[j] + 1);
							is_short = true;
						}

						u64 subi = 0;
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
}

// Returns -2 if an error occurs and sets errno.
// Returns -1 if the value is not present.
// Returns 0 if the value is found (flags or max_value_len == 0)
// Returns the length of the value (takes_value true and max_value_len > 0)
int args_value_of(const Args *args, const char *param_name, char *value_buf,
				  const u64 max_value_len, const u32 index) {
	if (args == NULL || param_name == NULL) {
		errno = EINVAL;
		print_error("Input may not be NULL");
		return -2;
	}
	u64 plen = strlen(param_name);

	char param_name_buf[plen + 3];
	strcpy(param_name_buf, "--");
	strcat(param_name_buf, param_name);

	char short_name_buf[plen + 3];
	strcpy(short_name_buf, "-");
	bool found = false;
	char *default_value = NULL;
	bool takes_value = false;

	// linear search through all elements. Potential to improve here.
	for (u64 i = 0; i < args->subs_count; i++) {
		for (u64 j = 0; j < args->subs[i].param_count; j++) {
			if (!strcmp(args->subs[i].params[j].name, param_name)) {
				found = true;
				strcat(short_name_buf, args->subs[i].params[j].short_name);
				default_value = args->subs[i].params[j].default_value;
				takes_value = args->subs[i].params[j].takes_value;
			}
		}
	}

	if (!found) {
		print_error("Unknown parameter tested");
		errno = ENOENT;
		return -2;
	}
	u64 itt_index = 0;
	for (u64 i = 1; i < args->argc; i++) {
		if (!strcmp(args->argv[i], param_name_buf) || !strcmp(args->argv[i], short_name_buf)) {
			if (itt_index == index) {
				if (takes_value && i + 1 < args->argc) {
					if (value_buf == NULL) {
						errno = EINVAL;
						print_error("Input may not be NULL");
						return -2;
					}
					return snprintf(value_buf, max_value_len, "%s", args->argv[i + 1]);
				} else if (takes_value && max_value_len > 0) {
					if (value_buf == NULL) {
						errno = EINVAL;
						print_error("Input may not be NULL");
						return -2;
					}
					strcpy(value_buf, "");
					return 0;
				} else {
					return 0;
				}
			} else {
				itt_index += 1;
			}
		}
	}

	if (default_value != NULL && index == 0) {
		if (value_buf == NULL && max_value_len > 0) {
			errno = EINVAL;
			print_error("Input may not be NULL");
			return -2;
		}
		return snprintf(value_buf, max_value_len, "%s", default_value);
	}

	return -1;
}

int args_get_argument(const Args *args, const u32 index, char *value_buf, const u64 max_value_len) {
	if (args == NULL) {
		print_error("Input may not be NULL");
		errno = EINVAL;
		return -2;
	}
	u64 index_itt = 0;
	for (u64 i = 1; i < args->argc; i++) {
		i32 len = strlen(args->argv[i]);
		if (len > 1 && args->argv[i][0] == '-') {
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

			// linear search through all elements. Potential to improve here.
			for (u64 k = 0; k < args->subs_count; k++) {
				for (u64 j = 0; j < args->subs[k].param_count; j++) {
					if (is_short && !strcmp(name, args->subs[k].params[j].short_name)) {
						if (args->subs[k].params[j].takes_value) {
							i += 1;
							found = true;
							break;
						}
					} else if (!strcmp(name, args->subs[k].params[j].name)) {
						if (args->subs[k].params[j].takes_value) {
							i += 1;
							found = true;
							break;
						}
					}
				}
			}
		} else {
			if (index_itt == index) {
				if (value_buf == NULL && max_value_len != 0) {
					print_error("Input may not be NULL");
					errno = EINVAL;
					return -2;
				}
				return snprintf(value_buf, max_value_len, "%s", args->argv[i]);
			}
			index_itt += 1;
		}
	}
	return -1;
}

void args_print_version(const Args *args) {
	const char *prog = args->prog;
	const char *version = args->version;
	const char *arch = args->arch;
	if (strlen(arch) == 0) {
		arch = "";
	}
	fprintf(stderr, "%s%s%s %s%s%s (%s%s%s)\n", BRIGHT_RED, prog, RESET, CYAN, version, RESET,
			YELLOW, arch, RESET);
	EXIT_ERR_IF_NO_DEBUG(0);
}

void args_usage(const Args *args, const char *sub_command) {
	bool found = false;
	u64 subs_count = args->subs_count;
	u64 sub_index = 0;

	if (sub_command) {
		for (u64 i = 1; i < subs_count; i++) {
			char *name = args->subs[i].name;
			if (!strcmp(name, sub_command)) {
				sub_index = i;
				found = true;
			}
		}
	} else // general help displayed
		found = true;

	if (!found) {
		args_exit_error(args, "Unknown SubCommand \"%s\"", sub_command);
	}

	u64 count = args->subs[0].param_count;
	u64 max_len = 0;
	for (u64 i = 0; i < count; i++) {
		bool takes_value = args->subs[0].params[i].takes_value;
		bool multi = args->subs[0].params[i].multiple;
		char *name = args->subs[0].params[i].name;
		char *short_name = args->subs[0].params[i].short_name;
		u64 len;
		if (!takes_value)
			len = snprintf(NULL, 0, "    -%s, --%s", short_name, name);
		else if (multi)
			len = snprintf(NULL, 0, "    -%s, --%s (<%s>, ...)", short_name, name, name);
		else
			len = snprintf(NULL, 0, "    -%s, --%s <%s>", short_name, name, name);
		if (len > max_len)
			max_len = len;
	}

	max_len += 4;

	if (max_len < 17)
		max_len = 17;

	const char *prog = args->prog;
	const char *author = args->author;
	const char *version = args->version;
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
	char *sub_arg_doc_str = "";

	if (sub_command) {
		snprintf(sub_command_str, sub_command_str_len + 30, "%s%s%s", BRIGHT_RED, sub_command,
				 RESET);
		sub_arg_doc_str = args->subs[sub_index].arg_doc;
	} else {
		snprintf(sub_command_str, sub_command_str_len + 30, "[%sSUB_COMMAND%s]", DIMMED, RESET);
	}

	fprintf(stderr,
			"%s%s%s %s%s%s\n%s%s%s\n\n%sUSAGE%s:\n    %s%s%s "
			"[%sCORE_OPTIONS%s] %s [%sSUB_OPTIONS%s] %s\n\n"
			"%sCORE_FLAGS%s:\n"
			"    %s-h%s, %s--help%s%sPrints help information\n"
			"    %s-V%s, %s--version%s%sPrints version "
			"information\n",
			CYAN, prog, RESET, YELLOW, version, RESET, GREEN, author, RESET, DIMMED, RESET,
			BRIGHT_RED, prog, RESET, DIMMED, RESET, sub_command_str, DIMMED, RESET, sub_arg_doc_str,
			DIMMED, RESET, CYAN, RESET, YELLOW, RESET, buffer, CYAN, RESET, YELLOW, RESET, buffer2);

	for (u64 i = 0; i < count; i++) {
		bool takes_value = args->subs[0].params[i].takes_value;
		if (!takes_value) {
			char *name = args->subs[0].params[i].name;
			char *short_name = args->subs[0].params[i].short_name;
			char *help = args->subs[0].params[i].help;
			u64 len = snprintf(NULL, 0, "    -%s, --%s", short_name, name);
			if (len > max_len)
				len = max_len;
			u64 i;
			for (i = 0; i < max_len - len && i < 1024; i++)
				buffer[i] = ' ';
			buffer[i] = 0;
			fprintf(stderr, "    %s-%s%s, %s--%s%s %s%s\n", CYAN, short_name, RESET, YELLOW, name,
					RESET, buffer, help);
		}
	}

	if (count) {
		fprintf(stderr, "\n%sCORE_OPTIONS%s:\n", DIMMED, RESET);

		for (u64 i = 0; i < count; i++) {
			bool takes_value = args->subs[0].params[i].takes_value;
			if (takes_value) {
				char *name = args->subs[0].params[i].name;
				char *short_name = args->subs[0].params[i].short_name;
				char *help = args->subs[0].params[i].help;
				bool multi = args->subs[0].params[i].multiple;
				char *default_value = args->subs[0].params[i].default_value;
				u64 default_value_str_len;
				if (default_value == NULL) {
					default_value_str_len = 1;
				} else {
					default_value_str_len = strlen(default_value) + 100;
				}
				char default_value_str[default_value_str_len];
				if (default_value == NULL)
					strcpy(default_value_str, "");
				else
					snprintf(default_value_str, default_value_str_len, " (default value: '%s')",
							 default_value);

				if (multi) {
					u64 len = snprintf(NULL, 0, "    -%s, --%s <%s>, ...", short_name, name, name);
					if (len > max_len)
						len = max_len;
					u64 i;
					for (i = 0; i < max_len - len && i < 1024; i++)
						buffer[i] = ' ';
					buffer[i] = 0;
					fprintf(stderr,
							"    %s-%s%s, %s--%s%s "
							"<%s>, "
							"... %s%s%s\n",
							CYAN, short_name, RESET, YELLOW, name, RESET, name, buffer, help,
							default_value_str);
				} else {

					u64 len = snprintf(NULL, 0, "    -%s, --%s <%s>", short_name, name, name);
					if (len > max_len)
						len = max_len;
					u64 i;
					for (i = 0; i < max_len - len && i < 1024; i++)
						buffer[i] = ' ';
					buffer[i] = 0;

					fprintf(stderr,
							"    %s-%s%s, %s--%s%s "
							"<%s> "
							"%s%s%s\n",
							CYAN, short_name, RESET, YELLOW, name, RESET, name, buffer, help,
							default_value_str);
				}
			}
		}
	}

	if (subs_count > 1 && sub_command == NULL) {
		fprintf(stderr, "\n%sSUB_COMMANDS%s:\n", DIMMED, RESET);
		for (u64 i = 1; i < subs_count; i++) {

			char *name = args->subs[i].name;
			char *help = args->subs[i].help;
			char *arg_doc = args->subs[i].arg_doc;

			u64 len = strlen(name) + strlen(arg_doc) + 4;
			if (len > max_len)
				len = max_len;
			char buffer[1025];
			u64 j;
			for (j = 0; j < (max_len - len) && j < 1024; j++)
				buffer[j] = ' ';
			buffer[j] = 0;

			fprintf(stderr, "    %s%s%s %s%s%s\n", CYAN, name, RESET, arg_doc, buffer, help);
		}
	}
	if (sub_command) {
		u64 param_index = 0;
		for (u64 i = 1; i < subs_count; i++) {
			char *name = args->subs[i].name;
			if (!strcmp(name, sub_command)) {
				param_index = i;
			}
		}

		if (param_index > 0) {
			count = args->subs[param_index].param_count;
			fprintf(stderr, "\n%sSUB_FLAGS%s (%s%s%s):\n", DIMMED, RESET, BRIGHT_RED, sub_command,
					RESET);

			for (u64 i = 0; i < count; i++) {
				char *name = args->subs[param_index].params[i].name;
				char *short_name = args->subs[param_index].params[i].short_name;
				bool takes_value = args->subs[param_index].params[i].takes_value;
				char *help = args->subs[param_index].params[i].help;

				if (!takes_value) {
					u64 len = strlen(name) + 10;
					if (len > max_len)
						len = max_len;
					char buffer[1025];
					u64 j;
					for (j = 0; j < (max_len - len) && j < 1024; j++)
						buffer[j] = ' ';
					buffer[j] = 0;
					fprintf(stderr,
							"    %s-%s%s, "
							"%s--%s%s%s %s\n",
							CYAN, short_name, RESET, YELLOW, name, RESET, buffer, help);
				}
			}
			fprintf(stderr, "\n%sSUB_OPTIONS%s (%s%s%s):\n", DIMMED, RESET, BRIGHT_RED, sub_command,
					RESET);
			for (u64 i = 0; i < count; i++) {
				char *name = args->subs[param_index].params[i].name;
				char *short_name = args->subs[param_index].params[i].short_name;
				bool takes_value = args->subs[param_index].params[i].takes_value;
				char *help = args->subs[param_index].params[i].help;
				bool multiple = args->subs[param_index].params[i].multiple;

				char *default_value = args->subs[param_index].params[i].default_value;
				u64 default_value_str_len;
				if (default_value == NULL) {
					default_value_str_len = 1;
				} else {
					default_value_str_len = strlen(default_value) + 100;
				}
				char default_value_str[default_value_str_len];
				if (default_value == NULL)
					strcpy(default_value_str, "");
				else
					snprintf(default_value_str, default_value_str_len, " (default value: %s)",
							 default_value);

				if (takes_value) {
					if (multiple) {
						u64 len = 2 * strlen(name) + 19;
						char buffer[1025];
						u64 j;
						if (len > max_len)
							len = max_len;
						for (j = 0; j < (max_len - len) && j < 1024; j++)
							buffer[j] = ' ';
						buffer[j] = ' ';
						buffer[j + 1] = 0;
						fprintf(stderr,
								"    %s-%s%s, "
								"%s--%s%s "
								"<%s>, ...%s %s"
								"%s\n",
								CYAN, short_name, RESET, YELLOW, name, RESET, name, buffer, help,
								default_value_str);
					} else {
						u64 len = 2 * strlen(name) + 13;
						char buffer[1025];
						u64 j;
						if (len > max_len)
							len = max_len;
						for (j = 0; j < (max_len - len) && j < 1024; j++)
							buffer[j] = ' ';
						buffer[j] = 0;
						fprintf(stderr,
								"    %s-%s%s, "
								"%s--%s%s "
								"<%s>%s "
								"%s%s\n",
								CYAN, short_name, RESET, YELLOW, name, RESET, name, buffer, help,
								default_value_str);
					}
				}
			}
		}
	}

	EXIT_ERR_IF_NO_DEBUG(0);
}
