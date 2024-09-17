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

int real_main(int argc, char **argv);

#define WRITE_RESOURCE_TO_DISK(dir, filename, data, size)                                          \
	do {                                                                                           \
		char path[1024];                                                                           \
		snprintf(path, sizeof(path), "%s/%s", dir, filename);                                      \
		printf("path=%s\n", path);                                                                 \
		FILE *f = fopen(path, "wb");                                                               \
		if (f) {                                                                                   \
			fwrite(data, 1, size, f);                                                              \
			fclose(f);                                                                             \
		} else {                                                                                   \
			perror("Could not open file for writing");                                             \
		}                                                                                          \
	} while (0)

#define BUILD_RESOURCE_DIR_(dir, namespace)                                                        \
	for (int i = 0; i < namespace##xxdir_file_count; i++)                                          \
	WRITE_RESOURCE_TO_DISK(dir, namespace##xxdir_file_names[i], namespace##xxdir_files[i],         \
		namespace##xxdir_file_sizes[i])
#define BUILD_RESOURCE_DIR_NAMESPACE(dir, namespace)                                               \
	printf("Building Resource Directory: '%s' '%s'\n", dir, #namespace);                           \
	BUILD_RESOURCE_DIR_(dir, namespace##_)
#define BUILD_RESOURCE_DIR(dir, ...)                                                               \
	MULTI_SWITCH2(BUILD_RESOURCE_DIR_NAMESPACE, BUILD_RESOURCE_DIR_, dir, __VA_ARGS__)

#endif // _MAIN_MAIN__
