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

byte *get_dimmed() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[2m";
	}
}

byte *get_red() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[31m";
	}
}

byte *get_bright_red() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[91m";
	}
}

byte *get_green() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[32m";
	}
}

byte *get_yellow() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[33m";
	}
}

byte *get_cyan() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[36m";
	}
}

byte *get_magenta() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[35m";
	}
}

byte *get_blue() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[34m";
	}
}

byte *get_reset() {
	if (env("NO_COLOR") != NULL) {
		return "";
	} else {
		return "\x1b[0m";
	}
}
