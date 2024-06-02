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

#ifdef __linux__ 
#define _XOPEN_SOURCE 500
#endif /* __linux__ */
#include <criterion/criterion.h>
#include <log/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ftw.h>

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

Test(log, basic)
{
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    log_config_option_show_colors(&opt4, false);

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

    cr_assert_neq(log_line(&log, Trace, "test"), 0);
    cr_assert_neq(log_set_config_option(&log, opt3), 0);
    cr_assert_neq(log_rotate(&log), 0);
    log_init(&log);
    cr_assert_neq(log_init(&log), 0);
    cr_assert_neq(log_set_config_option(&log, opt1), 0);
    log_set_level(&log, Trace);

    log_line(&log, Trace, "this is a test1");
    log_line(&log, Debug, "this is a test2");
    log_line(&log, Info, "this is a test3");
    log_line(&log, Warn, "this is a test4");
    log_line(&log, Error, "this is a test5");
    log_line(&log, Fatal, "this is a test6");
    log_set_config_option(&log, opt4);
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
    log_config_option_free(&opt4);

    log_free(&log);
}

Test(log, threshold)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    remove("./.log_threshold.fam/log_threshold.log");
    rmdir("./.log_threshold.fam/");

    mkdir("./.log_threshold.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_threshold.fam/log_threshold.log");
    log_config_option_show_millis(&opt2, false);
    log_config_option_show_colors(&opt3, false);
    log_config_option_max_age_millis(&opt4, 10000000);

    logger(&log, 4, opt1, opt2, opt3, opt4);
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

    rmrf("./.log_dorot.fam/");
    mkdir("./.log_dorot.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_dorot.fam/log_dorot.log");
    log_config_option_max_size_bytes(&opt2, 100);
    log_config_option_file_header(&opt3, "myheader");
    log_config_option_show_stdout(&opt4, false);

    logger(&log, 4, opt1, opt2, opt3, opt4);
    log_init(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    bool need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);

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

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, true);

    log_rotate(&log);
    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);
    log_plain(&log, Info, "---log_all--- %i", 7);
    log_close(&log);

    DIR *dfd;

    char *dir = "./.log_dorot.fam/";
    dfd = opendir(dir);
    struct dirent *dp;
    bool found_log = false;
    bool found_rot = false;
    int log_size = 0;
    int rot_size = 0;

    while((dp = readdir(dfd)) != NULL) {
	struct stat stbuf;
	char filename_qfd[300];

	sprintf( filename_qfd , "%s/%s",dir,dp->d_name) ;
        stat(filename_qfd, &stbuf);

	if(!strcmp(dp->d_name, "log_dorot.log")) {
		found_log = true;
		log_size = stbuf.st_size;
	} else if(strstr(dp->d_name, "log_dorot.r") != NULL) {
		rot_size = stbuf.st_size;
		found_rot = true;
	}

    }

    cr_assert_eq(rot_size > log_size, true);
    cr_assert_eq(rot_size != 0, true);
    cr_assert_eq(log_size != 0, true);
    cr_assert_eq(found_log, true);
    cr_assert_eq(found_rot, true);

    rmrf("./.log_dorot.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_config_option_free(&opt4);
    log_free(&log);
}

Test(log, autorotate) {
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4, opt5;

    rmrf("./.log_doautorot.fam/");
    mkdir("./.log_doautorot.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_doautorot.fam/log_doautorot.log");
    log_config_option_max_size_bytes(&opt2, 100);
    log_config_option_file_header(&opt3, "myheader");
    log_config_option_show_stdout(&opt4, false);
    log_config_option_auto_rotate(&opt5, true);

    logger(&log, 5, opt1, opt2, opt3, opt4, opt5);
    log_init(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    bool need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);

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

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);

    log_plain(&log, Info, "---log_all--- %i", 7);
    log_close(&log);

    DIR *dfd;

    char *dir = "./.log_doautorot.fam/";
    dfd = opendir(dir);
    struct dirent *dp;
    bool found_log = false;
    bool found_rot = false;
    int log_size = 0;
    int rot_size = 0;

    while((dp = readdir(dfd)) != NULL) {
        struct stat stbuf;
        char filename_qfd[300];

        sprintf( filename_qfd , "%s/%s",dir,dp->d_name) ;
        stat(filename_qfd, &stbuf);

        if(!strcmp(dp->d_name, "log_doautorot.log")) {
                found_log = true;
                log_size = stbuf.st_size;
        } else if(strstr(dp->d_name, "log_doautorot.r") != NULL) {
                rot_size = stbuf.st_size;
                found_rot = true;
        }

    }

    cr_assert_eq(rot_size > log_size, true);
    cr_assert_eq(rot_size != 0, true);
    cr_assert_eq(log_size != 0, true);
    cr_assert_eq(found_log, true);
    cr_assert_eq(found_rot, true);

    rmrf("./.log_doautorot.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_config_option_free(&opt4);
    log_config_option_free(&opt5);
    log_free(&log);
    
}

Test(log, deleterotate) {
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4, opt5;

    rmrf("./.log_dodelrot.fam/");
    mkdir("./.log_dodelrot.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_dodelrot.fam/log_dodelrot.log");
    log_config_option_max_size_bytes(&opt2, 100);
    log_config_option_file_header(&opt3, "myheader");
    log_config_option_show_stdout(&opt4, false);
    log_config_option_delete_rotation(&opt5, true);

    logger(&log, 5, opt1, opt2, opt3, opt4, opt5);
    log_init(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    bool need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);

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

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, true);
    log_rotate(&log);
    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);

    log_plain(&log, Info, "---log_all--- %i", 7);
    log_close(&log);

    DIR *dfd;

    char *dir = "./.log_dodelrot.fam/";
    dfd = opendir(dir);
    struct dirent *dp;
    bool found_log = false;
    bool found_rot = false;
    int log_size = 0;
    int rot_size = 0;

    while((dp = readdir(dfd)) != NULL) {
        struct stat stbuf;
        char filename_qfd[300];

        sprintf( filename_qfd , "%s/%s",dir,dp->d_name) ;
        stat(filename_qfd, &stbuf);

        if(!strcmp(dp->d_name, "log_dodelrot.log")) {
                found_log = true;
                log_size = stbuf.st_size;
        } else if(strstr(dp->d_name, "log_dodelrot.r") != NULL) {
                rot_size = stbuf.st_size;
                found_rot = true;
        }

    }

    cr_assert_eq(rot_size, 0);
    cr_assert_eq(log_size != 0, true);
    cr_assert_eq(found_log, true);
    cr_assert_eq(found_rot, false);

    rmrf("./.log_dodelrot.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_config_option_free(&opt4);
    log_config_option_free(&opt5);
    log_free(&log);
}

Test(log, misconfig) {
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4, opt5, opt6;
    LogConfigOption valid1, valid2;

    log_config_option_log_file_path(&opt1, NULL);
    log_config_option_max_size_bytes(&opt2, 0);
    log_config_option_file_header(&opt3, NULL);
    log_config_option_max_age_millis(&opt4, 100);
    log_config_option_log_file_path(&opt5, "");
    log_config_option_file_header(&opt6, "");

    log_config_option_log_file_path(&valid1, "valid.log");
    log_config_option_file_header(&valid2, "valid_header");

    int ret = logger(&log, 1, opt1);
    cr_assert_neq(ret, 0);
    ret = logger(&log, 1, opt2);
    cr_assert_neq(ret, 0);
    ret = logger(&log, 1, opt3);
    cr_assert_neq(ret, 0);
    ret = logger(&log, 1, opt4);
    cr_assert_neq(ret, 0);
    ret = logger(&log, 1, opt5);
    cr_assert_neq(ret, 0);
    ret = logger(&log, 1, opt6);
    cr_assert_neq(ret, 0);
    ret = logger(&log, 3, valid1, valid2, opt2);
    cr_assert_neq(ret, 0);

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_config_option_free(&opt4);
    log_config_option_free(&opt5);
    log_config_option_free(&valid1);
    log_config_option_free(&valid2);
    log_free(&log);
}

Test(log, timerotation) {
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4, opt5;

    rmrf("./.log_dotimerot.fam/");
    mkdir("./.log_dotimerot.fam", 0700);

    log_config_option_log_file_path(&opt1, "./.log_dotimerot.fam/log_dotimerot.log");
    log_config_option_max_age_millis(&opt2, 3000);
    log_config_option_file_header(&opt3, "myheader");
    log_config_option_show_stdout(&opt4, true);
    log_config_option_auto_rotate(&opt5, false);

    logger(&log, 5, opt1, opt2, opt3, opt4, opt5);
    log_init(&log);
    log_plain(&log, Info, "---log_all--- %i", 7);
    bool need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);

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

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);

    sleep(4);

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, true);
    log_rotate(&log);
    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);
    

    log_plain(&log, Info, "---log_all--- %i", 7);

    need_rotate = log_need_rotate(&log);
    cr_assert_eq(need_rotate, false);
    log_close(&log);

    rmrf("./.log_dotimerot.fam/");

    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_config_option_free(&opt4);
    log_config_option_free(&opt5);
    log_free(&log);

}

Test(log, changeheader)
{
    char buf[100];
    Log log;
    LogConfigOption opt1, opt2, opt3, opt4;

    rmrf("./.log_chh.fam/");

    mkdir("./.log_chh.fam", 0700);
    log_config_option_log_file_path(&opt1, "./.log_chh.fam/log_chh.log");
    log_config_option_file_header(&opt2, "1111111");
    log_config_option_show_colors(&opt3, false);
    log_config_option_file_header(&opt4, "2222222");

    logger(&log, 3, opt1, opt2, opt3);
    log_init(&log);
    log_line(&log, Debug, "test");
    log_set_config_option(&log, opt4);
    log_rotate(&log);
    log_close(&log);


    DIR *dfd;

    char *dir = "./.log_chh.fam/";
    dfd = opendir(dir);
    struct dirent *dp;
    bool found_log = false;
    bool found_rot = false;

    while((dp = readdir(dfd)) != NULL) {
        struct stat stbuf;
        char filename_qfd[300];

        sprintf( filename_qfd , "%s/%s",dir,dp->d_name) ;
        stat(filename_qfd, &stbuf);

        if(!strcmp(dp->d_name, "log_chh.log")) {
		printf("name=%s\n", dp->d_name);
                found_log = true;
		FILE* fp = fopen(filename_qfd, "r");
                fgets(buf, 100, fp);
	        cr_assert_eq(strcmp(buf, "2222222\n"), 0);
    		fclose(fp);
        } else if(strstr(dp->d_name, "log_chh.r") != NULL) {
		printf("name=%s\n", dp->d_name);
		FILE* fp = fopen(filename_qfd, "r");
                fgets(buf, 100, fp);
		cr_assert_eq(strcmp(buf, "1111111\n"), 0);
                fclose(fp);
                found_rot = true;
        }

    }

    cr_assert_eq(found_log, true);
    cr_assert_eq(found_rot, true);

    rmrf("./.log_chh.fam/");

    log_config_option_free(&opt4);
    log_config_option_free(&opt3);
    log_config_option_free(&opt2);
    log_config_option_free(&opt1);
    log_free(&log);
}

Test(log, rotationnames) {
    char buf[100];
    Log log;
    LogConfigOption opt1;

    rmrf("log_same_dirfam");

    log_config_option_log_file_path(&opt1, "log_same_dirfam");

    logger(&log, 1, opt1);
    log_init(&log);
    log_line(&log, Debug, "test");
    log_set_config_option(&log, opt1);
    log_rotate(&log);
    log_close(&log);

    log_config_option_free(&opt1);
    log_free(&log);

    DIR *dfd;

    char *dir = ".";
    dfd = opendir(dir);
    struct dirent *dp;
    bool found_log = false;
    bool found_rot = false;

    while((dp = readdir(dfd)) != NULL) {
        struct stat stbuf;
        char filename_qfd[300];

        sprintf( filename_qfd , "%s/%s",dir,dp->d_name) ;
        stat(filename_qfd, &stbuf);

        if(!strcmp(dp->d_name, "log_same_dirfam")) {
                printf("name=%s\n", dp->d_name);
                found_log = true;
                FILE* fp = fopen(filename_qfd, "r");
                fgets(buf, 100, fp);
		remove(dp->d_name);
                fclose(fp);
        } else if(strstr(dp->d_name, "log_same_dirfam.r") != NULL) {
                printf("name=%s\n", dp->d_name);
                FILE* fp = fopen(filename_qfd, "r");
                fgets(buf, 100, fp);
                fclose(fp);
                found_rot = true;
		remove(dp->d_name);
        }

    }

    cr_assert_eq(found_rot, true);
    cr_assert_eq(found_log, true);

}

Test(log, global_log) {
    char buf[100];
    LogConfigOption opt1, opt2, opt3, opt4;

    rmrf("./.log_global.fam/");
    mkdir("./.log_global.fam", 0700);

    log_config_option_log_file_path(&opt1, "./.log_global.fam/log_global.log");
    log_config_option_show_colors(&opt2, false);
    log_config_option_max_size_bytes(&opt3, 10);
    init_global_logger(3, opt1, opt2, opt3);

    cr_assert_neq(init_global_logger(0), 0);

    log_config_option_free(&opt1);
    log_config_option_free(&opt2);
    log_config_option_free(&opt3);

    #define LOG_LEVEL Debug

    debug("this is a test %d", 1);
    debug_plain("this is a test %d", 2);
    debug_all("this is a test %d", 3);
    cr_assert_eq(global_log_need_rotate(), true);

    log_config_option_file_header(&opt4, "test_header");
    global_log_config_option(opt4);
    global_log_rotate();

    log_config_option_free(&opt4);


    DIR *dfd;

    char *dir = "./.log_global.fam/";
    dfd = opendir(dir);
    struct dirent *dp;
    bool found_log = false;
    bool found_rot = false;

    while((dp = readdir(dfd)) != NULL) {
        struct stat stbuf;
        char filename_qfd[300];

        sprintf( filename_qfd , "%s/%s",dir,dp->d_name) ;
        stat(filename_qfd, &stbuf);

        if(!strcmp(dp->d_name, "log_global.log")) {
                printf("name=%s\n", dp->d_name);
                found_log = true;
                FILE* fp = fopen(filename_qfd, "r");
                fgets(buf, 100, fp);
		cr_assert_neq(strstr(buf, "test_header"), NULL);
                remove(dp->d_name);
                fclose(fp);
        } else if(strstr(dp->d_name, "log_global.r") != NULL) {
                printf("name=%s\n", dp->d_name);
                FILE* fp = fopen(filename_qfd, "r");
                fgets(buf, 100, fp);
		cr_assert_eq(strstr(buf, "test_header"), NULL);
                fclose(fp);
                found_rot = true;
                remove(dp->d_name);
        }

    }

    cr_assert_eq(found_rot, true);
    cr_assert_eq(found_log, true);

    // test other sections of the global logger
    _debug_global_logger_is_init__();
    trace("this is a test ");

    rmrf("./.log_global.fam/");
}

Test(log, malloc_err) {
    bool b;
    LogConfigOption opt1;
    Log log;
    int v;

    v = _log_allocate_config_option(&opt1, sizeof(bool), true, &b);
    cr_assert_neq(v, 0);

    logger(&log, 0);
    log_init(&log);
    log_line(&log, Debug, "test");

    log_config_option_file_header(&opt1, "header");
    log.debug_malloc = true;
    v = log_set_config_option(&log, opt1);
    cr_assert_neq(v, 0);
    log_config_option_free(&opt1);

    log_config_option_log_file_path(&opt1, "test.log");
    log.debug_malloc = true;
    v = log_set_config_option(&log, opt1);
    cr_assert_neq(v, 0);
    
    log_free(&log);
}

