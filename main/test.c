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

#include <base/test.h>
#include <main/main.h>

MySuite(main);

MyTest(main, test_main) {
	Path fam_test;
	path_copy(&fam_test, test_dir);
	path_push(&fam_test, "fam");
	setup_config_dir(path_to_string(&fam_test));
	Path build_id_file;
	path_copy(&build_id_file, test_dir);
	path_push(&build_id_file, "fam");
	path_push(&build_id_file, "build_id");
	cr_assert(path_exists(&build_id_file));
	cr_assert(check_build_id(path_to_string(&fam_test)));

	// setup again should be ok since we have the same build id
	setup_config_dir(path_to_string(&fam_test));
	cr_assert(path_exists(&build_id_file));

	// modify the build id
	MYFILE *fp = myfopen(&build_id_file, "a");
	myfprintf(fp, "add data");
	myfclose(fp);
	u64 fsize = path_file_size(&build_id_file);

	// now setup again
	setup_config_dir(path_to_string(&fam_test));

	cr_assert(fsize != path_file_size(&build_id_file));

	path_push(&fam_test, "1");
	path_push(&fam_test, "1");
	path_push(&fam_test, "1");

	// create should fail because we cannot create this directory with standard mkdir
	__is_debug_misc_no_exit = true;
	setup_config_dir(path_to_string(&fam_test));
	__is_debug_misc_no_exit = false;

	// should not exist
	cr_assert(!path_exists(&fam_test));

	Path fam_test2;
	path_copy(&fam_test2, test_dir);
	path_push(&fam_test2, "fam2");

	__is_debug_misc_no_exit = true;
	__is_debug_real_main_res_mkdir = true;
	setup_config_dir(path_to_string(&fam_test2));
	__is_debug_real_main_res_mkdir = false;
	__is_debug_misc_no_exit = false;

	// this one should be created
	cr_assert(path_exists(&fam_test2));
	path_push(&fam_test2, "resources");
	// but resources does not exist
	cr_assert(!path_exists(&fam_test2));
}

MyTest(main, test_real_main) {
	char *rmain[2] = {"fam", NULL};
	real_main(1, rmain);
	// Path rmtestdir;
	// path_for(&rmtestdir
	// remove_directory(".fam", false);
}
