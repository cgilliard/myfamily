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

#ifndef __LOG_LOG_
#define __LOG_LOG_

#define _FILE_OFFSET_BITS 64

#include <base/types.h>
#include <time.h>

#define ANSI_COLOR_DIMMED     "\x1b[2m"
#define ANSI_COLOR_RED        "\x1b[31m"
#define ANSI_COLOR_BRIGHT_RED "\x1b[91m"
#define ANSI_COLOR_GREEN      "\x1b[32m"
#define ANSI_COLOR_YELLOW     "\x1b[33m"
#define ANSI_COLOR_BLUE       "\x1b[34m"
#define ANSI_COLOR_MAGENTA    "\x1b[35m"
#define ANSI_COLOR_CYAN       "\x1b[36m"
#define ANSI_COLOR_RESET      "\x1b[0m"

enum LogConfigOptionType {
        ShowColors        = 0, // value is bool *
	ShowStdout        = 1, // value is bool *
	ShowTimestamp     = 2, // value is bool *
	ShowMillis        = 3, // value is bool *
	ShowLogLevel      = 4, // value is bool *
	AutoRotate        = 5, // value is bool *
	DeleteRotation    = 6, // value is bool *
	MaxSizeBytes      = 7, // value is u64 *
	MaxAgeMillis      = 8, // value is u64 *
	LogFilePath       = 9, // value is char *
	FileHeader        = 10, // value is char *
};
typedef enum LogConfigOptionType LogConfigOptionType;

struct LogConfigOption {
	LogConfigOptionType type;
	void *value;
};
typedef struct LogConfigOption LogConfigOption;

enum LogLevel {
	Trace = 0,
	Debug = 1,
	Info  = 2,
	Warn  = 3,
	Error = 4,
	Fatal = 5,
};
typedef enum LogLevel LogLevel;

struct Log {
	FILE *fp;
	bool show_colors;
	bool show_stdout;
	bool show_timestamp;
	bool show_millis;
	bool show_log_level;
	bool auto_rotate;
	bool delete_rotation;
	u64 max_size_bytes;
	u64 max_age_millis;
	off_t off;
	clock_t last_rotation; 
	char *path;
	char *file_header;
	LogLevel level;
};
typedef struct Log Log;

int  log_config_option_show_colors(LogConfigOption *option, bool value);
int  log_config_option_show_stdout(LogConfigOption *option, bool value);
int  log_config_option_show_timestamp(LogConfigOption *option, bool value);
int  log_config_option_show_millis(LogConfigOption *option, bool value);
int  log_config_option_show_log_level(LogConfigOption *option, bool value);
int  log_config_option_auto_rotate(LogConfigOption *option, bool value);
int  log_config_option_delete_rotation(LogConfigOption *option, bool value);
int  log_config_option_max_size_bytes(LogConfigOption *option, u64 value);
int  log_config_option_max_age_millis(LogConfigOption *option, u64 value);
int  log_config_option_log_file_path(LogConfigOption *option, char *value);
int  log_config_option_file_header(LogConfigOption *option, char *value);
void log_config_option_free(LogConfigOption *option);

int  log_line(Log *log, LogLevel level, char *line, ...);
int  log_all(Log *log, LogLevel level, char *line, ...);
int  log_plain(Log *log, LogLevel level, char *line, ...);
int  log_rotate(Log *log);
bool log_need_rotate(Log *log);
int  log_set_level(Log *log, LogLevel level);
int  log_init(Log *log);
int  log_close(Log *log);
void log_free(Log *log);
int  log_config_option(Log *log, LogConfigOption option);

int logger(Log *log, int num, ...);

#endif /* __LOG_LOG_ */

