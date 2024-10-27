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

#ifndef _BASE_COLORS__
#define _BASE_COLORS__

#include <base/types.h>

byte *get_dimmed();
#define DIMMED get_dimmed()

byte *get_red();
#define RED get_red()

byte *get_bright_red();
#define BRIGHT_RED get_bright_red()

byte *get_green();
#define GREEN get_green()

byte *get_yellow();
#define YELLOW get_yellow()

byte *get_cyan();
#define CYAN get_cyan()

byte *get_magenta();
#define MAGENTA get_magenta()

byte *get_blue();
#define BLUE get_blue()

byte *get_reset();
#define RESET get_reset()

#endif // _BASE_COLORS__
