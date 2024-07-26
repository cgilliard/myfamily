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

#include <dirent.h>
#include <errno.h>
#include <log/log.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define LOG_LEVEL DEBUG

Result real_main(int argc, char **argv) {
	Log log = LOG(LogFilePath("/tmp/abc.log"), MaxAgeMillis(10000),
		      AutoRotate(false));

	for (int i = 0; i < 200; i++) {
		idebug(&log, "this is a test");
		sleep(1);
	}

	return Ok(_());
}
