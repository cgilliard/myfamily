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

#include <base/colors.h>
#include <base/os.h>

char *get_dimmed() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[2m";
	}
}

char *get_red() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[31m";
	}
}

char *get_bright_red() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[91m";
	}
}

char *get_green() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[32m";
	}
}

char *get_yellow() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[33m";
	}
}

char *get_cyan() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[36m";
	}
}

char *get_magenta() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[35m";
	}
}

char *get_blue() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[34m";
	}
}

char *get_reset() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[0m";
	}
}
