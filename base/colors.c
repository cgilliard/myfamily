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
#include <base/util.h>
extern char **environ;
static bool checked = false;
static bool no_color_value = false;

bool no_color() {
	if (checked) return no_color_value;
	for (int i = 0; environ[i] != NULL; i++) {
		char *env_var = environ[i];
		if (!cstring_compare_n("NO_COLOR=", env_var, 9)) {
			no_color_value = true;
			checked = true;
			return true;
		}
	}
	checked = true;
	return false;
}

byte *get_dimmed() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[2m";
	}
}

byte *get_red() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[31m";
	}
}

byte *get_bright_red() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[91m";
	}
}

byte *get_green() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[32m";
	}
}

byte *get_yellow() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[33m";
	}
}

byte *get_cyan() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[36m";
	}
}

byte *get_magenta() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[35m";
	}
}

byte *get_blue() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[34m";
	}
}

byte *get_reset() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[0m";
	}
}
