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

#include <criterion/criterion.h>
#include <log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

Test(log, basic)
{
    Log log;
    LogConfigOption opt1, opt2, opt3;

    log.show_colors = false;
    log.show_stdout = false;

    log_config_option_show_colors(&opt1, true);
    log_config_option_show_stdout(&opt2, true);
    logger(&log, 2, opt1, opt2);

    cr_assert_eq(log.show_colors, true);
    cr_assert_eq(log.show_stdout, true);

    log_config_option_free(&opt1);
    log_config_option_free(&opt2);

    log_config_option_show_colors(&opt1, true);
    log_config_option_show_stdout(&opt2, false);

    log_free(&log);
    logger(&log, 2, opt1, opt2);

    cr_assert_eq(log.show_colors, true);
    cr_assert_eq(log.show_stdout, false);
    cr_assert_eq(log.path, NULL);

    log_config_option_free(&opt1);
    log_config_option_free(&opt2);

    mkdir("./.log_basic.fam", 0700);

    log_config_option_log_file_path(&opt1, "./.log_basic.fam/log_basic.log");
    log_config_option_file_header(&opt2, "myheader");
    log_config_option_show_millis(&opt3, true);

    log_free(&log);
    logger(&log, 3, opt1, opt2, opt3);
    cr_assert_eq(strcmp(log.path, "./.log_basic.fam/log_basic.log"), 0);

    log_init(&log);
    log_set_level(&log, Trace);

    log_line(&log, Trace, "this is a test1");
    log_line(&log, Debug, "this is a test2");
    log_line(&log, Info, "this is a test3");
    log_line(&log, Warn, "this is a test4");
    log_line(&log, Error, "this is a test5");
    log_line(&log, Fatal, "this is a test6");
    log_close(&log);

    remove("./.log_basic.fam/log_basic.log");
    rmdir("./.log_basic.fam/");

    log_config_option_free(&opt1);
    log_config_option_free(&opt2);

    log_config_option_file_header(&opt1, "header");
    log_free(&log);
    logger(&log, 1, opt1);
    cr_assert_eq(strcmp(log.file_header, "header"), 0);
    log_init(&log);

    log_config_option_free(&opt1);

    log_free(&log);
}

Test(log, threshold)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3;

    remove("./.log_threshold.fam/log_threshold.log");
    rmdir("./.log_threshold.fam/");

    mkdir("./.log_threshold.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_threshold.fam/log_threshold.log");
    log_config_option_show_millis(&opt2, false);
    log_config_option_show_colors(&opt3, false);

    logger(&log, 3, opt1, opt2, opt3);
    log_set_level(&log, Warn);
    log_init(&log);
    log_line(&log, Info, "this is a test1");
    log_close(&log);

    FILE* fp = fopen("./.log_threshold.fam/log_threshold.log", "r");
    fgets(buf, 100, fp);
    printf("s=%s\n", buf);
    cr_assert_neq(strstr(buf, "]") - buf, 20);
    cr_assert_eq(strstr(buf, "[20"), NULL);
    cr_assert_eq(strstr(buf, "(INFO):: this is a test1"), NULL);
    fclose(fp);

    remove("./.log_threshold.fam/log_threshold.log");
    rmdir("./.log_threshold.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}

Test(log, output)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3;

    remove("./.log_output.fam/log_output.log");
    rmdir("./.log_output.fam/");

    mkdir("./.log_output.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_output.fam/log_output.log");
    log_config_option_show_millis(&opt2, false);
    log_config_option_show_colors(&opt3, false);

    logger(&log, 3, opt1, opt2, opt3);
    log_init(&log);
    log_line(&log, Info, "this is a test1");
    log_close(&log);

    FILE* fp = fopen("./.log_output.fam/log_output.log", "r");
    fgets(buf, 100, fp);
    printf("s=%s\n", buf);
    cr_assert_eq(strstr(buf, "]") - buf, 20);
    cr_assert_neq(strstr(buf, "[20"), NULL);
    cr_assert_neq(strstr(buf, "(INFO):: this is a test1"), NULL);
    fclose(fp);

    remove("./.log_output.fam/log_output.log");
    rmdir("./.log_output.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}

Test(log, configurations)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    remove("./.log_configurations.fam/log_output.log");
    rmdir("./.log_configurations.fam/");

    mkdir("./.log_configurations.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_configurations.fam/log_output.log");
    log_config_option_show_millis(&opt2, true);
    log_config_option_show_colors(&opt4, false);

    logger(&log, 3, opt1, opt2, opt4);
    log_init(&log);
    log_line(&log, Info, "this is a testx");
    log_close(&log);

    FILE* fp = fopen("./.log_configurations.fam/log_output.log", "r");
    fgets(buf, 100, fp);
    cr_assert_eq(strstr(buf, "]") - buf, 24);
    cr_assert_neq(strstr(buf, "]"), NULL);
    cr_assert_neq(strstr(buf, "(INFO):: this is a test"), NULL);
    fclose(fp);

    remove("./.log_configurations.fam/log_output.log");
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_config_option_free(&opt4);
    log_free(&log);

    // without timestamp
    log_config_option_log_file_path(&opt1, "./.log_configurations.fam/log_output.log");
    log_config_option_show_timestamp(&opt2, false);
    log_config_option_show_colors(&opt4, false);

    logger(&log, 3, opt1, opt2, opt4);
    log_init(&log);
    log_line(&log, Info, "this is a testx");
    log_close(&log);

    fp = fopen("./.log_configurations.fam/log_output.log", "r");
    fgets(buf, 100, fp);
    cr_assert_eq(strstr(buf, "]"), NULL);
    cr_assert_neq(strstr(buf, "(INFO):: this is a test"), NULL);
    cr_assert_eq(strstr(buf, "(INFO):: this is a test"), buf);
    fclose(fp);
    remove("./.log_configurations.fam/log_output.log");
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);

    // without timestamp or log level
    log_config_option_log_file_path(&opt1, "./.log_configurations.fam/log_output.log");
    log_config_option_show_timestamp(&opt2, false);
    log_config_option_show_log_level(&opt3, false);

    logger(&log, 3, opt1, opt2, opt3);
    log_init(&log);
    log_line(&log, Info, "this is a testx");
    log_close(&log);

    fp = fopen("./.log_configurations.fam/log_output.log", "r");
    fgets(buf, 100, fp);
    cr_assert_eq(strstr(buf, "]"), NULL);
    cr_assert_eq(strstr(buf, ": this is a test"), buf);
    fclose(fp);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);

    remove("./.log_configurations.fam/log_output.log");
    rmdir("./.log_configurations.fam/");
}

Test(log, plain)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3;

    remove("./.log_plain.fam/log_plain.log");
    rmdir("./.log_plain.fam/");

    mkdir("./.log_plain.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_plain.fam/log_plain.log");
    log_config_option_show_millis(&opt2, false);
    log_config_option_show_colors(&opt3, false);

    logger(&log, 3, opt1, opt2, opt3);
    log_init(&log);
    log_plain(&log, Info, "this is a test1");
    log_close(&log);

    FILE* fp = fopen("./.log_plain.fam/log_plain.log", "r");
    fgets(buf, 100, fp);
    printf("s=%s\n", buf);
    cr_assert_eq(strstr(buf, "]"), NULL);
    cr_assert_eq(strstr(buf, "[20"), NULL);
    cr_assert_eq(strstr(buf, "this is a test1"), buf);
    fclose(fp);

    remove("./.log_plain.fam/log_plain.log");
    rmdir("./.log_plain.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}

Test(log, all)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    remove("./.log_all.fam/log_all.log");
    rmdir("./.log_all.fam/");

    mkdir("./.log_all.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_all.fam/log_all.log");
    log_config_option_show_millis(&opt2, false);
    log_config_option_show_colors(&opt3, false);
    log_config_option_show_stdout(&opt4, false);

    logger(&log, 4, opt1, opt2, opt3, opt4);
    log_init(&log);
    log_all(&log, Info, "---log_all---");
    log_close(&log);

    FILE* fp = fopen("./.log_all.fam/log_all.log", "r");
    fgets(buf, 100, fp);
    printf("s=%s\n", buf);
    cr_assert_neq(strstr(buf, "]"), NULL);
    cr_assert_neq(strstr(buf, "[20"), NULL);
    cr_assert_neq(strstr(buf, "---log all---"), buf);
    fclose(fp);

    remove("./.log_all.fam/log_all.log");
    rmdir("./.log_all.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}

Test(log, formatting)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    remove("./.log_fmt.fam/log_fmt.log");
    rmdir("./.log_fmt.fam/");

    mkdir("./.log_fmt.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_fmt.fam/log_fmt.log");
    log_config_option_show_millis(&opt2, false);
    log_config_option_show_colors(&opt3, false);
    log_config_option_show_stdout(&opt4, false);

    logger(&log, 4, opt1, opt2, opt3, opt4);
    log_init(&log);
    log_all(&log, Info, "---log_all--- %i", 7);
    log_close(&log);

    FILE* fp = fopen("./.log_fmt.fam/log_fmt.log", "r");
    fgets(buf, 100, fp);
    printf("s=%s\n", buf);
    cr_assert_neq(strstr(buf, "]"), NULL);
    cr_assert_neq(strstr(buf, "[20"), NULL);
    cr_assert_neq(strstr(buf, "---log_all--- 7"), NULL);
    fclose(fp);

    remove("./.log_fmt.fam/log_fmt.log");
    rmdir("./.log_fmt.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}

Test(log, need_rotate)
{
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    remove("./.log_rot.fam/log_rot.log");
    rmdir("./.log_rot.fam/");

    mkdir("./.log_rot.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_rot.fam/log_rot.log");
    log_config_option_max_size_bytes(&opt2, 100);
    log_config_option_show_colors(&opt3, false);
    log_config_option_show_stdout(&opt4, false);

    logger(&log, 4, opt1, opt2, opt3, opt4);
    log_init(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    bool need_rotate = log_need_rotate(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);

    cr_assert_eq(need_rotate, false);

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, true);
    log_close(&log);

    remove("./.log_rot.fam/log_rot.log");
    rmdir("./.log_rot.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}

Test(log, rotate)
{
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    remove("./.log_dorot.fam/log_dorot.log");
    rmdir("./.log_dorot.fam/");

    mkdir("./.log_dorot.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_dorot.fam/log_dorot.log");
    log_config_option_max_size_bytes(&opt2, 100);
    log_config_option_show_colors(&opt3, false);
    log_config_option_show_stdout(&opt4, false);

    logger(&log, 4, opt1, opt2, opt3, opt4);
    log_init(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    bool need_rotate = log_need_rotate(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_plain(&log, Info, "---log_all--- %i", 7);

    cr_assert_eq(need_rotate, false);

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, true);

    log_rotate(&log);
    log_close(&log);

    remove("./.log_dorot.fam/log_dorot.log");
    rmdir("./.log_dorot.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}
