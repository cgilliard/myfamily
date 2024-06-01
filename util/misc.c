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

#include <base/constants.h>
#include <util/misc.h>
#include <log/log.h>

#define LOG_LEVEL Debug

int is_white_space(char ch)
{
    debug("is whitespace '%c'", ch);
    if (ch == '\n' || ch == '\t' || ch == '\r' || ch == '\v' || ch == '\f' || ch == ' ')
        return TRUE;
    else
        return FALSE;
}

int is_ident_start(char ch)
{
    if ((ch <= 'Z' && ch >= 'A') || (ch <= 'z' && ch >= 'a') || ch == '_') {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_ident_secondary(char ch)
{
    if (is_ident_start(ch) || (ch <= '9' && ch >= '0')) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int is_joint_possible(char ch)
{
    if (ch == '.' || ch == '=' || ch == '/' || ch == '+' || ch == '&' || ch == '<' || ch == '-' || ch == '%' || ch == '^' || ch == '*' || ch == '>' || ch == '|' || ch == '!' || ch == ':') {
        return TRUE;
    } else {
        return FALSE;
    }
}
