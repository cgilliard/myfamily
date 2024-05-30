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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


Test(log, basic) {
	Log log;
	LogConfigOption opt1, opt2, opt3;

	log.show_colors = false;
	log.show_stdout = false;

	log_config_option_show_colors(&opt1, true);
	log_config_option_show_stdout(&opt2, true);
	logger(&log, 2, opt1, opt2);

	cr_assert_eq(log.show_colors, true);
	cr_assert_eq(log.show_stdout, true);

	free_log_config_option(&opt1);
        free_log_config_option(&opt2);

	log_config_option_show_colors(&opt1, true);
        log_config_option_show_stdout(&opt2, false);

	free_log(&log);
	logger(&log, 2, opt1, opt2);

        cr_assert_eq(log.show_colors, true);
        cr_assert_eq(log.show_stdout, false);
	cr_assert_eq(log.path, NULL);

	free_log_config_option(&opt1);
	free_log_config_option(&opt2);

	mkdir("./.log_basic.fam", 0700);

	log_config_option_log_file_path(&opt1, "./.log_basic.fam/log_basic.log");
	log_config_option_file_header(&opt2, "myheader");
	log_config_option_show_millis(&opt3, true);

	free_log(&log);
	logger(&log, 3, opt1, opt2, opt3);
	cr_assert_eq(strcmp(log.path, "./.log_basic.fam/log_basic.log"), 0);

	
	init(&log);
	
	log_line(&log, Trace, "this is a test1");
	log_line(&log, Debug, "this is a test2");
	log_line(&log, Info, "this is a test3");
	log_line(&log, Warn, "this is a test4");
	log_line(&log, Error, "this is a test5");
	log_line(&log, Fatal, "this is a test6");
	log_close(&log);

	remove("./.log_basic.fam/log_basic.log");
	rmdir("./.log_basic.fam/");

	free_log_config_option(&opt1);
	free_log_config_option(&opt2);

	log_config_option_file_header(&opt1, "header");
	free_log(&log);
        logger(&log, 1, opt1);
        cr_assert_eq(strcmp(log.file_header, "header"), 0);
	init(&log);

        free_log_config_option(&opt1);

	free_log(&log);
}

Test(log, output) {
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
	init(&log);
	log_line(&log, Info, "this is a test1");
	log_close(&log);

	FILE *fp = fopen("./.log_output.fam/log_output.log", "r");
	fgets(buf, 100, fp);
	printf("s=%s\n", buf);
	cr_assert_eq(strstr(buf, "]")-buf, 20);
	cr_assert_neq(strstr(buf, "[20"), NULL);
	cr_assert_neq(strstr(buf, "(INFO):  this is a test1"), NULL);
	fclose(fp);

	remove("./.log_output.fam/log_output.log");
        rmdir("./.log_output.fam/");


	free_log_config_option(&opt3);
	free_log_config_option(&opt2);
	free_log_config_option(&opt1);
	free_log(&log);

}

Test(log, configurations) {
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
        init(&log);
        log_line(&log, Info, "this is a testx");
        log_close(&log);
        
        FILE *fp = fopen("./.log_configurations.fam/log_output.log", "r");
        fgets(buf, 100, fp);
	cr_assert_eq(strstr(buf, "]")-buf, 24);
	cr_assert_neq(strstr(buf, "]"), NULL);
        cr_assert_neq(strstr(buf, "(INFO):  this is a test"), NULL);
        fclose(fp);

        remove("./.log_configurations.fam/log_output.log");
        free_log_config_option(&opt2);
        free_log_config_option(&opt1);
	free_log_config_option(&opt4);
        free_log(&log);

	// without timestamp
	log_config_option_log_file_path(&opt1, "./.log_configurations.fam/log_output.log");
	log_config_option_show_timestamp(&opt2, false);
	log_config_option_show_colors(&opt4, false);

	logger(&log, 3, opt1, opt2, opt4);
	init(&log);
        log_line(&log, Info, "this is a testx");
        log_close(&log);

	
        fp = fopen("./.log_configurations.fam/log_output.log", "r");
        fgets(buf, 100, fp);
        cr_assert_eq(strstr(buf, "]"), NULL);
        cr_assert_neq(strstr(buf, "(INFO):  this is a test"), NULL);
	cr_assert_eq(strstr(buf, "(INFO):  this is a test"), buf);
        fclose(fp);
	remove("./.log_configurations.fam/log_output.log");
	free_log_config_option(&opt2);
        free_log_config_option(&opt1);
        free_log(&log);

	// without timestamp or log level
	log_config_option_log_file_path(&opt1, "./.log_configurations.fam/log_output.log");
        log_config_option_show_timestamp(&opt2, false);
	log_config_option_show_log_level(&opt3, false);

        logger(&log, 3, opt1, opt2, opt3);
        init(&log);
        log_line(&log, Info, "this is a testx");
        log_close(&log);


        fp = fopen("./.log_configurations.fam/log_output.log", "r");
        fgets(buf, 100, fp);
        cr_assert_eq(strstr(buf, "]"), NULL);
        cr_assert_eq(strstr(buf, ": this is a test"), buf);
        fclose(fp);
        free_log_config_option(&opt2);
        free_log_config_option(&opt1);
        free_log(&log);

	remove("./.log_configurations.fam/log_output.log");
	rmdir("./.log_configurations.fam/");

}
