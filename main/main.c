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

#include <stdio.h>
#include <base/backtrace.h>

int real_main(int argc, char** argv)
{
    String s;
    backtrace_to_string(&s);
    printf("backtrace returned: '%s'\n", s.ptr);

    printf("main doesn't currently do anything\n");
    return 0;
}

int sub_main(int argc, char **argv) {
    return real_main(argc, argv);
}

int main(int argc, char **argv) {





    return sub_main(argc, argv);
}

