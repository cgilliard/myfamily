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

void build_libs(const char *base_dir, const char *config_dir)
{
	printf("build libs: %s\n", base_dir);
	Path build_dir;
	path_for(&build_dir, base_dir);
	path_push(&build_dir, "target");
	path_push(&build_dir, "build");
	path_canonicalize(&build_dir);
	Path lib_h;
	path_for(&lib_h, base_dir);
	path_push(&lib_h, "lib.h");
	Path base_path;
	path_for(&base_path, base_dir);
	path_canonicalize(&base_path);
	if (path_exists(&lib_h)) {
		printf("found lib.h!\n");
		char **header_list = mymalloc(sizeof(char *) * 10);
		int header_to_parse = 0;
		Vec headers;
		vec_init(&headers, 10, sizeof(HeaderInfo));
		Vec types;
		vec_init(&types, 10, sizeof(TypeInfo));
		printf("headers size=%" PRIu64 "\n", vec_size(&headers));
		path_canonicalize(&lib_h);
		parse_header(&lib_h, &headers, &types);
		printf("headers size=%" PRIu64 "\n", vec_size(&headers));

		while (vec_size(&headers)) {
			HeaderInfo *next = vec_pop(&headers);
			printf("next header = %s\n", next->path);
			Path next_path;
			path_for(&next_path, next->path);
			if (path_exists(&next_path) && path_is_dir(&next_path)) {
				printf("path directory exists!\n");
				path_push(&next_path, "mod.h");
				if (path_exists(&next_path)) {
					printf("mod.h exists\n");
					parse_header(&next_path, &headers, &types);
				}
			} else {
				printf("path does not exist. check file.\n");
				char file_buf[PATH_MAX];
				printf("fname='%s'\n", path_file_name(&next_path));
				snprintf(file_buf, PATH_MAX - 1, "%s%s", path_file_name(&next_path), ".h");
				path_pop(&next_path);
				printf("checking %s\n", path_to_string(&next_path));

				path_push(&next_path, file_buf);
				printf("checking %s\n", path_to_string(&next_path));
				if (path_exists(&next_path) && !path_is_dir(&next_path)) {
					printf("file %s exists!\n", path_to_string(&next_path));
					// parse header
					parse_header(&next_path, &headers, &types);
				}
			}
		}

		printf("types count = %" PRIu64 "\n", vec_size(&types));
		if (chdir(path_to_string(&build_dir))) {
			fprintf(stderr, "Could not change directory to the build directory");
		}

		while (vec_size(&types)) {
			TypeInfo *next = vec_pop(&types);
			printf("file to compile: %s\n", next->path);
			Path next_path;
			path_for(&next_path, next->path);
			if (path_exists(&next_path) && !path_is_dir(&next_path)) {
				remove_directory(path_to_string(&build_dir), true);
				path_push(&build_dir, path_file_name(&next_path));
				copy_file(path_to_string(&build_dir), path_to_string(&next_path));
				path_pop(&build_dir);

				char modules[PATH_MAX];
				char *file_name = path_file_name(&next_path);
				strcpy(modules, "");
				printf("file=%s,base=%s\n", path_to_string(&next_path), path_to_string(&base_path));
				for (int i = 0; i < 3; i++) {
					path_pop(&next_path);
					char *modname = path_file_name(&next_path);
					printf("file_pop[%i]=%s\n", i, path_to_string(&next_path));
					if (!strcmp(path_to_string(&next_path), path_to_string(&base_path))) {
						printf("match\n");
						break;
					}
					if (strlen(path_to_string(&next_path)) < strlen(path_to_string(&base_path))) {
						exit_error("could not find the base path");
					}

					strcat(modules, modname);
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
				printf("modules: %s\n", modules);
				char obj_path[PATH_MAX];
				strcpy(obj_path, "../objs/");
				strcat(obj_path, modules);

				char *args[] = { "cc", "-c", "-o", obj_path, "Server.c", NULL };
				if (execute_process(args)) {
					exit_error("execution of process '%s' failed", args[0]);
				}

			} else {
				exit_error(
					"Implementation not found. Expected at '%s'.\n", path_to_string(&next_path));
			}
		}
	}
}

int proc_build(const char *base_dir, const char *config_dir)
{
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
			if (!path_exists(&target)) {
				if (path_mkdir(&target, 0700))
					exit_error("Could not create directory '%s'.", path_to_string(&target));
				path_push(&target, "objs");
				if (path_mkdir(&target, 0700))
					exit_error("Could not create directory '%s'.", path_to_string(&target));
				path_pop(&target);
				path_push(&target, "build");
				if (path_mkdir(&target, 0700))
					exit_error("Could not create directory '%s'.", path_to_string(&target));
			}

			build_libs(base_dir, config_dir);

			// change to the base directory
			printf("chdir %s\n", base_dir);
			if (chdir(path_to_string(&base))) {
				fprintf(stderr, "Could not change directory to the project directory");
			}

			char *args[] = { "cc", "-c", "-o", "target/objs/main.o", "main.c", NULL };
			if (execute_process(args)) {
				exit_error("execution of process '%s' failed", args[0]);
			}

			glob_t glob_result;
			glob("target/objs/*.o", 0, NULL, &glob_result);

			// Construct the argument array for exec
			char *link[5 + glob_result.gl_pathc + 1];
			Path output_file;
			path_for(&output_file, "target");
			path_push(&output_file, name);
			link[0] = "cc";
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
