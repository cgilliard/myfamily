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
#include <base/unit.h>

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

void SubCommand_cleanup(SubCommand *ptr) {
	char *name = *SubCommand_get_name(ptr);
	u64 count = *SubCommand_get_count(ptr);
	if (name) {
		tlfree(name);
		SubCommand_set_name(ptr, NULL);
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
				 u64 max_args) {
	SubCommandPtr ret =
	    BUILD(SubCommand, name, params, state, count, min_args, max_args);

	return ret;
}

bool SubCommand_copy(SubCommand *dst, SubCommand *src) {
	bool ret = true;
	char *src_name = *SubCommand_get_name(src);
	char *name = tlmalloc(sizeof(char) * strlen(src_name));
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
					     min_args, max_args);
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
size_t SubCommand_size(SubCommand *ptr) { return sizeof(SubCommand); }
Result SubCommand_build(char *name, u32 min_args, u32 max_args) {

	SubCommandPtr src =
	    BUILD(SubCommand, name, NULL, NULL, 0, min_args, max_args);
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

void Args_cleanup(Args *ptr) {
	u64 count = *Args_get_subs_count(ptr);
	SubCommandPtr *subs = *Args_get_subs(ptr);
	for (u64 i = 0; i < count; i++) {
		cleanup(&subs[i]);
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
}
Result Args_add_param(Args *ptr, char *name, char *help, char *short_name,
		      bool takes_value, bool multiple) {
	SubCommandPtr *subs = *Args_get_subs(ptr);
	Result res =
	    ArgsParam_build(name, help, short_name, takes_value, multiple);
	ArgsParam param = *(ArgsParam *)Try(res);
	SubCommand_add_param(&subs[0], &param);
	return Ok(UNIT);
}
Result Args_add_sub(Args *ptr, SubCommand *sub) {
	u64 count = *Args_get_subs_count(ptr);
	SubCommandPtr *subs = *Args_get_subs(ptr);

	if (count == 0) {
		void *tmp = tlmalloc(sizeof(SubCommand));
		if (tmp == NULL) {
			Error err =
			    ERROR(ALLOC_ERROR, "Could not increase the size of "
					       "the sub command list");
			return Err(err);
		}
		subs = tmp;
		Args_set_subs(ptr, tmp);
	} else {
		void *tmp = tlrealloc(subs, sizeof(SubCommand) * (1 + count));
		if (tmp == NULL) {
			Error err =
			    ERROR(ALLOC_ERROR, "Could not increase the size of "
					       "the sub command list");
			return Err(err);
		}
		subs = tmp;
		Args_set_subs(ptr, tmp);
	}
	copy(&subs[count], sub);
	Args_set_subs_count(ptr, count + 1);

	return Ok(UNIT);
}

bool Args_copy(Args *dst, Args *src) {
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

	if (ret) {
		Args_set_prog(dst, prog);
		Args_set_version(dst, version);
		Args_set_author(dst, author);
	}

	u64 subs_count = *Args_get_subs_count(src);
	SubCommandPtr *subs_src = *Args_get_subs(src);
	SubCommandPtr *subs;
	if (subs_count > 0) {
		subs = tlmalloc(sizeof(SubCommand) * subs_count);
		if (subs == NULL)
			ret = false;
		else
			for (u64 i = 0; i < subs_count; i++) {
				if (!copy(&subs[i], &subs_src[i]))
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

size_t Args_size(Args *obj) { return sizeof(Args); }

Result Args_build(char *prog, char *version, char *author) {
	ArgsPtr src = BUILD(Args, prog, version, author, NULL, 0, NULL, 0, 0);
	Args ret;
	if (!copy(&ret, &src)) {
		Error err = ERROR(COPY_ERROR, "could not copy sub args");
		return Err(err);
	}

	Result res = SubCommand_build("", 0, 0);
	SubCommand sub = *(SubCommand *)Try(res);
	Result res2 = Args_add_sub(&ret, &sub);
	Try(res2);

	return Ok(ret);
}

Result Args_init(Args *ptr, int argc, char **argv, u64 flags) { todo() }

void Args_usage(Args *ptr) {}

Option Args_argument(Args *ptr, u64 index) { return None; }

Result Args_value(Args *ptr, char *buffer, size_t len, char *param,
		  char *value) {
	todo()
}
