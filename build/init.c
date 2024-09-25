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
#include <base/version.h>
#include <build/build.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/replace.h>

int proc_build_init(const char *config_dir, const char *proj_name, const char *proj_path,
					char authors[11][1024], int author_count, bool lib, char *description) {
	int author_len = 1;
	for (int i = 0; i < author_count; i++) {
		author_len += strlen(authors[i]) + 10;
	}
	char author_replace[author_len];
	strcpy(author_replace, "");
	for (int i = 0; i < author_count; i++) {
		strcat(author_replace, "\"");
		strcat(author_replace, authors[i]);
		if (i == author_count - 1)
			strcat(author_replace, "\"");
		else
			strcat(author_replace, "\", ");
	}
	Path path;
	path_for(&path, proj_path);
	if (path_exists(&path)) {
		path_canonicalize(&path);
		fprintf(stderr, "Path: '%s' already exists!\n", path_to_string(&path));
		exit(-1);
	}
	if (!path_mkdir(&path, 0700, false)) {
		perror("Error: could not create directory for project");
		exit(-1);
	}

	path_push(&path, "fam.toml");
	Path template;
	path_for(&template, config_dir);
	path_push(&template, "resources");
	if (lib)
		path_push(&template, "fam.lib.toml");
	else
		path_push(&template, "fam.toml");
	path_canonicalize(&template);
	if (copy_file(path_to_string(&path), path_to_string(&template)) != 0) {
		perror("Error: copying file");
		exit(-1);
	}

	const char *patterns_in[] = {"REPLACE_FAM_VERSION", "REPLACE_NAME", "REPLACE_AUTHORS",
								 "REPLACE_DESCRIPTION"};
	const bool is_case_sensitive[] = {true, true, true};
	char desc[1024];
	if (strlen(description) > 0)
		strcpy(desc, description);
	else
		strcpy(desc, proj_name);
	const char *replace[] = {FAM_VERSION, proj_name, author_replace, desc};
	replace_file(&path, &path, patterns_in, is_case_sensitive, replace, 4);

	Path main_c;
	path_for(&main_c, proj_path);
	path_push(&main_c, "main.c");

	Path main_c_src;
	path_for(&main_c_src, config_dir);
	path_push(&main_c_src, "resources");
	path_push(&main_c_src, "main.c");
	if (copy_file(path_to_string(&main_c), path_to_string(&main_c_src)) != 0) {
		perror("Error: copying file");
		exit(-1);
	}

	Path mod_h;
	path_for(&mod_h, proj_path);
	path_push(&mod_h, "mod.h");

	Path mod_h_src;
	path_for(&mod_h_src, config_dir);
	path_push(&mod_h_src, "resources");
	path_push(&mod_h_src, "mod.h");
	if (copy_file(path_to_string(&mod_h), path_to_string(&mod_h_src)) != 0) {
		perror("Error: copying file");
		exit(-1);
	}

	Path proj_dir;
	path_for(&proj_dir, proj_path);
	path_canonicalize(&proj_dir);
	printf("Project successfully created in '%s'.\n", path_to_string(&proj_dir));

	return 0;
}
