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

#ifndef _CORE_DIR__
#define _CORE_DIR__

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

const char *get_home_directory() {
	const char *home = NULL;

#ifdef _WIN32
	home = getenv("USERPROFILE");
	if (home == NULL) {
		const char *homeDrive = getenv("HOMEDRIVE");
		const char *homePath = getenv("HOMEPATH");
		if (homeDrive != NULL && homePath != NULL) {
			static char homeBuffer[MAX_PATH];
			snprintf(homeBuffer, sizeof(homeBuffer), "%s%s", homeDrive, homePath);
			home = homeBuffer;
		}
	}
#else
	home = getenv("HOME");
#endif

	return home;
}

#endif // _CORE_DIR__
