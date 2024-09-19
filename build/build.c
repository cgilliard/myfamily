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
#include <stdlib.h>
#include <string.h>
#include <toml/toml.h>
#include <util/proc_executor.h>

void build_obj(char *include_param, char *obj_path, char *file_name)
{
	char *args[] = { "gcc", include_param, "-I.", "-c", "-o", obj_path, file_name, NULL };
	if (execute_process(args)) {
		exit_error("execution of process '%s' failed", args[0]);
	}
}

void build_module_list(Vec *module_info, const Path *path, const Path *base_path)
{
	Path next_path;
	path_copy(&next_path, path);
	Vec module_info_reverse;
	vec_init(&module_info_reverse, 10, sizeof(ModuleInfo));
	printf("build_path module list for %s\n", path_to_string(path));
	for (int i = 0; i < 10; i++) {

		if (!strcmp(path_to_string(&next_path), path_to_string(base_path))) {

			break;
		}
		if (strlen(path_to_string(&next_path)) < strlen(path_to_string(base_path))) {
			exit_error("could not find the base path");
		}
		char *modname = path_file_name(&next_path);
		path_pop(&next_path);
		ModuleInfo mi;
		strcpy(mi.name, modname);
		vec_push(&module_info_reverse, &mi);
	}
	while (vec_size(&module_info_reverse) > 0) {
		ModuleInfo *next = vec_pop(&module_info_reverse);
		if (vec_size(&module_info_reverse) > 0) {
			vec_push(module_info, next);
			char *modname = next->name;
		}
	}
}

void build_libs(const char *base_dir, const char *config_dir)
{
	Path include_dir;
	path_for(&include_dir, config_dir);
	path_push(&include_dir, "resources");
	path_push(&include_dir, "include");
	path_canonicalize(&include_dir);
	Path build_dir;
	path_for(&build_dir, base_dir);
	path_push(&build_dir, "target");
	path_push(&build_dir, "build");
	path_canonicalize(&build_dir);
	Path include_build_dir;
	path_for(&include_build_dir, base_dir);
	path_push(&include_build_dir, "target");
	path_push(&include_build_dir, "include");
	path_canonicalize(&include_build_dir);
	Path lib_h;
	path_for(&lib_h, base_dir);
	path_push(&lib_h, "lib.h");
	Path base_path;
	path_for(&base_path, base_dir);
	path_canonicalize(&base_path);
	printf("build libs: %s\n", path_to_string(&base_path));
	if (path_exists(&lib_h)) {
		Vec headers;
		vec_init(&headers, 10, sizeof(HeaderInfo));
		Vec types;
		vec_init(&types, 10, sizeof(TypeInfo));
		path_canonicalize(&lib_h);

		Vec module_info;
		vec_init(&module_info, 10, sizeof(ModuleInfo));
		parse_header(base_dir, &lib_h, &headers, &types, &module_info);

		while (vec_size(&headers)) {
			HeaderInfo *next = vec_pop(&headers);
			Path next_path;
			path_for(&next_path, next->path);
			if (path_exists(&next_path) && path_is_dir(&next_path)) {
				path_push(&next_path, "mod.h");
				vec_clear(&module_info);
				build_module_list(&module_info, &next_path, &base_path);
				if (path_exists(&next_path)) {
					parse_header(base_dir, &next_path, &headers, &types, &module_info);
				}
			} else {
				char file_buf[PATH_MAX];
				snprintf(file_buf, PATH_MAX - 1, "%s%s", path_file_name(&next_path), ".h");
				path_pop(&next_path);

				path_push(&next_path, file_buf);
				if (path_exists(&next_path) && !path_is_dir(&next_path)) {
					vec_clear(&module_info);
					build_module_list(&module_info, &next_path, &base_path);
					// parse header
					parse_header(base_dir, &next_path, &headers, &types, &module_info);
				} else {
					exit_error("Module not found. Expected at '%s'.", path_to_string(&next_path));
				}
			}
		}

		if (chdir(path_to_string(&build_dir))) {
			fprintf(stderr, "Could not change directory to the build directory");
		}

		while (vec_size(&types)) {
			TypeInfo *next = vec_pop(&types);
			Path next_path;
			path_for(&next_path, next->path);
			if (path_exists(&next_path) && !path_is_dir(&next_path)) {
				remove_directory(path_to_string(&build_dir), true);
				Path file_path;
				path_copy(&file_path, &build_dir);
				path_push(&file_path, path_file_name(&next_path));
				copy_file(path_to_string(&file_path), path_to_string(&next_path));

				char modules[PATH_MAX];
				char *file_name = path_file_name(&next_path);
				strcpy(modules, "");

				Vec module_info;
				vec_init(&module_info, 10, sizeof(ModuleInfo));
				build_module_list(&module_info, &next_path, &base_path);
				if (strlen(next->module_file_name) > 0) {
					ModuleInfo mi;
					strcpy(mi.name, next->module_file_name);
					vec_push(&module_info, &mi);
				}

				for (u64 i = 0; i < vec_size(&module_info); i++) {
					ModuleInfo *next = vec_element_at(&module_info, i);
					strcat(modules, next->name);
					strcat(modules, "_");
				}

				strcat(modules, file_name);

				int modules_len = strlen(modules);
				if (modules_len < 3) {
					exit_error("unexpectedly short name for object file: %s\n", modules);
				}
				if (modules[modules_len - 1] != 'c' || modules[modules_len - 2] != '.') {
					exit_error("expected file to end in .c");
				}
				modules[modules_len - 1] = 'o';
				char obj_path[PATH_MAX];
				strcpy(obj_path, "../objs/");
				strcat(obj_path, modules);

				char *include_full_path = path_to_string(&include_dir);
				char include_param[strlen(include_full_path) + 10];
				strcpy(include_param, "-I");
				strcat(include_param, include_full_path);

				char module_str[PATH_MAX + 1024];
				char module_path[PATH_MAX + 1024];
				strcpy(module_str, "");
				strcpy(module_path, "");
				int vec_sz = vec_size(&module_info);
				for (int i = 0; i < vec_sz; i++) {
					strcat(module_str, vec_element_at(&module_info, i));
					strcat(module_path, vec_element_at(&module_info, i));
					if (i != vec_sz - 1) {
						strcat(module_str, "::");
						strcat(module_path, "_");
					}
				}
				strcat(module_path, ".h");

				Path build_specific_dst;
				Path build_specific_src;
				path_for(&build_specific_dst, path_to_string(&build_dir));
				path_push(&build_specific_dst, "build_specific.h");
				path_for(&build_specific_src, path_to_string(&include_build_dir));
				path_push(&build_specific_src, module_path);
				printf("header name %s\n", next->module_file_name);
				printf("copy from %s -> %s\n", path_to_string(&build_specific_dst),
					path_to_string(&build_specific_src));
				copy_file(path_to_string(&build_specific_dst), path_to_string(&build_specific_src));

				printf("Parsing type: %s[%s].\n", module_str, file_name);
				build_obj(include_param, obj_path, file_name);

			} else {
				exit_error(
					"Implementation not found. Expected at '%s'.", path_to_string(&next_path));
			}
		}
	}
}

int proc_build(const char *base_dir, const char *config_dir)
{
	Path include_dir;
	path_for(&include_dir, config_dir);
	path_push(&include_dir, "resources");
	path_push(&include_dir, "include");
	path_canonicalize(&include_dir);

	Path base;
	path_for(&base, base_dir);
	if (path_canonicalize(&base)) {
		exit_error("Directory ('%s') not found.", base_dir);
	}
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

	myfclose(toml_fp);

	toml_raw_t raw_fam_version = toml_raw_in(table, "fam_version");
	char *fam_version;
	if (raw_fam_version) {
		toml_rtos(raw_fam_version, &fam_version);
		fprintf(stderr, "fam.toml generated by fam version: %s\n", fam_version);
		free((void *)fam_version); // Clean up after toml_rtos
	}

	toml_table_t *package_table = toml_table_in(table, "package");
	if (package_table) {
		toml_raw_t raw_name = toml_raw_in(package_table, "name");
		char *name;
		if (raw_name) {
			toml_rtos(raw_name, &name);
			fprintf(stderr, "Building Project: '%s'\n", name);

			Path target;
			path_for(&target, base_dir);
			path_push(&target, "target");
			path_canonicalize(&target);
			if (!path_exists(&target)) {
				printf("mkdir\n");
				if (!path_mkdir(&target, 0700, false))
					exit_error("Could not create directory '%s'.", path_to_string(&target));
				printf("ok\n");
				path_push(&target, "objs");
				if (!path_mkdir(&target, 0700, false))
					exit_error("Could not create directory '%s'.", path_to_string(&target));
				path_pop(&target);
				path_push(&target, "build");
				if (!path_mkdir(&target, 0700, false))
					exit_error("Could not create directory '%s'.", path_to_string(&target));
				path_pop(&target);
				path_push(&target, "include");
				printf("x\n");
				if (!path_mkdir(&target, 0700, false))
					exit_error("Could not create directory '%s'.", path_to_string(&target));
				printf("y\n");
			}

			build_libs(base_dir, config_dir);

			// change to the base directory
			if (chdir(path_to_string(&base))) {
				fprintf(stderr, "Could not change directory to the project directory");
			}

			char *include_full_path = path_to_string(&include_dir);
			char include_param[strlen(include_full_path) + 10];
			strcpy(include_param, "-I");
			strcat(include_param, include_full_path);
			build_obj(include_param, "target/objs/main.o", "main.c");

			glob_t glob_result;
			glob("target/objs/*.o", 0, NULL, &glob_result);

			// Construct the argument array for exec
			char *link[5 + glob_result.gl_pathc + 1];
			Path output_file;
			path_for(&output_file, "target");
			path_push(&output_file, name);
			link[0] = "gcc";
			link[1] = "-o";
			link[2] = path_to_string(&output_file);

			// Add all the .o files from glob to the argument list
			for (size_t i = 0; i < glob_result.gl_pathc; i++) {
				link[3 + i] = glob_result.gl_pathv[i];
			}
			link[3 + glob_result.gl_pathc] = NULL; // Terminate the argument list

			if (execute_process(link)) {
				exit_error("execution of process '%s' failed", link[0]);
			}

			free((void *)name); // Cleanup after toml_rtos
			// Free the TOML table when done
			toml_free(table);
		} else {
			exit_error("project name not found!");
		}
	} else {
		exit_error("[package] directive not found!");
	}

	return 0;
}
