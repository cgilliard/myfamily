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

#include <base/rand.h>
#include <execinfo.h>
#include <inttypes.h>
#include <limits.h>
#include <log/log.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

const char PathSeparator =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif

#define _FILE_OFFSET_BITS 64

u64 log_now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    u64 ret = (u64)(tv.tv_sec) * 1000 + (u64)(tv.tv_usec) / 1000;

    return ret;
}

int sprintf_err(char* str, const char* string, ...)
{
    va_list args;
    va_start(args, string);
    int ret = vsprintf(str, string, args);
    va_end(args);
    return (ret > 0) ? 0 : ret;
}

int set_option(Log* log, int type, void* value)
{
    if (value == NULL) {
        fputs("error: value pointer cannot be NULL\n", stderr);
        return -1;
    }
    if (type == ShowColors) {
        log->show_colors = *((bool*)value);
    } else if (type == ShowStdout) {
        log->show_stdout = *((bool*)value);
    } else if (type == ShowTimestamp) {
        log->show_timestamp = *((bool*)value);
    } else if (type == ShowMillis) {
        log->show_millis = *((bool*)value);
    } else if (type == ShowLogLevel) {
        log->show_log_level = *((bool*)value);
    } else if (type == AutoRotate) {
        log->auto_rotate = *((bool*)value);
    } else if (type == DeleteRotation) {
        log->delete_rotation = *((bool*)value);
    } else if (type == MaxSizeBytes) {
        if (*((u64*)value) < 1) {
            fputs("error: MaxSizeBytes cannot be less than 1\n", stderr);
            return -1;
        }
        log->max_size_bytes = *((u64*)value);
    } else if (type == MaxAgeMillis) {
        if (*((u64*)value) < 1000) {
            fputs("error: MaxAgeMillis cannot be less than 1,000\n", stderr);
            return -1;
        }
        log->max_age_millis = *((u64*)value);
    } else if (type == LogFilePath) {
        char* buf = ((char*)value);
        int len = strlen(buf);
        if (len < 1) {
            fputs("error: LogFilePath must be at least 1 bytes long\n", stderr);
            return -1;
        }
        log->path = malloc(sizeof(char) * (len + 1));
        if (log->path == NULL || log->debug_malloc) {
            if (log->path)
                free(log->path); // if debug_malloc is set
            log->path = NULL;
            fputs("error: Could not allocate the required memory\n", stderr);
            return -1;
        }
        strcpy(log->path, buf);
    } else if (type == FileHeader) {
        char* buf = ((char*)value);
        int len = strlen(buf);
        if (len < 1) {
            fputs("error: FileHeader must be at least 1 bytes long\n", stderr);
            return -1;
        }
        char* tmp = malloc(sizeof(char) * (len + 1));
        if (tmp == NULL || log->debug_malloc) {
            if (tmp)
                free(tmp); // if debug_malloc is set
            tmp = NULL;
            fputs("error: Could not allocate the required memory\n", stderr);
            return -1;
        }
        strcpy(tmp, buf);
        if (log->file_header != NULL)
            free(log->file_header);
        log->file_header = tmp;
    }

    return 0;
}

int do_logger(Log* log, int num, va_list valist)
{
    int ret = 0;

    // set defaults
    log->fp = NULL;
    log->level = Info;
    log->show_colors = true;
    log->show_stdout = true;
    log->show_timestamp = true;
    log->show_millis = true;
    log->show_log_level = true;
    log->auto_rotate = false;
    log->delete_rotation = false;
    log->max_size_bytes = ULONG_MAX;
    log->max_age_millis = ULONG_MAX;
    log->path = NULL;
    log->file_header = NULL;
    log->off = 0;
    log->last_rotation = log_now();
    log->is_init = false;
    log->debug_malloc = false;

    // iterate through arg list for overrides
    for (int i = 0; i < num; i++) {
        LogConfigOptionImpl next = va_arg(valist, LogConfigOptionImpl);
        if (set_option(log, next.type, next.value)) {
            if (log->file_header != NULL)
                free(log->file_header);
            if (log->path != NULL)
                free(log->path);
            log->path = NULL;
            log->file_header = NULL;
            ret = -1;
            break;
        }
    }
    return ret;
}

int logger(Log* log, int num, ...)
{
    va_list valist;
    va_start(valist, num);
    int ret = do_logger(log, num, valist);
    va_end(valist);
    return ret;
}

int get_format(Log* log, LogLevel level, char* buf)
{
    int ret = 0;
    char milli_buf[14];
    char log_level_buf[20];
    char dt_buf[40];
    char spacing[2];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (log->show_millis) {
        struct timeval time;
        gettimeofday(&time, NULL);
        int millis = time.tv_usec / 1000;
        ret = sprintf_err(milli_buf, ".%03d", millis);
    } else {
        strcpy(milli_buf, "");
    }

    if (log->show_timestamp) {
        strcpy(spacing, " ");
    } else {
        strcpy(spacing, "");
    }

    if (log->show_log_level && ret == 0) {
        if (level == Trace) {
            if (log->show_colors) {
                ret = sprintf_err(log_level_buf, "%s(" ANSI_COLOR_YELLOW "TRACE" ANSI_COLOR_RESET ")", spacing);
            } else {
                ret = sprintf_err(log_level_buf, "%s(TRACE)", spacing);
            }
        } else if (level == Debug) {
            if (log->show_colors) {
                ret = sprintf_err(log_level_buf, "%s(" ANSI_COLOR_CYAN "DEBUG" ANSI_COLOR_RESET ")", spacing);
            } else {
                ret = sprintf_err(log_level_buf, "%s(DEBUG)", spacing);
            }
        } else if (level == Info) {
            if (log->show_colors) {
                ret = sprintf_err(log_level_buf, "%s(" ANSI_COLOR_GREEN "INFO" ANSI_COLOR_RESET ")", spacing);
            } else {
                ret = sprintf_err(log_level_buf, "%s(INFO)", spacing);
            }
        } else if (level == Warn) {
            if (log->show_colors) {
                ret = sprintf_err(log_level_buf, "%s(" ANSI_COLOR_MAGENTA "WARN" ANSI_COLOR_RESET ")", spacing);
            } else {
                ret = sprintf_err(log_level_buf, "%s(WARN)", spacing);
            }
        } else if (level == Error) {
            if (log->show_colors) {
                ret = sprintf_err(log_level_buf, "%s(" ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET ")", spacing);
            } else {
                ret = sprintf_err(log_level_buf, "%s(ERROR)", spacing);
            }
        } else if (level == Fatal) {
            if (log->show_colors) {
                ret = sprintf_err(log_level_buf, "%s(" ANSI_COLOR_BRIGHT_RED "FATAL" ANSI_COLOR_RESET ")", spacing);
            } else {
                ret = sprintf_err(log_level_buf, "%s(FATAL)", spacing);
            }
        }
    } else {
        strcpy(log_level_buf, "");
    }

    if (log->show_timestamp && ret == 0) {
        if (log->show_colors) {
            ret = sprintf_err(
                dt_buf,
                "[" ANSI_COLOR_DIMMED
                "%d-%02d-%02d %02d:%02d:%02d%s" ANSI_COLOR_RESET
                "]",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                milli_buf);
        } else {
            ret = sprintf_err(
                dt_buf,
                "["
                "%d-%02d-%02d %02d:%02d:%02d%s"
                "]",
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                milli_buf);
        }
    } else {
        strcpy(dt_buf, "");
    }

    if ((level == Info || level == Warn) && log->show_log_level) {
        strcpy(spacing, ":");
    } else {
        strcpy(spacing, "");
    }

    if (ret == 0) {
        ret = sprintf_err(
            buf,
            "%s%s%s: ",
            dt_buf,
            log_level_buf,
            spacing);
    }
    return ret;
}

int do_log(Log* log, LogLevel level, char* line, bool is_plain, bool is_all, va_list args)
{
    int ret = 0;
    if (!log->is_init) {
        fputs("error: log has not been initialized\n", stderr);
        return -1;
    }

    if (level >= log->level) {
        va_list args_copy;
        va_copy(args_copy, args);

        int len = strlen(line);
        char fline[len + 100];

        if (is_plain) {
            strcpy(fline, "");
        } else {
            ret = get_format(log, level, fline);
        }
        strcat(fline, line);
        strcat(fline, "\n");

        if ((log->show_stdout || is_all) && ret == 0) {
            int v = vprintf(fline, args);
	    ret = (v > 0) ? 0 : v;
        }

        if (log->fp && ret == 0) {
            int v = vfprintf(log->fp, fline, args_copy);
	    ret = (v > 0) ? 0 : v;
            log->off = ftello(log->fp);
        }
        if (log->auto_rotate && ret == 0) {
            if (log_need_rotate(log)) {
                ret = log_rotate(log);
            }
        }
    }
    return ret;
}

int log_all(Log* log, LogLevel level, char* line, ...)
{
    va_list args;
    va_start(args, line);
    int ret = do_log(log, level, line, false, true, args);
    va_end(args);
    return ret;
}

int log_plain(Log* log, LogLevel level, char* line, ...)
{
    va_list args;
    va_start(args, line);
    int ret = do_log(log, level, line, true, false, args);
    va_end(args);
    return ret;
}

int log_line(Log* log, LogLevel level, char* line, ...)
{
    va_list args;
    va_start(args, line);
    int ret = do_log(log, level, line, false, false, args);
    va_end(args);
    return ret;
}

void log_set_level(Log* log, LogLevel level)
{
    log->level = level;
}

int log_init(Log* log)
{
    if (log->is_init) {
        fputs("error: log has already been initialized\n", stderr);
        return -1;
    }
    if (log->path) {
        bool write_header = false;
        if (log->file_header && access(log->path, F_OK) != 0) {
            write_header = true;
        }
        log->fp = fopen(log->path, "a");
        if (write_header) {
            fprintf(log->fp, "%s\n", log->file_header);
        }
        fseek(log->fp, 0, SEEK_END);
        log->off = ftello(log->fp);
    }
    log->is_init = true;
    return 0;
}

int log_set_config_option(Log* log, LogConfigOptionImpl option)
{
    if (!log->is_init) {
        fputs("error: log has not been initialized\n", stderr);
        return -1;
    }
    if (option.type == LogFilePath && !log->debug_malloc) { // bypass this when we're debugging malloc
        fputs("error: cannot change log file path after initialization\n", stderr);
        return -1;
    }

    return set_option(log, option.type, option.value);
}

int log_rotate(Log* log)
{
    // format:
    // name.log -> name.r_<mon>_<day>_<year>_<time>_<rand>.log

    int ret = 0;
    char rotation_name[strlen(log->path) + 100];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char* fname = strrchr(log->path, PathSeparator);
    if (fname == NULL) {
        fname = log->path;
    } else {
        ret = -1;
        if (strlen(fname) > 0) {
            ret = 0;
            fname = fname + 1;
        }
    }

    if (!log->is_init) {
        fputs("error: log has not been initialized\n", stderr);
        ret = -1;
    }

    char* ext;
    if (ret == 0) {
        strncpy(rotation_name, log->path, fname - log->path);
        rotation_name[fname - log->path] = 0;

        ext = strrchr(fname, '.');
        if (ext == NULL) {
            ext = fname + strlen(fname);
        }
    }

    if (ret == 0) {
        char date_format[100];
        strncpy(rotation_name + (fname - log->path), fname, ext - fname);
        rotation_name[ext - log->path] = 0;
        strcat(rotation_name, ".r_");
        u64 r;
        ret = rand_u64(&r);
        if (ret == 0) {
            sprintf_err(date_format,
                "%d_%02d_%02d_%02d_%02d_%02d_%" PRIu64,
                tm.tm_year + 1900,
                tm.tm_mon + 1,
                tm.tm_mday,
                tm.tm_hour,
                tm.tm_min,
                tm.tm_sec,
                r);
            strcat(rotation_name, date_format);
            strcat(rotation_name, ext);
            fclose(log->fp);
            if (log->delete_rotation) {
                remove(log->path);
            } else {
                rename(log->path, rotation_name);
            }

            log->fp = fopen(log->path, "w");
            if (log->file_header)
                fprintf(log->fp, "%s\n", log->file_header);
            fseek(log->fp, 0, SEEK_END);
            log->off = ftello(log->fp);
        }
    }

    if (ret == 0) {
        log->off = 0;
        log->last_rotation = log_now();
    }
    return ret;
}

bool log_need_rotate(Log* log)
{
    u64 now = log_now();
    u64 diff = now - log->last_rotation;
    return log->off > log->max_size_bytes || diff > log->max_age_millis;
}

int log_close(Log* log)
{
    if (log->fp) {
        fclose(log->fp);
        log->fp = NULL;
    }
    return 0;
}

int _log_allocate_config_option(LogConfigOptionImpl* option, size_t size, bool debug_malloc_err, void* value)
{
    int ret = 0;

    option->value = malloc(size);
    if (option->value == NULL || debug_malloc_err) {
        ret = -1;
        fputs("error: Could not allocate the required memory\n", stderr);
    }

    return ret;
}

int log_config_option_show_colors(LogConfigOptionImpl* option, bool value)
{
    option->type = ShowColors;
    int ret = _log_allocate_config_option(option, sizeof(bool), false, &value);
    if (!ret)
        *((bool*)option->value) = value;
    return ret;
}

int log_config_option_show_stdout(LogConfigOptionImpl* option, bool value)
{
    option->type = ShowStdout;
    int ret = _log_allocate_config_option(option, sizeof(bool), false, &value);
    if (!ret)
        *((bool*)option->value) = value;
    return ret;
}

int log_config_option_show_timestamp(LogConfigOptionImpl* option, bool value)
{
    option->type = ShowTimestamp;
    int ret = _log_allocate_config_option(option, sizeof(bool), false, &value);
    if (!ret)
        *((bool*)option->value) = value;
    return ret;
}

int log_config_option_show_millis(LogConfigOptionImpl* option, bool value)
{
    option->type = ShowMillis;
    int ret = _log_allocate_config_option(option, sizeof(bool), false, &value);
    if (!ret)
        *((bool*)option->value) = value;
    return ret;
}

int log_config_option_show_log_level(LogConfigOptionImpl* option, bool value)
{
    option->type = ShowLogLevel;
    int ret = _log_allocate_config_option(option, sizeof(bool), false, &value);
    if (!ret)
        *((bool*)option->value) = value;
    return ret;
}

int log_config_option_auto_rotate(LogConfigOptionImpl* option, bool value)
{
    option->type = AutoRotate;
    int ret = _log_allocate_config_option(option, sizeof(bool), false, &value);
    if (!ret)
        *((bool*)option->value) = value;
    return ret;
}

int log_config_option_delete_rotation(LogConfigOptionImpl* option, bool value)
{
    option->type = DeleteRotation;
    int ret = _log_allocate_config_option(option, sizeof(bool), false, &value);
    if (!ret)
        *((bool*)option->value) = value;
    return ret;
}

int log_config_option_max_size_bytes(LogConfigOptionImpl* option, u64 value)
{
    option->type = MaxSizeBytes;
    int ret = _log_allocate_config_option(option, sizeof(u64), false, &value);
    if (!ret)
        *((u64*)option->value) = value;
    return ret;
}

int log_config_option_max_age_millis(LogConfigOptionImpl* option, u64 value)
{
    option->type = MaxAgeMillis;
    int ret = _log_allocate_config_option(option, sizeof(u64), false, &value);
    if (!ret)
        *((u64*)option->value) = value;
    return ret;
}

int log_config_option_log_file_path(LogConfigOptionImpl* option, char* value)
{
    option->type = LogFilePath;
    int ret = 0;
    if (value == NULL) {
        option->value = NULL;
    } else {
        int len = strlen(value);
        ret = _log_allocate_config_option(option, sizeof(char) * (len + 1), false, &value);
        if (!ret)
            strcpy(option->value, value);
    }
    return ret;
}

int log_config_option_file_header(LogConfigOptionImpl* option, char* value)
{
    option->type = FileHeader;
    int ret = 0;
    if (value == NULL) {
        option->value = NULL;
    } else {
        int len = strlen(value);
        ret = _log_allocate_config_option(option, sizeof(char) * (len + 1), false, &value);
        if (!ret)
            strcpy(option->value, value);
    }
    return ret;
}

void log_config_option_free(LogConfigOptionImpl* option)
{
    if (option->value != NULL) {
        free(option->value);
	option->value = NULL;
    }
}

void log_free(Log* log)
{
    if (log->path) {
        free(log->path);
	log->path = NULL;
    }
    if (log->file_header) {
        free(log->file_header);
	log->file_header = NULL;
    }
}

pthread_mutex_t _global_logger_mutex__ = PTHREAD_MUTEX_INITIALIZER;
Log _global_logger__;
bool _global_logger_is_init__ = false;

int _global_logger(bool is_plain, bool is_all, LogLevel level, LogLevel global, char* line, ...)
{
    int ret = 0;
    pthread_mutex_lock(&_global_logger_mutex__);

    if (!_global_logger_is_init__) {
        ret = logger(&_global_logger__, 0);
        if (ret == 0)
            ret = log_init(&_global_logger__);
        if (ret == 0)
            _global_logger_is_init__ = true;
    }

    if (ret == 0) {
        log_set_level(&_global_logger__, global);

        va_list args;
        va_start(args, line);
        ret = do_log(&_global_logger__, level, line, is_plain, is_all, args);
        va_end(args);
    }

    pthread_mutex_unlock(&_global_logger_mutex__);
    return ret;
}

int init_global_logger(int num, ...)
{
    if (_global_logger_is_init__)
        return -1;
    int ret = 0;
    pthread_mutex_lock(&_global_logger_mutex__);

    va_list valist;
    va_start(valist, num);
    ret = do_logger(&_global_logger__, num, valist);
    va_end(valist);

    if (ret == 0)
        ret = log_init(&_global_logger__);
    if (ret == 0)
        _global_logger_is_init__ = true;

    pthread_mutex_unlock(&_global_logger_mutex__);
    return ret;
}

int global_log_rotate()
{
    pthread_mutex_lock(&_global_logger_mutex__);
    int ret = log_rotate(&_global_logger__);
    pthread_mutex_unlock(&_global_logger_mutex__);
    return ret;
}

bool global_log_need_rotate()
{
    pthread_mutex_lock(&_global_logger_mutex__);
    bool ret = log_need_rotate(&_global_logger__);
    pthread_mutex_unlock(&_global_logger_mutex__);
    return ret;
}

int global_log_config_option(LogConfigOptionImpl option)
{
    pthread_mutex_lock(&_global_logger_mutex__);
    int ret = log_set_config_option(&_global_logger__, option);
    pthread_mutex_unlock(&_global_logger_mutex__);
    return ret;
}

void _debug_global_logger_is_init__()
{
    _global_logger_is_init__ = false;
}
