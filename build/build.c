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

#include <base/misc.h>
#include <base/path.h>
#include <base/resources.h>
#include <build/build.h>
#include <build/parser.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <toml/toml.h>
#include <util/proc_executor.h>

typedef struct FamTomlInfo {
	char name[MAX_NAME_LEN];
	char version[MAX_NAME_LEN];
} FamTomlInfo;

FamTomlInfo extract_toml_info(const char *base_dir)
{
	Path toml;
	path_for(&toml, base_dir);
	path_push(&toml, "fam.toml");

	if (path_canonicalize(&toml)) {
		exit_error("fam.toml file ('%s') not found.", path_to_string(&toml));
	}

	FILE *toml_fp = myfopen(path_to_string(&toml), "r");
	char errbuf[1024];
	toml_table_t *table = toml_parse_file(toml_fp, errbuf, sizeof(errbuf));

	if (table == NULL) {
		exit_error("Could not parse toml file due to '%s'", errbuf);
	}

	FamTomlInfo ret;

	myfclose(toml_fp);

	toml_raw_t raw_fam_version = toml_raw_in(table, "fam_version");
	char *fam_version;
	if (raw_fam_version) {
		toml_rtos(raw_fam_version, &fam_version);
		strcpy(ret.version, fam_version);
		free((void *)fam_version); // Clean up after toml_rtos
	} else {
		exit_error("fam_version not specified in '%s'", path_to_string(&toml));
	}

	toml_table_t *package_table = toml_table_in(table, "package");
	if (package_table) {
		toml_raw_t raw_name = toml_raw_in(package_table, "name");
		char *name;
		if (raw_name) {
			toml_rtos(raw_name, &name);
			strcpy(ret.name, name);
		} else {
			exit_error("name not specified in fam.toml!");
		}
	} else {
		exit_error("package not found in fam.toml");
	}

	return ret;
}

void ensure_target_structure(const char *base_dir)
{
	Path target;
	path_for(&target, base_dir);
	path_push(&target, "target");
	if (!path_exists(&target)) {
		if (!path_mkdir(&target, 0700, false))
			exit_error("Could not create directory '%s'.", path_to_string(&target));
		path_push(&target, "objs");
		if (!path_mkdir(&target, 0700, false))
			exit_error("Could not create directory '%s'.", path_to_string(&target));
		path_pop(&target);
		path_push(&target, "build");
		if (!path_mkdir(&target, 0700, false))
			exit_error("Could not create directory '%s'.", path_to_string(&target));
		path_pop(&target);
		path_push(&target, "include");
		if (!path_mkdir(&target, 0700, false))
			exit_error("Could not create directory '%s'.", path_to_string(&target));
		path_pop(&target);
		path_push(&target, "args");
		if (!path_mkdir(&target, 0700, false))
			exit_error("Could not create directory '%s'.", path_to_string(&target));
	}

	if (path_canonicalize(&target)) {
		exit_error("Could not create path: %s\n", path_to_string(&target));
	}
}

u64 module_to_string(const ModuleInfo *module_info, char *buf, u64 limit)
{
	u64 itt = 0;
	strncpy(buf, "", limit);
	for (u64 i = 0; i < module_info->sub_module_count; i++) {
		if (itt < limit)
			strncat(buf, module_info->module_list[i].name, limit - itt);
		itt += strlen(module_info->module_list[i].name);
		if (i != module_info->sub_module_count - 1) {
			if (itt < limit)
				strncat(buf, "/", limit - itt);
			itt += 1;
		}
	}

	return itt;
}

u64 module_to_type_string(const ModuleInfo *type_info, char *buf, u64 limit)
{
	u64 itt = 0;
	strncpy(buf, "type_____", limit);
	itt += 10;
	for (u64 i = 0; i < type_info->sub_module_count; i++) {
		if (itt < limit)
			strncat(buf, type_info->module_list[i].name, limit - itt);
		itt += strlen(type_info->module_list[i].name);
		if (1 + i < type_info->sub_module_count) {
			if (itt < limit)
				strncat(buf, "_____", limit - itt);
			itt += 5;
		}
	}

	return itt;
}

u64 type_info_to_string(const TypeInfo *type_info, char *buf, u64 limit)
{
	u64 itt = 0;
	strncpy(buf, "type_____", limit);
	itt += 10;
	for (u64 i = 0; i < type_info->mi.sub_module_count; i++) {
		if (itt < limit)
			strncat(buf, type_info->mi.module_list[i].name, limit - itt);
		itt += strlen(type_info->mi.module_list[i].name);
		if (itt < limit)
			strncat(buf, "_____", limit - itt);
		itt += 5;
	}
	if (itt < limit)
		strncat(buf, type_info->type_name, limit - itt);
	itt += strlen(type_info->type_name);

	return itt;
}

u64 type_info_to_path(const TypeInfo *type_info, char *buf, u64 limit)
{
	u64 itt = 0;
	strncpy(buf, "", limit);
	for (u64 i = 0; i < type_info->mi.sub_module_count; i++) {
		if (itt < limit)
			strncat(buf, type_info->mi.module_list[i].name, limit - itt);
		itt += strlen(type_info->mi.module_list[i].name);
		if (itt < limit)
			strncat(buf, "/", limit - itt);
		itt += 1;
	}
	if (itt < limit)
		strncat(buf, type_info->type_name, limit - itt);
	itt += strlen(type_info->type_name);

	return itt;
}

u64 type_info_module_file(const char *base_dir, const TypeInfo *type_info, char *buf, u64 limit)
{
	Path p;
	path_for(&p, base_dir);
	if (type_info->mi.sub_module_count) {
		for (u64 i = 0; i < type_info->mi.sub_module_count - 1; i++) {
			path_push(&p, type_info->mi.module_list[i].name);
		}
	}
	if (type_info->mi.sub_module_count) {
		char header_name[PATH_MAX];
		strcpy(header_name, type_info->mi.module_list[type_info->mi.sub_module_count - 1].name);
		strcat(header_name, ".h");
		path_push(&p, header_name);

		if (path_exists(&p)) {
			strncpy(buf, path_to_string(&p), limit);
			return strlen(path_to_string(&p));
		} else {
			path_pop(&p);
			path_push(&p, type_info->mi.module_list[type_info->mi.sub_module_count - 1].name);
			path_push(&p, "mod.h");
			if (path_exists(&p)) {
				strncpy(buf, path_to_string(&p), limit);
				return strlen(path_to_string(&p));
			} else {
				exit_error("header file did not exist at path '%s'", path_to_string(&p));
			}
		}
	} else {
		path_push(&p, "mod.h");
		strncpy(buf, path_to_string(&p), limit);
		return strlen(path_to_string(&p));
	}

	// should never get here
	return 0;
}

void build_obj(const char *cc, const char *include_dir, const char *obj_dir, const char *src_dir,
	const TypeInfo *type_info, const char *obj_prefix, const char *config_include,
	const char *impl_def, const char *implconfig_def, const char *args_path)
{
	Path obj_path;
	path_for(&obj_path, obj_dir);

	u64 obj_prefix_len = 0;
	if (obj_prefix)
		obj_prefix_len = strlen(obj_prefix);
	char obj_file_name[obj_prefix_len + strlen(type_info->type_name) + 5];
	if (obj_prefix) {
		strcpy(obj_file_name, obj_prefix);
		strcat(obj_file_name, type_info->type_name);
		strcat(obj_file_name, ".o");
	} else {
		strcpy(obj_file_name, type_info->type_name);
		strcat(obj_file_name, ".o");
	}

	path_push(&obj_path, obj_file_name);

	u64 needed = type_info_to_path(type_info, NULL, 0);
	char file_name[needed + 3];
	type_info_to_path(type_info, file_name, needed);
	strcat(file_name, ".c");

	Path src_path;
	path_for(&src_path, src_dir);
	path_push(&src_path, file_name);

	char include_param[strlen(include_dir) + 5];
	strcpy(include_param, "-I");
	strcat(include_param, include_dir);

	char args_path_with_at[strlen(args_path) + 2];
	strcpy(args_path_with_at, "@");
	strcat(args_path_with_at, args_path);

	if (config_include) {
		char config_include_param[strlen(config_include) + 5];
		strcpy(config_include_param, "-I");
		strcat(config_include_param, config_include);
		const char *args[] = { cc, "-Wno-attributes", "-Wno-ignored-attributes", include_param,
			config_include_param, "-c", "-o", path_to_string(&obj_path), path_to_string(&src_path),
			impl_def, implconfig_def, args_path_with_at, NULL };
		if (execute_process(args)) {
			exit_error("execution of process '%s' failed", args[0]);
		}
	} else {
		const char *args[] = { cc, "-Wno-attributes", "-Wno-ignored-attributes", include_param,
			"-c", "-o", path_to_string(&obj_path), path_to_string(&src_path), impl_def,
			implconfig_def, args_path_with_at, NULL };
		if (execute_process(args)) {
			exit_error("execution of process '%s' failed", args[0]);
		}
	}
}

void link_objs(const char *objs_path, const char *base_dir, const char *name)
{
	glob_t glob_result;
	Path objs_dir;
	path_for(&objs_dir, base_dir);
	path_push(&objs_dir, "target/objs/*.o");

	glob(path_to_string(&objs_dir), 0, NULL, &glob_result);

	// Construct the argument array for exec
	char *link[5 + glob_result.gl_pathc + 1];
	Path output_file;
	path_for(&output_file, base_dir);
	path_push(&output_file, "target");
	path_push(&output_file, name);
	link[0] = "cc";
	link[1] = "-o";
	link[2] = path_to_string(&output_file);

	// Add all the .o files from glob to the argument list
	for (size_t i = 0; i < glob_result.gl_pathc; i++) {
		link[3 + i] = glob_result.gl_pathv[i];
	}
	link[3 + glob_result.gl_pathc] = NULL; // Terminate the argument list

	const char **link_const = (const char **)&link;

	if (execute_process(link_const)) {
		exit_error("execution of process '%s' failed", link[0]);
	}
}

void build_internal(const char *base_dir, const char *config_dir)
{
	Path build_path;
	path_for(&build_path, base_dir);
	path_push(&build_path, "target");
	path_push(&build_path, "build");
	remove_directory(path_to_string(&build_path), true);

	Path objs_path;
	path_for(&objs_path, base_dir);
	path_push(&objs_path, "target");
	path_push(&objs_path, "objs");

	Path config_src;
	path_for(&config_src, config_dir);
	path_push(&config_src, "resources");
	path_push(&config_src, "src");
	path_push(&config_src, "*.c");

	Path args_path;
	path_for(&args_path, base_dir);
	path_push(&args_path, "target");
	path_push(&args_path, "args");
	path_push(&args_path, "args.txt");

	Path include_dir;
	path_for(&include_dir, config_dir);
	path_push(&include_dir, "resources");
	path_push(&include_dir, "include");

	glob_t glob_result;
	glob(path_to_string(&config_src), 0, NULL, &glob_result);

	Path config_dir_src_path;
	path_for(&config_dir_src_path, config_dir);
	path_push(&config_dir_src_path, "resources");
	path_push(&config_dir_src_path, "src");

	for (u64 i = 0; i < glob_result.gl_pathc; i++) {
		TypeInfo ti;
		ti.mi.sub_module_count = 0;
		if (strlen(glob_result.gl_pathv[i]) >= MAX_NAME_LEN) {
			exit_error("name too long! [%s]", glob_result.gl_pathv[i]);
		}
		Path glob_path;
		path_for(&glob_path, glob_result.gl_pathv[i]);
		char path_stem[strlen(path_file_name(&glob_path)) + 1];
		path_file_stem(&glob_path, path_stem, strlen(path_file_name(&glob_path)));
		strcpy(ti.type_name, path_stem);
		build_obj("cc", path_to_string(&include_dir), path_to_string(&objs_path),
			path_to_string(&config_dir_src_path), &ti, "___internal_objs_", NULL,
			"-DIMPL=", "-DIMPLCONFIG=", path_to_string(&args_path));
	}
}

int proc_build(const char *base_dir, const char *config_dir)
{
	fprintf(stderr, "proc build %s %s\n", base_dir, config_dir);
	FamTomlInfo fti = extract_toml_info(base_dir);
	fprintf(stderr, "building project %s. fam.toml generated by %s.\n", fti.name, fti.version);
	ensure_target_structure(base_dir);

	Path args_dir;
	path_for(&args_dir, base_dir);
	path_push(&args_dir, "target");
	path_push(&args_dir, "args");
	// remove_directory(path_to_string(&args_dir), true);

	Path config_include_dir;
	path_for(&config_include_dir, config_dir);
	path_push(&config_include_dir, "resources");
	path_push(&config_include_dir, "include");

	Path include_dir;
	path_for(&include_dir, base_dir);
	path_push(&include_dir, "target");
	path_push(&include_dir, "include");

	Path objs_path;
	path_for(&objs_path, base_dir);
	path_push(&objs_path, "target");
	path_push(&objs_path, "objs");

	Path args_path;
	path_for(&args_path, base_dir);
	path_push(&args_path, "target");
	path_push(&args_path, "args");
	path_push(&args_path, "args.txt");

	build_internal(base_dir, config_dir);
	Path modh;
	path_for(&modh, base_dir);
	path_push(&modh, "mod.h");

	Vec headers;
	vec_init(&headers, 10, sizeof(ModuleInfo));
	Vec types;
	vec_init(&types, 10, sizeof(TypeInfo));
	Vec module_list;
	vec_init(&module_list, 10, sizeof(ModuleInfo));

	path_canonicalize(&modh);

	ModuleInfo self_info;
	self_info.sub_module_count = 0;
	parse_header(config_dir, base_dir, &headers, &types, &self_info);

	while (vec_size(&headers)) {
		ModuleInfo *next = vec_pop(&headers);
		parse_header(config_dir, base_dir, &headers, &types, next);
	}

	while (vec_size(&types)) {
		TypeInfo *next = vec_pop(&types);
		char type_prefix[PATH_MAX];
		type_info_to_string(next, type_prefix, PATH_MAX);
		strcat(type_prefix, "_");
		char gen_header_bs_dir[PATH_MAX];
		snprintf(gen_header_bs_dir, PATH_MAX, "f%" PRIu64, next->gen_file_counter);

		path_push(&include_dir, gen_header_bs_dir);

		char type_str[PATH_MAX * 2];
		type_info_to_string(next, type_str, PATH_MAX * 2);
		printf("type=%s\n", type_str);

		char dimpl[(PATH_MAX * 2) + 30];
		strcpy(dimpl, "-DIMPL=");
		strcat(dimpl, type_str);

		char dimplconfig[(PATH_MAX * 2) + 30];
		strcpy(dimplconfig, "-DIMPLCONFIG=");
		strcat(dimplconfig, type_str);
		strcat(dimplconfig, "Config");

		build_obj("cc", path_to_string(&include_dir), path_to_string(&objs_path), base_dir, next,
			type_prefix, path_to_string(&config_include_dir), dimpl, dimplconfig,
			path_to_string(&args_path));

		path_pop(&include_dir);
	}

	path_push(&include_dir, "f0");
	TypeInfo main_ti;
	strcpy(main_ti.type_name, "main");
	main_ti.mi.sub_module_count = 0;
	main_ti.gen_file_counter = 0;

	// build main
	build_obj("cc", path_to_string(&include_dir), path_to_string(&objs_path), base_dir, &main_ti,
		"type_____", path_to_string(&config_include_dir),
		"-DIMPL=", "-DIMPLCONFIG=", path_to_string(&args_path));

	// link objects
	link_objs(path_to_string(&objs_path), base_dir, fti.name);

	return 0;
}
