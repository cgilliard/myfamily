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
#include <core/colors.h>
#include <core/resources.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int args_param_build(ArgsParam *ap, char *name, char *help, char *short_name,
		     bool takes_value, bool multiple, char *default_value) {
	if (ap == NULL || name == NULL || help == NULL || short_name == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (strlen(short_name) > strlen(name)) {
		fprintf(stderr, "short_name cannot be longer than name\n");
		return -1;
	}

	ap->name = NULL;
	ap->help = NULL;
	ap->short_name = NULL;
	ap->default_value = NULL;

	ap->name = mymalloc((1 + strlen(name)) * sizeof(char));
	if (ap->name == NULL) {
		return -1;
	}
	strcpy(ap->name, name);

	ap->help = mymalloc((1 + strlen(help)) * sizeof(char));
	if (ap->help == NULL) {
		args_param_cleanup(ap);
		return -1;
	}
	strcpy(ap->help, help);

	ap->short_name = mymalloc((1 + strlen(short_name)) * sizeof(char));
	if (ap->short_name == NULL) {
		args_param_cleanup(ap);
		return -1;
	}
	strcpy(ap->short_name, short_name);

	ap->takes_value = takes_value;
	ap->multiple = multiple;

	if (default_value) {
		ap->default_value =
		    mymalloc((1 + strlen(default_value)) * sizeof(char));
		if (ap->default_value == NULL) {
			args_param_cleanup(ap);
			return -1;
		}
		strcpy(ap->default_value, default_value);
	} else
		ap->default_value = NULL;

	return 0;
}
void args_param_cleanup(ArgsParam *ap) {
	if (ap->name) {
		myfree(ap->name);
		ap->name = NULL;
	}
	if (ap->short_name) {
		myfree(ap->short_name);
		ap->short_name = NULL;
	}
	if (ap->help) {
		myfree(ap->help);
		ap->help = NULL;
	}
	if (ap->default_value) {
		myfree(ap->default_value);
		ap->default_value = NULL;
	}
}
int sub_command_build(SubCommand *sc, char *name, char *help, u32 min_args,
		      u32 max_args) {
	if (name == NULL || help == NULL || min_args > max_args) {
		errno = EINVAL;
		return -1;
	}

	sc->name = NULL;
	sc->help = NULL;
	sc->params_count = 0;

	sc->name = mymalloc((1 + strlen(name)) * sizeof(char));
	if (sc->name == NULL) {
		sub_command_cleanup(sc);
		return -1;
	}
	strcpy(sc->name, name);

	sc->help = mymalloc((1 + strlen(help)) * sizeof(char));
	if (sc->help == NULL) {
		sub_command_cleanup(sc);
		return -1;
	}
	strcpy(sc->help, help);

	sc->min_args = min_args;
	sc->max_args = max_args;

	return 0;
}
void sub_command_cleanup(SubCommand *sc) {
	if (sc->name) {
		myfree(sc->name);
		sc->name = NULL;
	}

	if (sc->help) {
		myfree(sc->help);
		sc->help = NULL;
	}

	if (sc->params_count != 0) {
		for (u64 i = 0; i < sc->params_count; i++) {
			args_param_cleanup(&sc->params[i]);
		}

		myfree(sc->params);
		myfree(sc->params_state);
		sc->params = NULL;
		sc->params_state = NULL;
		sc->params_count = 0;
	}
}
int sub_command_add_param(SubCommand *sc, ArgsParam *ap) {
	if (sc->params_count == 0) {
		sc->params = mymalloc(sizeof(ArgsParam));
		sc->params_state = mymalloc(sizeof(ArgsParamState));

		if (sc->params == NULL || sc->params_state == NULL) {

			sub_command_cleanup(sc);
			return -1;
		}
		sc->params_count = 1;
	} else {
		sc->params_count += 1;
		void *tmp1 =
		    myrealloc(sc->params, sizeof(ArgsParam) * sc->params_count);
		void *tmp2 =
		    myrealloc(sc->params_state,
			      sizeof(ArgsParamState) * sc->params_count);

		sc->params = tmp1;
		sc->params_state = tmp2;
	}
	u64 index = sc->params_count - 1;

	sc->params[index].name =
	    mymalloc((strlen(ap->name) + 1) * sizeof(char));
	strcpy(sc->params[index].name, ap->name);

	sc->params[index].help =
	    mymalloc((strlen(ap->help) + 1) * sizeof(char));
	strcpy(sc->params[index].help, ap->help);

	sc->params[index].short_name =
	    mymalloc((strlen(ap->short_name) + 1) * sizeof(char));
	strcpy(sc->params[index].short_name, ap->short_name);

	sc->params[index].takes_value = ap->takes_value;
	sc->params[index].multiple = ap->multiple;

	if (ap->default_value) {
		sc->params[index].default_value =
		    mymalloc((strlen(ap->default_value) + 1) * sizeof(char));
		if (sc->params[index].default_value == NULL) {
			sub_command_cleanup(sc);
			return -1;
		}
		strcpy(sc->params[index].default_value, ap->default_value);
	} else {
		sc->params[index].default_value = NULL;
	}

	sc->params_state[index].specified = false;
	sc->params_state[index].itt = 0;

	if (sc->params[index].name == NULL || sc->params[index].help == NULL ||
	    sc->params[index].short_name == NULL) {
		sub_command_cleanup(sc);
		return -1;
	}

	return 0;
}

int args_build(Args *args, char *prog, char *version, char *author,
	       u32 min_args, u32 max_args, u64 debug_flags) {
	if (prog == NULL && version == NULL && author == NULL) {
		errno = EINVAL;
		return -1;
	}
	args->subs_count = 0;
	args->argc = 0;
	args->debug_flags = debug_flags;

	args->prog = mymalloc(sizeof(char) * (1 + strlen(prog)));
	args->version = mymalloc(sizeof(char) * (1 + strlen(version)));
	args->author = mymalloc(sizeof(char) * (1 + strlen(author)));

	if (args->prog == NULL || args->version == NULL ||
	    args->author == NULL) {
		args_cleanup(args);
		return -1;
	}
	strcpy(args->prog, prog);
	strcpy(args->version, version);
	strcpy(args->author, author);

	// first subcommand is our params
	SubCommand sc;
	sub_command_build(&sc, "", "", min_args, max_args);
	args_add_sub_command(args, &sc);
	sub_command_cleanup(&sc);

	return 0;
}

void args_cleanup(Args *args) {
	if (args->argc) {
		for (u64 i = 0; i < args->argc; i++) {
			myfree(args->argv[i]);
		}
		myfree(args->argv);
		args->argc = 0;
	}

	if (args->prog) {
		myfree(args->prog);
		args->prog = NULL;
	}

	if (args->author) {
		myfree(args->author);
		args->author = NULL;
	}

	if (args->version) {
		myfree(args->version);
		args->version = NULL;
	}

	if (args->subs_count) {
		for (u64 i = 0; i < args->subs_count; i++) {
			sub_command_cleanup(args->subs[i]);
			myfree(args->subs[i]);
		}

		myfree(args->subs);
	}
}
int args_add_param(Args *args, ArgsParam *ap) {
	return sub_command_add_param(args->subs[0], ap);
}

int args_add_sub_command(Args *args, SubCommand *sc) {
	if (args->subs_count == 0) {
		args->subs = mymalloc(sizeof(SubCommand *));
		if (args->subs == NULL)
			return -1;
	} else {
		SubCommand **tmp = myrealloc(
		    args->subs, sizeof(SubCommand *) * (1 + args->subs_count));

		if (tmp == NULL) {
			return -1;
		}

		args->subs = tmp;
	}

	args->subs[args->subs_count] = mymalloc(sizeof(SubCommand));
	if (args->subs[args->subs_count] == NULL) {
		return -1;
	}

	sub_command_build(args->subs[args->subs_count], sc->name, sc->help,
			  sc->min_args, sc->max_args);
	for (u64 i = 0; i < sc->params_count; i++) {
		sub_command_add_param(args->subs[args->subs_count],
				      &sc->params[i]);
	}

	args->subs_count += 1;
	return 0;
}

bool args_takes_value(Args *args, char *name, bool is_short) {
	for (u64 k = 0; k < args->subs_count; k++) {
		for (u64 j = 0; j < args->subs[k]->params_count; j++) {
			if (is_short &&
			    !strcmp(name,
				    args->subs[k]->params[j].short_name)) {
				if (name,
				    args->subs[k]->params[j].takes_value) {
					return true;
				} else
					break;
			} else if (!strcmp(name,
					   args->subs[k]->params[j].name)) {
				if (name,
				    args->subs[k]->params[j].takes_value) {
					return true;
				} else
					break;
			}
		}
	}
	return false;
}

bool args_sub_takes_value(Args *args, u64 subi, char *name, bool is_short) {
	for (u64 j = 0; j < args->subs[subi]->params_count; j++) {
		if (is_short &&
		    !strcmp(name, args->subs[subi]->params[j].short_name)) {
			if (name, args->subs[subi]->params[j].takes_value) {
				return true;
			} else
				break;
		} else if (!strcmp(name, args->subs[subi]->params[j].name)) {
			if (name, args->subs[subi]->params[j].takes_value) {
				return true;
			} else
				break;
		}
	}
	return false;
}

void args_exit_error(Args *args, char *format, ...) {
	va_list va_args;
	va_start(va_args, format);
	fprintf(stderr, "%sError%s: ", BRIGHT_RED, RESET);
	vfprintf(stderr, format, va_args);
	fprintf(stderr, "\n\n");
	fprintf(stderr,
		"%sUSAGE%s:\n    %s%s%s [%sOPTIONS%s]\n\nFor more information "
		"try %s--help%s\n",
		DIMMED, RESET, BRIGHT_RED, args->prog, RESET, DIMMED, RESET,
		GREEN, RESET);
	va_end(va_args);
	u64 debug_flags = args->debug_flags;
	if (!(debug_flags & DEBUG_INIT_NO_EXIT))
		exit(-1);
}

void args_check_validity(Args *args, int argc, char **argv) {
	// check if there's a sub command and check arg count
	char *sub = NULL;
	u32 arg_count = 0;
	u64 sub_arg = ULONG_MAX;
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
			if (args_takes_value(args, name, is_short)) {
				i += 1;
			}
		} else {
			if (!sub && args->subs_count > 1) {
				sub = argv[i];
				sub_arg = i;
			} else
				arg_count += 1;
		}
	}

	u64 sub_index = 0;

	if (sub) {
		// there's a sub so validate it
		bool valid = false;
		for (u64 i = 1; i < args->subs_count; i++) {
			if (!strcmp(sub, args->subs[i]->name)) {
				valid = true;
				if (arg_count > args->subs[i]->max_args ||
				    arg_count < args->subs[i]->min_args) {
					args_exit_error(
					    args,
					    "Incorrect number of arguments for "
					    "sub command '%s' (%i "
					    "specified). "
					    "Number of arguments must be "
					    "between %i and %i.",
					    args->subs[i]->name, arg_count,
					    args->subs[i]->min_args,
					    args->subs[i]->max_args);
				}
				sub_index = i;

				break;
			}
		}
		if (!valid) {
			args_exit_error(args, "Unknown SubCommand \"%s\"", sub);
		}
	} else if (args->subs_count > 1) {
		args_exit_error(args, "SubCommand was not specified");
	} else {
		// check number of args
		if (arg_count > args->subs[0]->max_args ||
		    arg_count < args->subs[0]->min_args) {
			args_exit_error(
			    args,
			    "Incorrect number of arguments (%i specified). "
			    "Number of arguments must be between %i and %i.",
			    arg_count, args->subs[0]->min_args,
			    args->subs[0]->max_args);
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
			if (args_sub_takes_value(args, subi, name, is_short)) {

				for (u64 j = 0;
				     j < args->subs[subi]->params_count; j++) {
					bool multi = args->subs[subi]
							 ->params[j]
							 .multiple;
					if ((is_short &&
					     !strcmp(args->subs[subi]
							 ->params[j]
							 .short_name,
						     name)) ||
					    (!is_short &&
					     !strcmp(args->subs[subi]
							 ->params[j]
							 .name,
						     name))) {

						if (args->subs[subi]
							->params_state[j]
							.specified &&
						    !multi)
							args_exit_error(
							    args,
							    "Option: "
							    "%s was "
							    "specified "
							    "more than "
							    "once ",
							    argv[i]);
						args->subs[subi]
						    ->params_state[j]
						    .specified = true;
					}
				}

				i += 1;
			} else {
				bool multi =
				    args->subs[subi]->params[i].multiple;
				for (u64 j = 0;
				     j < args->subs[subi]->params_count; j++) {
					if (is_short && !strcmp(args->subs[subi]
								    ->params[j]
								    .short_name,
								name)) {
						found = true;
						if (args->subs[subi]
							->params_state[j]
							.specified &&
						    !multi)
							args_exit_error(
							    args,
							    "Op"
							    "ti"
							    "on"
							    ": "
							    "%s"
							    " w"
							    "as"
							    " s"
							    "pe"
							    "ci"
							    "fi"
							    "ed"
							    " "
							    "mo"
							    "re"
							    " t"
							    "ha"
							    "n "
							    "on"
							    "ce"
							    " ",
							    argv[i]);
						args->subs[subi]
						    ->params_state[j]
						    .specified = true;
						break;
					} else if (!is_short &&
						   !strcmp(args->subs[subi]
							       ->params[j]
							       .name,
							   name)) {
						found = true;
						if (args->subs[subi]
							->params_state[j]
							.specified &&
						    !multi)
							args_exit_error(
							    args,
							    "Op"
							    "ti"
							    "on"
							    ": "
							    "%s"
							    " w"
							    "as"
							    " s"
							    "pe"
							    "ci"
							    "fi"
							    "ed"
							    " "
							    "mo"
							    "re"
							    " t"
							    "ha"
							    "n "
							    "on"
							    "ce"
							    " ",
							    argv[i]);
						args->subs[subi]
						    ->params_state[j]
						    .specified = true;
						break;
					}
				}
				if (!found) {
					if (subi == 0) {
						args_exit_error(args,
								"Unknown "
								"option: "
								"%s",
								argv[i]);
					} else {
						args_exit_error(
						    args,
						    "Unknown "
						    "option: "
						    "%s. Not "
						    "valid for "
						    "SubCommand"
						    " "
						    "\"%s\".",
						    argv[i],
						    args->subs[subi]->name);
					}
				}
			}
		}
	}
}

int args_init(Args *args, int argc, char **argv) {
	args->argc = argc;
	args->argv = mymalloc(sizeof(char *) * argc);

	if (args->argv == NULL) {
		args->argc = 0;
		return -1;
	}

	for (u64 i = 0; i < argc; i++) {
		args->argv[i] = mymalloc(sizeof(char) * (strlen(argv[i]) + 1));
		if (args->argv[i]) {
			strcpy(args->argv[i], argv[i]);
		} else {
			for (u64 j = i - 1; j >= 0; j--) {
				myfree(args->argv[j]);
			}
			myfree(args->argv);
			args->argc = 0;
			return -1;
		}
	}

	for (u64 i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-V")) {
			args_print_version(args);
		}
		if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
			char *sub = NULL;
			for (u64 j = 1; j < i; j++) {
				u64 len = strlen(argv[j]);
				if (len > 0) {
					if (argv[j][0] != '-') {
						sub = argv[j];
						break;
					} else {
						char name[len];
						bool is_short;
						if (len > 1 &&
						    args->argv[j][1] == '-') {
							strcpy(name,
							       args->argv[j] +
								   2);
							is_short = false;
						} else {
							strcpy(name,
							       args->argv[j] +
								   1);
							is_short = true;
						}

						if (args_takes_value(
							args, name, is_short)) {
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

int args_value_of(Args *args, char *param_name, char *value_buf,
		  u64 max_value_len, u64 index) {
	if (args == NULL || param_name == NULL || value_buf == NULL) {
		errno = EINVAL;
		return -1;
	}

	char param_name_buf[strlen(param_name) + 3];
	strcpy(param_name_buf, "--");
	strcat(param_name_buf, param_name);

	char short_name_buf[strlen(param_name) + 3];
	strcpy(short_name_buf, "-");
	bool found = false;
	char *default_value = NULL;
	bool takes_value = false;
	for (u64 i = 0; i < args->subs_count; i++) {
		for (u64 j = 0; j < args->subs[i]->params_count; j++) {
			if (!strcmp(args->subs[i]->params[j].name,
				    param_name)) {
				found = true;
				strcat(short_name_buf,
				       args->subs[i]->params[j].short_name);
				default_value =
				    args->subs[i]->params[j].default_value;
				takes_value =
				    args->subs[i]->params[j].takes_value;
			}
		}
	}

	if (!found) {
		errno = ENOENT;
		return -1;
	}

	u64 itt_index = 0;
	for (u64 i = 1; i < args->argc; i++) {
		if (!strcmp(args->argv[i], param_name_buf) ||
		    !strcmp(args->argv[i], short_name_buf)) {
			if (itt_index == index) {
				if (takes_value && i + 1 < args->argc) {
					snprintf(value_buf, max_value_len, "%s",
						 args->argv[i + 1]);
				} else if (takes_value && max_value_len > 0) {
					strcpy(value_buf, "");
				}
				return 1;
			} else {
				itt_index += 1;
			}
		}
	}

	if (default_value != NULL && index == 0) {
		snprintf(value_buf, max_value_len, "%s", default_value);
		return 1;
	}

	return 0;
}
int args_get_argument(Args *args, u64 index, char *value_buf,
		      u64 max_value_len) {
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

			for (u64 k = 0; k < args->subs_count; k++) {
				for (u64 j = 0; j < args->subs[k]->params_count;
				     j++) {
					if (is_short &&
					    !strcmp(name, args->subs[k]
							      ->params[j]
							      .short_name)) {
						if (name, args->subs[k]
							      ->params[j]
							      .takes_value) {
							i += 1;
							found = true;
							break;
						}
					} else if (!strcmp(name, args->subs[k]
								     ->params[j]
								     .name)) {
						if (name, args->subs[k]
							      ->params[j]
							      .takes_value) {
							i += 1;
							found = true;
							break;
						}
					}
				}
			}
		} else {
			if (index_itt == index) {
				snprintf(value_buf, max_value_len, "%s",
					 args->argv[i]);
				return 1;
			}
			index_itt += 1;
		}
	}
	return 0;
}

void args_print_version(Args *args) {
	char *prog = args->prog;
	char *version = args->version;
	fprintf(stderr, "%s%s%s %s%s%s\n", BRIGHT_RED, prog, RESET, CYAN,
		version, RESET);
	exit(0);
}

void args_usage(Args *args, char *sub_command) {
	bool found = false;
	u64 subs_count = args->subs_count;

	if (sub_command) {
		for (u64 i = 1; i < subs_count; i++) {
			char *name = args->subs[i]->name;
			if (!strcmp(name, sub_command)) {
				found = true;
			}
		}
	} else // general help displayed
		found = true;

	if (!found) {
		args_exit_error(args, "Unknown SubCommand \"%s\"", sub_command);
	}

	SubCommand **subs = args->subs;
	ArgsParam *params = args->subs[0]->params;
	u64 count = args->subs[0]->params_count;
	u64 max_len = 0;
	for (u64 i = 0; i < count; i++) {
		bool takes_value = args->subs[0]->params[i].takes_value;
		bool multi = args->subs[0]->params[i].multiple;
		char *name = args->subs[0]->params[i].name;
		char *short_name = args->subs[0]->params[i].short_name;
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

	char *prog = args->prog;
	char *author = args->author;
	char *version = args->version;
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
		"    %s-V%s, %s--version%s%sPrints version "
		"information\n",
		CYAN, prog, RESET, YELLOW, version, RESET, GREEN, author, RESET,
		DIMMED, RESET, BRIGHT_RED, prog, RESET, DIMMED, RESET,
		sub_command_str, DIMMED, RESET, DIMMED, RESET, CYAN, RESET,
		YELLOW, RESET, buffer, CYAN, RESET, YELLOW, RESET, buffer2);

	for (u64 i = 0; i < count; i++) {
		bool takes_value = params[i].takes_value;
		if (!takes_value) {
			char *name = params[i].name;
			char *short_name = params[i].short_name;
			char *help = params[i].help;
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
			bool takes_value = params[i].takes_value;
			if (takes_value) {
				char *name = params[i].name;
				char *short_name = params[i].short_name;
				char *help = params[i].help;
				bool multi = params[i].multiple;
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

			char *name = args->subs[i]->name;
			char *help = args->subs[i]->help;

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
			char *name = args->subs[i]->name;
			if (!strcmp(name, sub_command)) {
				param_index = i;
			}
		}

		if (param_index > 0) {
			params = args->subs[param_index]->params;

			count = args->subs[param_index]->params_count;
			fprintf(stderr, "\n%sSUB_FLAGS%s (%s%s%s):\n", DIMMED,
				RESET, BRIGHT_RED, sub_command, RESET);

			for (u64 i = 0; i < count; i++) {
				char *name = params[i].name;
				char *short_name = params[i].short_name;
				bool takes_value = params[i].takes_value;
				char *help = params[i].help;

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
				char *name = params[i].name;
				char *short_name = params[i].short_name;
				bool takes_value = params[i].takes_value;
				char *help = params[i].help;
				bool multiple = params[i].multiple;

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

	exit(0);
}
