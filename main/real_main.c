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

#include <args/args.h>
#include <bible/bible.h>
#include <core/dir.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void build_args(Args *args) {
	const char *home_dir = get_home_directory();

	args_build(args, "fam", "0.0.1-alpha.1", "The MyFamily Developers", 0,
		   0, 0);

	// build the init SubCommand
	SubCommand init;
	ArgsParam cfg_dir;
	args_param_build(&cfg_dir, "dir",
			 "Directory to initialize the project in (defaults to "
			 "the project name if not specified)",
			 "p", true, false, NULL);
	sub_command_build(&init, "init", "Initialize project", 1, 1,
			  "<project_name>");
	sub_command_add_param(&init, &cfg_dir);

	args_add_sub_command(args, &init);

	// build the build SubCommand
	SubCommand build;
	sub_command_build(&build, "build", "Build project in release mode", 0,
			  0, "");

	args_add_sub_command(args, &build);

	// build the debug SubCommand
	SubCommand bdebug;
	sub_command_build(&bdebug, "debug", "Build project in debug mode", 0, 0,
			  "");

	args_add_sub_command(args, &bdebug);

	// build the test SubCommand
	SubCommand test;
	sub_command_build(&test, "test", "Run tests", 0, 0, "");

	args_add_sub_command(args, &test);

	// build the doc SubCommand
	SubCommand doc;
	sub_command_build(&doc, "doc", "Build documentation", 0, 0, "");

	args_add_sub_command(args, &doc);

	// build the verse SubCommand
	SubCommand verse;
	sub_command_build(&verse, "verse", "Print a random Bible verse", 0, 0,
			  "");
	ArgsParam no_colors_param, book_param, chapter_param, verse_param;
	args_param_build(&no_colors_param, "no_colors", "Do not display colors",
			 "n", false, false, NULL);
	args_param_build(
	    &book_param, "book",
	    "Randomly select a verse from the specified book of the Bible", "b",
	    true, false, NULL);
	args_param_build(
	    &chapter_param, "chapter",
	    "Randomly select a verse from the specified chapter of the Bible. "
	    "This option requires that --book be specified as well.",
	    "x", true, false, NULL);
	args_param_build(&verse_param, "verse",
			 "Select the specified verse. This option requires "
			 "that --book and --chapter be specified as well.",
			 "v", true, false, NULL);

	sub_command_add_param(&verse, &no_colors_param);
	sub_command_add_param(&verse, &book_param);
	sub_command_add_param(&verse, &chapter_param);
	sub_command_add_param(&verse, &verse_param);

	args_add_sub_command(args, &verse);

	// build the node SubCommand
	SubCommand node;
	ArgsParam cfg_threads, cfg_show;
	args_param_build(&cfg_show, "show_request", "Show request information",
			 "s", false, false, NULL);
	args_param_build(&cfg_threads, "threads",
			 "Number of threads to execute", "t", true, false, "1");

	sub_command_build(&node, "node", "Run full node", 0, 0, "");
	sub_command_add_param(&node, &cfg_threads);
	sub_command_add_param(&node, &cfg_show);

	args_add_sub_command(args, &node);

	// build core options
	ArgsParam cfg_config_dir;
	ArgsParam cfg_debug;

	args_param_build(&cfg_debug, "debug",
			 "Prints additional debugging information", "d", false,
			 false, NULL);

	char default_config_dir[strlen(home_dir) + 100];
	snprintf(default_config_dir, strlen(home_dir) + 100, "%s/.fam",
		 home_dir);
	args_param_build(&cfg_config_dir, "config_dir",
			 "Directory where configuration files are stored", "c",
			 true, false, default_config_dir);

	args_add_param(args, &cfg_debug);
	args_add_param(args, &cfg_config_dir);
}

void process_verse(Args *args, char *config_dir) {
	bool no_colors = args_value_of(args, "no_colors", NULL, 0, 0) >= 0;
	char book[100];
	args_value_of(args, "book", book, 100, 0);
	char chapter_buf[100];
	int chapter_int = -1;
	if (args_value_of(args, "chapter", chapter_buf, 100, 0) != -1) {
		chapter_int = atoi(chapter_buf);
		if (chapter_int <= 0 || chapter_int > 255) {
			fprintf(
			    stderr,
			    "The chapter specified must be a positive "
			    "integer which is equal to or less than 255.\n");
			exit(-1);
		}
	}
	char verse_buf[100];
	int verse_int = -1;
	if (args_value_of(args, "verse", verse_buf, 100, 0) != -1) {
		verse_int = atoi(verse_buf);
		if (verse_int <= 0 || verse_int > 255) {
			fprintf(
			    stderr,
			    "The verse specified must be a positive "
			    "integer which is equal to or less than 255.\n");
			exit(-1);
		}
	}

	Bible bible;
	char bible_path[strlen(config_dir) + 10];
	strcpy(bible_path, config_dir);
	strcat(bible_path, "/akjv.txt");
	if (bible_build(&bible, bible_path)) {
		fprintf(stderr, "Could not load bible at path %s: %s\n",
			bible_path, strerror(errno));
		exit(-1);
	}

	char buf[1024];
	char book_buf[100];
	u8 chapter, verse;

	u8 *chapter_ptr = NULL;
	if (chapter_int != -1) {
		chapter = chapter_int;
		chapter_ptr = &chapter;
	}
	u8 *verse_ptr = NULL;
	if (verse_int != -1) {
		verse = verse_int;
		verse_ptr = &verse;
	}

	if (verse_int != -1 && chapter_int == -1) {
		fprintf(stderr, "If --verse is specified, --chapter must also "
				"be specified\n");
		exit(-1);
	}
	if (chapter_int != -1 && (book == NULL || strlen(book) == 0)) {
		fprintf(stderr, "If --chapter is specified, --book must also "
				"be specified\n");
		exit(-1);
	}
	if (bible_random_verse_to_string(&bible, buf, 1024, !no_colors, book,
					 chapter_ptr, verse_ptr) < 0) {
		fprintf(stderr, "Could not find random verse due to: %s\n",
			strerror(errno));
		exit(-1);
	}

	printf("%s\n", buf);
	bible_cleanup(&bible);
}

int real_main(int argc, char **argv) {
	Args args;
	build_args(&args);
	args_init(&args, argc, argv);

	// we always ensure that the config directory exists.
	int config_dir_len = args_value_of(&args, "config_dir", NULL, 0, 0);
	char config_dir[config_dir_len + 1];
	args_value_of(&args, "config_dir", config_dir, config_dir_len + 1, 0);
	if (access(config_dir, F_OK)) {
		if (mkdir(config_dir, 0700)) {
			perror("Failed to create specified config directory");
			exit(-1);
		}
	}

	// get first argument (subcommand)
	char command[100];
	int ret = args_get_argument(&args, 0, command, 100);
	if (ret > 0) {
		if (!strcmp(command, "verse")) {
			process_verse(&args, config_dir);
		} else {
			printf("Not implemented!\n");
		}
	}

	args_cleanup(&args);
	return 0;
}
