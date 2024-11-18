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

#include <base/test.h>

Suite(base);

Test(test_colors) {
	byte buf[1024];
	int len;

	len = sprint(buf, 1024, "Colors: %sgreen%s %sdimmed%s %sred%s %smagenta%s",
				 GREEN, RESET, DIMMED, RESET, RED, RESET, MAGENTA, RESET);
	fam_assert_eq(len, 67);
	_debug_no_color__ = true;
	len = sprint(buf, 1024,
				 "Colors: %sgreen%s %sdimmed%s %sred%s %smagenta%s %sbred%s "
				 "%syellow%s %scyan%s %sblue%s",
				 GREEN, RESET, DIMMED, RESET, RED, RESET, MAGENTA, RESET,
				 BRIGHT_RED, RESET, YELLOW, RESET, CYAN, RESET, BLUE, RESET);
	fam_assert_eq(len, 54);
	test_reset_colors();
}
