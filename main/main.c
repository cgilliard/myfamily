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
#include <log/log.h>
#include <util/misc.h>

#define LOG_LEVEL Info

int main(int argc, char** argv)
{
    LogConfigOption opt1, opt2, opt3;
    log_config_option_show_colors(&opt1, true);
    log_config_option_show_stdout(&opt2, true);
    log_config_option_show_timestamp(&opt3, false);

    init_global_logger(2, opt1, opt2);
    log_config_option_free(&opt1);
    log_config_option_free(&opt2);
    info("Main currently doesn't do %s.", "anything");
    global_log_config_option(opt3);
    log_config_option_free(&opt3);
    info("Main currently doesn't do %s.", "anything");
    info("end");

    return 0;
}
