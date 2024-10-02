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

#ifndef _MAIN_MAIN__
#define _MAIN_MAIN__

#include <base/macro_utils.h>
#include <base/misc.h>
#include <base/resources.h>
#include <base/sha3.h>
#include <base/types.h>

#include <main/resources.h>

#define FAM_VERSION "0.0.1-alpha.1"
#define DATE_TIME __DATE__ " " __TIME__
static char _build_id__[65];
static char *get_build_id() {
	SHA3_HASH(DATE_TIME, _build_id__);
	_build_id__[64] = 0;
	return _build_id__;
}

int real_main(int argc, char **argv);
void setup_config_dir(const char *config_dir);
bool check_build_id(const char *config_dir);
void write_to_disk(const char *dir, const char *file_name, const unsigned char *data, u64 size);

#define WRITE_RESOURCE_TO_DISK(dir, filename, data, size) write_to_disk(dir, filename, data, size)

#define WRITE_BUILD_ID(dir)                                                                        \
	({                                                                                             \
		const char *_bid__ = get_build_id();                                                       \
		WRITE_RESOURCE_TO_DISK(dir, "build_id", (const unsigned char *)_bid__, strlen(_bid__));    \
	})

#define BUILD_RESOURCE_DIR(dir, namespace)                                                         \
	for (int i = 0; i < namespace##_##xxdir_file_count; i++)                                       \
	WRITE_RESOURCE_TO_DISK(dir, namespace##_##xxdir_file_names[i], namespace##_##xxdir_files[i],   \
						   namespace##_##xxdir_file_sizes[i])

#define MAIN                                                                                       \
	int main(int argc, char **argv) {                                                              \
		return real_main(argc, argv);                                                              \
	}

#ifdef TEST
extern bool __is_debug_real_main_res_mkdir;
#endif // TEST

#ifdef TEST
#define DEFAULT_CONFIG_DIR "./.fam"
#else
#define DEFAULT_CONFIG_DIR "~/.fam"
#endif // TEST

#endif // _MAIN_MAIN__
