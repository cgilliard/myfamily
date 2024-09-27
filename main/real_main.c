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
#include <limits.h>
#include <main/main.h>
#include <stdio.h>
#include <string.h>

bool __is_debug_real_main_res_mkdir = false;

void write_to_disk(const char *dir, const char *file_name, const unsigned char *data, u64 size) {
	char path[PATH_MAX + 1];
	snprintf(path, sizeof(path), "%s/%s", dir, file_name);
	Path ensure_parent;
	path_for(&ensure_parent, path);
	const char *pfile_name = path_file_name(&ensure_parent);
	char pfile_name_copy[PATH_MAX + 1];
	strcpy(pfile_name_copy, pfile_name);
	path_pop(&ensure_parent);
	path_canonicalize(&ensure_parent);
	path_mkdir(&ensure_parent, 0700, true);
	path_push(&ensure_parent, pfile_name_copy);
	MYFILE *f = myfopen(&ensure_parent, "wb");
	if (f) {
		myfwrite(data, 1, size, f);
		myfclose(f);
	} else {
		exit_error("Could not open file for writing");
	}
}

bool check_build_id(const char *config_dir) {
	Path bid_file;
	path_for(&bid_file, config_dir);
	path_push(&bid_file, "build_id");
	path_canonicalize(&bid_file);
	// check build id
	MYFILE *fp = myfopen(&bid_file, "r");
	if (!fp) {
		exit_error("could not open the build file");
	}
	char bid_file_contents[1024];
	size_t rlen = read_all(bid_file_contents, 1, 100, fp);
	bid_file_contents[rlen] = 0;
	myfclose(fp);
	return !strcmp(bid_file_contents, BUILD_ID);
}

void setup_config_dir(const char *config_dir) {
	Path cd;
	path_for(&cd, config_dir);
	path_canonicalize(&cd);
	if (path_exists(&cd)) {
		if (check_build_id(config_dir))
			return;
		remove_directory(&cd, false);
	}
	fprintf(stderr, "Installing config directory at %s. Build id = %s\n", config_dir, BUILD_ID);
	if (!path_mkdir(&cd, 0700, false)) {
		exit_error("Could not create config directory at path [%s].", path_to_string(&cd));
		return;
	}

	Path rd;
	path_copy(&rd, &cd);
	path_push(&rd, "resources");

	if (__is_debug_real_main_res_mkdir || !path_mkdir(&rd, 0700, true)) {
		exit_error("Could not create resources directory at path [%s].", path_to_string(&rd));
		return;
	}

	BUILD_RESOURCE_DIR(path_to_string(&rd), fam);
	WRITE_BUILD_ID(config_dir);
}

int real_main(int argc, char **argv) {
	setup_config_dir(DEFAULT_CONFIG_DIR);
	return 0;
}
