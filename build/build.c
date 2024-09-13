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
#include <stdio.h>
#include <stdlib.h>

int proc_build_init(const char *proj_name, const char *proj_path)
{
	Path path;
	path_for(&path, proj_path);
	if (path_exists(&path)) {
		fprintf(stderr, "Path: '%s' already exists!\n", proj_path);
		exit(-1);
	}
	if (path_mkdir(&path, 0700)) {
		perror("Error: could not create directory for project");
		exit(-1);
	}

	path_push(&path, "fam.toml");
	Path template;
	path_for(&template, "~/.fam/resources/fam.toml");
	path_canonicalize(&template);
	if (copy_file(path_to_string(&path), path_to_string(&template)) != 0) {
		perror("Error: copying file");
		exit(-1);
	}

	Path proj_dir;
	path_for(&proj_dir, proj_path);
	path_canonicalize(&proj_dir);
	printf("Project successfully created in '%s'.\n", path_to_string(&proj_dir));

	return 0;
}
