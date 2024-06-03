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

#ifndef DOXYGEN_SKIP
/** \cond */

#include <base/types.h>
#include <stdio.h>
#include <time.h>

#define _FILE_OFFSET_BITS 64

#define ANSI_COLOR_DIMMED "\x1b[2m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_BRIGHT_RED "\x1b[91m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

enum LogConfigOptionType {
    ShowColors = 0, // value is bool *
    ShowStdout = 1, // value is bool *
    ShowTimestamp = 2, // value is bool *
    ShowMillis = 3, // value is bool *
    ShowLogLevel = 4, // value is bool *
    AutoRotate = 5, // value is bool *
    DeleteRotation = 6, // value is bool *
    MaxSizeBytes = 7, // value is u64 *
    MaxAgeMillis = 8, // value is u64 *
    LogFilePath = 9, // value is char *
    FileHeader = 10, // value is char *
};
typedef enum LogConfigOptionType LogConfigOptionType;

struct LogConfigOption {
    LogConfigOptionType type;
    void* value;
};
typedef struct LogConfigOption LogConfigOption;

enum LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
};
typedef enum LogLevel LogLevel;

struct Log {
    FILE* fp;
    bool show_colors;
    bool show_stdout;
    bool show_timestamp;
    bool show_millis;
    bool show_log_level;
    bool auto_rotate;
    bool delete_rotation;
    bool is_init;
    u64 max_size_bytes;
    u64 max_age_millis;
    off_t off;
    clock_t last_rotation;
    char* path;
    char* file_header;
    LogLevel level;

    bool debug_malloc;
};
typedef struct Log Log;

/** \endcond */
#endif // DOXYGEN_SKIP

/**
 * Initialize option as a ShowColors option with the specified value.
 * The default value for ShowColors is true.
 * @param option Pointer to the location of the option to configure.
 * @param value the boolean value to set. true - show colors, false
 * do not show colors.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_show_colors(&option, false);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_show_colors(LogConfigOption* option, bool value);

/**
 * Initialize option as a ShowStdout option with the specified value.
 * The default value for ShowStdout is true.
 * @param option Pointer to the location of the option to configure.
 * @param value the boolean value to set. true - print log lines to
 * stdout, false - do not print log lines to stdout.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_show_stdout(&option, false);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_show_stdout(LogConfigOption* option, bool value);

/**
 * Initialize option as a ShowTimestamp option with the specified value.
 * The default value for ShowTimestamp is true.
 * @param option Pointer to the location of the option to configure.
 * @param value the boolean value to set. true - display a timestamp
 * with the log line, false - do not display a timestamp.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_show_timestamp(&option, false);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_show_timestamp(LogConfigOption* option, bool value);

/**
 * Initialize option as a ShowMillis option with the specified value.
 * The default value for ShowMillis is true.
 * @param option Pointer to the location of the option to configure.
 * @param value the boolean value to set. true - display milliseconds
 * with the log line, false - do not display milliseconds.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_show_millis(&option, false);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_show_millis(LogConfigOption* option, bool value);

/**
 * Initialize option as a ShowLogLevel option with the specified value.
 * The default value for ShowLogLevel is true.
 * @param option Pointer to the location of the option to configure.
 * @param value the boolean value to set. true - display log level
 * information with the log line, false - do not display log level
 * information.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_show_log_level(&option, false);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_show_log_level(LogConfigOption* option, bool value);

/**
 * Initialize option as an AutoRotate option with the specified value.
 * The default value for AutoRotate is false.
 * @param option Pointer to the location of the option to configure.
 * @param value the boolean value to set. true - automatic rotation will occur,
 * false - automatic rotation will not occur.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_auto_rotate(&option, false);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_auto_rotate(LogConfigOption* option, bool value);

/**
 * Initialize option as a DeleteRotation option with the specified value.
 * The default value for DeleteRotation is false.
 * @param option Pointer to the location of the option to configure.
 * @param value the boolean value to set. true - delete the log files as
 * they are rotated, false - do not delete the log files as they
 * are rotated. This value must be false in all non-test configurations.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_delete_rotation(&option, false);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_delete_rotation(LogConfigOption* option, bool value);

/**
 * Initialize option as a MaxSizeBytes option with the specified value.
 * The default value for MaxSizeBytes is the maximum value for u64.
 * It is important to note that logs do not automatically rotate when
 * this size is reached. The [log_rotate] function must be called. Even with
 * auto rotate configured, rotation will only occur when a line is logged
 * or the rotate function is called.
 * @param option Pointer to the location of the option to configure.
 * @param value the number of bytes at which a log file should be rotated.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option - max log size of 1 mb
 *     log_config_option_max_size_bytes(&option, 1024 * 1024);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_max_size_bytes(LogConfigOption* option, u64 value);

/**
 * Initialize option as a MaxAgeMillis option with the specified value.
 * The default value for MaxAgeMillis is the maximum value for u64.
 * It is important to note that logs do not automatically rotate when
 * this time occurs. The [log_rotate] function must be called. Even with
 * auto rotate configured, rotation will only occur when a line is logged
 * or the rotate function is called.
 * @param option Pointer to the location of the option to configure.
 * @param value the number of milliseconds at which a log file should
 * be rotated.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option - 1 hr
 *     // (1000 milliseconds * 60 secs * 60 mins)
 *     log_config_option_max_age_millis(&option, 1000 * 60 * 60);
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_max_age_millis(LogConfigOption* option, u64 value);

/**
 * Initialize option as a LogFilePath option with the specified value.
 * The default value for LogFilePath is NULL (or no log file).
 * @param option Pointer to the location of the option to configure.
 * @param value the path to the location, on disk, where the log file
 * should be stored.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_log_file_path(&option, "/home/myuser/test.log");
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_log_file_path(LogConfigOption* option, char* value);

/**
 * Initialize option as a FileHeader option with the specified value.
 * The default value for FileHeader is NULL (or no file header).
 * @param option Pointer to the location of the option to configure.
 * @param value the path to the location, on disk, where the log file
 * should be stored.
 * @return 0 on success, -1 if the required memory cannot be allocated.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_file_header(&option, "myheader");
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log line
 *     log_line(&log, Info, "this is a test");
 *
 *     return 0;
 * }
 * @endcode
 */
int log_config_option_file_header(LogConfigOption* option, char* value);

/**
 * Free a LogConfigOption.
 * @param option the log config option to free.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *
 *     // init a configuration option
 *     log_config_option_file_header(&option, "myheader");
 *
 *     // free the log config option
 *     log_config_option_free(&option);
 *
 *     return 0;
 * }
 * @endcode
 */
void log_config_option_free(LogConfigOption* option);

/**
 * Log a line to the specified Log at the specified LogLevel.
 * @param log the log to write to.
 * @param level the level to log at.
 * @param line the formatted line to log.
 * @return 0 on on success -1 if an error occurs.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_file_header(&option, "myheader");
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log lines
 *     log_line(&log, Trace, "this is a test %d", 1);
 *     log_line(&log, Debug, "this is a test %d", 2);
 *     log_line(&log, Info, "this is a test %d", 3);
 *     log_line(&log, Warn, "this is a test %d", 4);
 *     log_line(&log, Error, "this is a test %d", 5);
 *     log_line(&log, Fatal, "this is a test %d", 6);
 *
 *     return 0;
 * }
 * @endcode
 */
int log_line(Log* log, LogLevel level, char* line, ...);

/**
 * Log a line to the specified Log at the specified LogLevel.
 * This function is the same as log_line, except that the
 * line will be logged to both stdout and any specified
 * log file regardless of the configuration of the underlying
 * log.
 * @param log the log to write to.
 * @param level the level to log at.
 * @param line the formatted line to log.
 * @return 0 on on success -1 if an error occurs.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_file_header(&option, "myheader");
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log lines
 *     log_all(&log, Trace, "this is a test %d", 1);
 *     log_all(&log, Debug, "this is a test %d", 2);
 *     log_all(&log, Info, "this is a test %d", 3);
 *     log_all(&log, Warn, "this is a test %d", 4);
 *     log_all(&log, Error, "this is a test %d", 5);
 *     log_all(&log, Fatal, "this is a test %d", 6);
 *
 *     return 0;
 * }
 * @endcode
 */
int log_all(Log* log, LogLevel level, char* line, ...);

/**
 * Log a line to the specified Log at the specified LogLevel.
 * This function is the same as log_line, except that the
 * line is logged plain without any formatting regardless
 * of the configuration of the underlying log.
 * @param log the log to write to.
 * @param level the level to log at.
 * @param line the formatted line to log.
 * @return 0 on on success -1 if an error occurs.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_file_header(&option, "myheader");
 *
 *     // init our logger with a single configuration option
 *     logger(&log, 1, option);
 *     log_init(&log);
 *
 *     // test log lines
 *     log_plain(&log, Trace, "this is a test %d", 1);
 *     log_plain(&log, Debug, "this is a test %d", 2);
 *     log_plain(&log, Info, "this is a test %d", 3);
 *     log_plain(&log, Warn, "this is a test %d", 4);
 *     log_plain(&log, Error, "this is a test %d", 5);
 *     log_plain(&log, Fatal, "this is a test %d", 6);
 *
 *     return 0;
 * }
 * @endcode
 */
int log_plain(Log* log, LogLevel level, char* line, ...);

/**
 * Rotate the log. Note that this function will rotate the
 * log whether or not the underlying log _needs_ to be rotated
 * per its configuration. It's the responsibility of the user
 * to call log_need_rotate to determine whether or not the
 * log actually needs to be rotated before calling this function.
 * @param log the log to be rotated.
 * @return 0 on on success -1 if an error occurs.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option, option2;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_file_header(&option, "myheader");
 *     // init a configuration option
 *     log_config_option_max_size_bytes(&option2, 1000000);
 *
 *     // init our logger with configuration options
 *     logger(&log, 2, option, option2);
 *     log_init(&log);
 *
 *     while((line = get_next_line()) != NULL) {
 *         log_line(&log, Info, "line=%s", line);
 *         if(log_need_rotate(&log)) {
 *             log_rotate(&log);
 *         }
 *     }
 *     return 0;
 * }
 * @endcode
 */
int log_rotate(Log* log);

/**
 * Determine, based on the underlying log's configuration
 * whether or not the log needs to be rotated.
 * @param log the log to check.
 * @return true if the log needs to be rotated, otherwise
 * return false.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option, option2;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_file_header(&option, "myheader");
 *     // init a configuration option
 *     log_config_option_max_size_bytes(&option2, 1000000);
 *
 *     // init our logger with configuration options
 *     logger(&log, 2, option, option2);
 *     log_init(&log);
 *
 *     while((line = get_next_line()) != NULL) {
 *         log_line(&log, Info, "line=%s", line);
 *         if(log_need_rotate(&log)) {
 *             log_rotate(&log);
 *         }
 *     }
 *     return 0;
 * }
 * @endcode
 */
bool log_need_rotate(Log* log);

/**
 * Sets the LogLevel for the specified Log. The default LogLevel
 * is Info.
 * @param log the log to set the LogLevel for.
 * @param level the level to set.
 *
 * @code
 * #include <log/log.h>
 *
 * int main() {
 *     LogConfigOption option, option2;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_max_size_bytes(&option, 1000000);
 *     // init a configuration option
 *     log_config_option_max_age_millis(&option2, 1000000);
 *
 *     // init our logger with configuration options
 *     logger(&log, 2, option, option2);
 *     log_init(&log);
 *
 *     log_set_level(&log, Warn);
 *
 *     log_line(&log, Info, "this will not show up");
 *     log_line(&log, Warn, "this will show up");
 *
 *     return 0;
 * }
 * @endcode
 */
void log_set_level(Log* log, LogLevel level);

/**
 * Initialize a log opening the log file for writing.
 * @param log the log to init.
 * @return 0 on success 1 if an error occurs.
 */
int log_init(Log* log);

/**
 * Close the log's underlying file desciptor / file handle.
 * @param log the log to close.
 * @return 0 on success -1 if an error occurs.
 */
int log_close(Log* log);

/**
 * Free all resources associated with this log.
 * @param log the log to free.
 */
void log_free(Log* log);

/**
 * Set a LogConfigOption after the log has been initialized.
 * This function will result in an error if the LogFilePath
 * is specified. That option may not be configured after
 * the log has been initialized.
 * @param log the log to set the config option for.
 * @param option the option to set.
 * @return 0 on on success -1 if an error occurs.
 */
int log_set_config_option(Log* log, LogConfigOption option);

/**
 * Configure a logger based on the specified parameters.
 * @param log the log configure.
 * @param num the number of additional parameters specified using the VA_ARGS
 * mechanism.
 * @return 0 on on success -1 if an error occurs.
 */
int logger(Log* log, int num, ...);

// global logger

#ifndef DOXYGEN_SKIP
/** \cond */
int _global_logger(bool is_plain, bool is_all, LogLevel level, LogLevel global, char* line, ...);
/** \endcond */
#endif // DOXYGEN_SKIP

/**
 * Initialize the global logger with the specified LogConfigOptions.
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Info
 *
 * int main() {
 *     LogConfigOption option, option2;
 *     Log log;
 *
 *     // init a configuration option
 *     log_config_option_show_colors(&option, false);
 *     // init a configuration option
 *     log_config_option_show_millis(&option, false);
 *
 *     // init our logger with configuration options
 *     init_global_logger(2, option, option2);
 *
 *     trace("global log message");
 *     debug("global log message");
 *     info("global log message");
 *     warn("global log message");
 *     error("global log message");
 *     fatal("global log message");
 *
 *     return 0;
 * }
 * @endcode
 */
int init_global_logger(int num, ...);

/**
 * Rotate the global log. Note that this function will rotate the
 * log whether or not the underlying log _needs_ to be rotated
 * per its configuration. It's the responsibility of the user
 * to call log_need_rotate to determine whether or not the
 * log actually needs to be rotated before calling this function.
 */
int global_log_rotate();

/**
 * Determine whether or not the global log needs to be rotated
 * based on the specified configurations.
 */
bool global_log_need_rotate();

/**
 * Set a configuration option after the initialization of the global logger.
 * This function calls log_set_config_option for the underlying global log.
 * @see [log_set_config_option].
 */
int global_log_config_option(LogConfigOption option);

/**
 * Log to the global logger at the Trace level with the specified line and arguments.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Trace
 *
 * int main() {
 *     trace("global log message");
 *     trace("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define trace(line, ...) (_global_logger(false, false, Trace, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Debug level with the specified line and arguments.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Debug
 *
 * int main() {
 *     debug("global log message");
 *     debug("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define debug(line, ...) (_global_logger(false, false, Debug, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Info level with the specified line and arguments.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Info
 *
 * int main() {
 *     info("global log message");
 *     info("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define info(line, ...) (_global_logger(false, false, Info, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Warn level with the specified line and arguments.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Warn
 *
 * int main() {
 *     warn("global log message");
 *     warn("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define warn(line, ...) (_global_logger(false, false, Warn, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Error level with the specified line and arguments.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Error
 *
 * int main() {
 *     error("global log message");
 *     error("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define error(line, ...) (_global_logger(false, false, Error, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Fatal level with the specified line and arguments.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Fatal
 *
 * int main() {
 *     fatal("global log message");
 *     fatal("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define fatal(line, ...) (_global_logger(false, false, Fatal, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Trace level with the specified line and arguments.
 * This macro is identical to trace, except that no formatting is printed. Only the line
 * itself is printed to stdout and the log file if configured.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Trace
 *
 * int main() {
 *     trace_plain("global log message");
 *     trace_plain("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define trace_plain(line, ...) (_global_logger(true, false, Trace, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Debug level with the specified line and arguments.
 * This macro is identical to debug, except that no formatting is printed. Only the line
 * itself is printed to stdout and the log file if configured.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Debug
 *
 * int main() {
 *     debug_plain("global log message");
 *     debug_plain("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define debug_plain(line, ...) (_global_logger(true, false, Debug, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Info level with the specified line and arguments.
 * This macro is identical to info, except that no formatting is printed. Only the line
 * itself is printed to stdout and the log file if configured.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Info
 *
 * int main() {
 *     info_plain("global log message");
 *     info_plain("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define info_plain(line, ...) (_global_logger(true, false, Info, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Warn level with the specified line and arguments.
 * This macro is identical to warn, except that no formatting is printed. Only the line
 * itself is printed to stdout and the log file if configured.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Warn
 *
 * int main() {
 *     warn_plain("global log message");
 *     warn_plain("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define warn_plain(line, ...) (_global_logger(true, false, Warn, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Error level with the specified line and arguments.
 * This macro is identical to error, except that no formatting is printed. Only the line
 * itself is printed to stdout and the log file if configured.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Error
 *
 * int main() {
 *     error_plain("global log message");
 *     error_plain("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define error_plain(line, ...) (_global_logger(true, false, Error, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Fatal level with the specified line and arguments.
 * This macro is identical to fatal, except that no formatting is printed. Only the line
 * itself is printed to stdout and the log file if configured.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Fatal
 *
 * int main() {
 *     fatal_plain("global log message");
 *     fatal_plain("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define fatal_plain(line, ...) (_global_logger(true, false, Fatal, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Trace level with the specified line and arguments.
 * This macro is identical to trace, except that the line is logged to both stdout and
 * a file, if configured, regardless of the underlying global logger's specified configuration.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Trace
 *
 * int main() {
 *     trace_all("global log message");
 *     trace_all("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define trace_all(line, ...) (_global_logger(false, true, Trace, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Debug level with the specified line and arguments.
 * This macro is identical to debug, except that the line is logged to both stdout and
 * a file, if configured, regardless of the underlying global logger's specified configuration.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Debug
 *
 * int main() {
 *     debug_all("global log message");
 *     debug_all("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define debug_all(line, ...) (_global_logger(false, true, Debug, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Info level with the specified line and arguments.
 * This macro is identical to info, except that the line is logged to both stdout and
 * a file, if configured, regardless of the underlying global logger's specified configuration.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Info
 *
 * int main() {
 *     info_all("global log message");
 *     info_all("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define info_all(line, ...) (_global_logger(false, true, Info, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Warn level with the specified line and arguments.
 * This macro is identical to warn, except that the line is logged to both stdout and
 * a file, if configured, regardless of the underlying global logger's specified configuration.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Warn
 *
 * int main() {
 *     warn_all("global log message");
 *     warn_all("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define warn_all(line, ...) (_global_logger(false, true, Warn, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Error level with the specified line and arguments.
 * This macro is identical to error, except that the line is logged to both stdout and
 * a file, if configured, regardless of the underlying global logger's specified configuration.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Error
 *
 * int main() {
 *     error_all("global log message");
 *     error_all("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define error_all(line, ...) (_global_logger(false, true, Error, LOG_LEVEL, line, ##__VA_ARGS__));

/**
 * Log to the global logger at the Fatal level with the specified line and arguments.
 * This macro is identical to fatal, except that the line is logged to both stdout and
 * a file, if configured, regardless of the underlying global logger's specified configuration.
 *
 * # Parameters
 * @param line The line to be logged. Additional formatting parameters may be specified.
 * The formatting is the same as printf and other related format string based function.
 *
 * # Return
 * return 0 on success or -1 if an error occurs.
 *
 * # Examples
 *
 * @code
 * #include <log/log.h>
 *
 * #define LOG_LEVEL Fatal
 *
 * int main() {
 *     fatal_all("global log message");
 *     fatal_all("global log message with arg %d", 10);
 *
 *     return 0;
 * }
 * @endcode
 */
#define fatal_all(line, ...) (_global_logger(false, true, Fatal, LOG_LEVEL, line, ##__VA_ARGS__));

#endif /* __LOG_LOG_ */
