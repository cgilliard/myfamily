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

#ifndef _BUILD_BUILD__
#define _BUILD_BUILD__

#include <base/types.h>

#define MAX_NAME_LEN 128
#define MAX_MODULE_DEPTH 10

typedef struct SubModuleInfo {
	char name[MAX_NAME_LEN];
} SubModuleInfo;

typedef struct ModuleInfo {
	SubModuleInfo module_list[MAX_MODULE_DEPTH];
	u16 sub_module_count;
} ModuleInfo;

typedef struct TypeInfo {
	char type_name[MAX_NAME_LEN];
	ModuleInfo mi;
	u64 gen_file_counter;
} TypeInfo;

int proc_build_init(const char *config_dir, const char *proj_name, const char *proj_path,
	char authors[11][1024], int author_count, bool lib, char *description);

int proc_build(const char *base_dir, const char *config_dir);
u64 type_info_to_string(const TypeInfo *type_info, char *buf, u64 limit);
u64 type_info_to_path(const TypeInfo *type_info, char *buf, u64 limit);
u64 type_info_module_file(const char *base_dir, const TypeInfo *type_info, char *buf, u64 limit);
u64 module_to_string(const ModuleInfo *module_info, char *buf, u64 limit);
u64 module_to_type_string(const ModuleInfo *type_info, char *buf, u64 limit);

#endif // _BUILD_BUILD__
