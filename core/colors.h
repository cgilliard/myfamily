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

#ifndef _CORE_COLORS__
#define _CORE_COLORS__

#define _GNU_SOURCE
#include <stdlib.h>

#ifdef __linux__
#define getenv(x) secure_getenv(x)
#endif // __linux__

static char *get_dimmed() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[2m";
	}
}
#define DIMMED get_dimmed()

static char *get_red() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[31m";
	}
}
#define RED get_red()

static char *get_bright_red() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[91m";
	}
}
#define BRIGHT_RED get_bright_red()

static char *get_green() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[32m";
	}
}
#define GREEN get_green()

static char *get_yellow() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[33m";
	}
}
#define YELLOW get_yellow()

static char *get_cyan() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[36m";
	}
}
#define CYAN get_cyan()

static char *get_magenta() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[35m";
	}
}
#define MAGENTA get_magenta()

static char *get_blue() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[34m";
	}
}
#define BLUE get_blue()

static char *get_reset() {
	if (getenv("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[0m";
	}
}
#define RESET get_reset()

#endif // _CORE_COLORS__
