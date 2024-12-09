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
#include <base/types.h>
#include <base/util.h>

extern char **environ;
static int checked = false;
static int no_color_value = false;

int _debug_no_color__ = false;

int no_color() {
	if (!_debug_no_color__ && checked) return no_color_value;
	for (int i = 0; environ[i] != 0; i++) {
		char *env_var = environ[i];
		if (_debug_no_color__ || !cstring_compare_n("NO_COLOR=", env_var, 9)) {
			no_color_value = true;
			checked = true;
			return true;
		}
	}
	checked = true;
	return false;
}

const char *get_dimmed() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[2m";
	}
}

const char *get_red() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[31m";
	}
}

const char *get_bright_red() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[91m";
	}
}

const char *get_green() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[32m";
	}
}

const char *get_yellow() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[33m";
	}
}

const char *get_cyan() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[36m";
	}
}

const char *get_magenta() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[35m";
	}
}

const char *get_blue() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[34m";
	}
}

const char *get_reset() {
	if (no_color()) {
		return "";
	} else {
		return "\x1b[0m";
	}
}

#ifdef TEST
void test_reset_colors() {
	_debug_no_color__ = false;
	checked = false;
	no_color_value = false;
}
#endif	// TEST
